
/* Note, quat lib's q_mult (q_dest, q_left, q_right) creates a new quat that,
 * when applied to a vec, first rotates by q_left, then by q_right.
 */

//#include <vector>

#include <rhinoio.h>
#include "nmg_Graphics.h"
#include "graphics_globals.h"
#include <BCGrid.h>
#include <BCPlane.h>
#include <nmb_PlaneSelection.h>

#include "../../vrml.h"

#include <v.h>


/** Computes the xform that is used in the viewing transformation.
    Unfortunately, vlib doesn't give a way to get the xform, only to get the
    matrix that is eventually put on the OpenGL viewing matrix stack.  So,
    I've extracted the code from v_gl_calculate_view_matrices, and used it
    almost as-is to write this function.  It could probably go into vlib, but
    I don't want to be involved with actually changing vlib, so I'll just
    leave it here.  */
static
void my_v_gl_calculate_inverse_view_xform(
    int whichUser,
    v_index displayIndex, 
    //v_matrix_type leftViewMatrix,
    //v_matrix_type rightViewMatrix
    v_xform_type & out_worldFromLeftEye,
    v_xform_type & out_worldFromRightEye);


/** Computes the smallest power of 2 greater than ARG. */
inline static
double next_power_of_2 (double arg)
{
    // XXX this is WRONG!!!!!
    return exp(ceil(log(arg)));
}


/** Build the CRhinoMesh object.
 *
 * Xform the surface over GRID defined by PLANE_SELECTION.height and save it
 * in OUT_MESH.  Xform the GRID by XF_SCALE, then by XF_ROT, then add
 * XF_XLATE.
 */
static
void build_mesh (
    CRhinoMesh &               out_mesh,
    const BCGrid * const       grid,
    const nmb_PlaneSelection & plane_selection,
    q_type                     xf_rot,
    q_vec_type                 xf_xlate,
    const double               xf_scale)
{
    // Number of gridpoints and faces in each axial direction
    const int num_X = grid->numX();
    const int num_Y = grid->numY();
    const int num_faces_X = num_X - 1;
    const int num_faces_Y = num_Y - 1;

    // The grid is oriented in a local coordinate system where:
    //  * the grid axes are parallel to the respective world axes
    //  * the grid base is embedded in the world X-Y plane
    //  * the grid origin is at world coords (minX, minY, 0)
    //  * the opposite corner of the grid is at world coords (maxX, maxY, 0)
    //
    // The gridpoints are regularly spaced, so we have now fully-defined the
    // location of all the gridpoints.  The width is the distance between
    // gridpoints.  NOTE: Adam may implement non-regularly spaced grids.
    //
    const double min_X = grid->minX();
    const double min_Y = grid->minY();
    const double width_X = (grid->maxX() - min_X) / num_X;
    const double width_Y = (grid->maxY() - min_Y) / num_Y;

    // A height field defined over the grid gives the surface mesh vertices.
    // The heightplane is a scalar field giving the height values (in the Z
    // direction, which is normal to the grid) at each grid point.  We've now
    // fully-defined the location of all the vertices of the surface mesh.
    // The color of each mesh vertex comes from the colorplane scalar field.
    const BCPlane * const height_scalar_field = plane_selection.height;

    // 3dm files don't support per-vertex color, so we'll ignore this one.
    // Perhaps we should put the color into a texture that gets mapped onto
    // the surface?
    // 
    //const BCPlane * const color_scalar_field = plane_selection.color;
    
    // Anything else?

    // Create some colors
    CRhinoColor specular (g_specular_color, g_specular_color, g_specular_color);
    CRhinoColor shiny (g_shiny, g_shiny, g_shiny);
    CRhinoColor dark (0,0,0);
    CRhinoColor white (255, 255, 255);

    // Create a material
    CRhinoMaterial material;
    material.SetAmbient (dark);
    material.SetDiffuse (CRhinoColor (190, 171, 136));
    // Not used: material.SetEmmission (dark);
    material.SetSpecular (specular);
    material.SetShine (shiny);
    // Not used: material.SetTexutreBitmapFileName ("texturefile");
    // Not used: material.SetBumpBitmapFileNmae ("bumpfile");
    material.SetRenderMaterialName ("nano_surface");  // shader name
    
    // Configure the non-geometry components
    out_mesh.SetLabel ("height_data");
    out_mesh.SetMaterial (material);
    out_mesh.SetWireColor (white);
    
    // Must set these before filling in the vertex/face/norma/texcoord arrays
    out_mesh.SetVertexCount (num_X * num_Y);
    
    out_mesh.SetFaceCount (num_faces_X * num_faces_Y);
    out_mesh.SetNormalsExist (true);
    out_mesh.SetTCoordsExist (true);
    
    // Valid texture coordinates are
    //     (u,v), st. (0 <= u < 1)  AND  0 <= v < 1)
    // The grid only uses tex coords in the interval
    //     (u,v), st.
    //            0 <= u <= (num_X-1) / nearest_greater_power_of_2
    //            0 <= v <= (num_Y-1) / nearest_greater_power_of_2
    // XXX I may have this backwards... (wrt x->u, y->v)
    // 
    // Texture Domain is a rectangular bounding box containing all the
    // texture coordinates.  The low left and high right corners are given
    // by these functions The default texture domain is the unit square.
    // 
    // void SetTextureDomain(CRhino2DPoint low, CRhino2DPoint high);
    // 
    {
        CRhino2DPoint low_left = { 0.0, 0.0 };
        CRhino2DPoint high_right = { next_power_of_2 (num_X - 1),
                                     next_power_of_2 (num_Y - 1) };

        // oops, next_power_of_2 is buggy.  For now, skip it.
        high_right[0] = num_X - 1;
        high_right[1] = num_Y - 1;
        
        out_mesh.SetTextureDomain (low_left, high_right);
    }
    

    // Set the vertices
    //
    // Since we're going to scale the grid to unit size then scale back up,
    // let's just ignore min_X and max_X for now.  Will have to deal with this
    // only if we later add non-uniform grid spacing, or if the units on the X
    // and the Y axes differ.
    //
    if (0.001 < (width_X - width_Y)) {
        cerr << "X and Y grid axes have different scales."
             << "  Using scale from X-axis" << endl;
    }
    double scale_Z_by_this = 1.0 / width_X;

    for (int vy=0, vertex_number=0;  (vy < num_Y);  ++vy)
    {
        for (int vx=0;  (vx < num_X);  ++vx)
        {
            double vert[3];

            // don't use these any more
            //
            //vert[0] = xf_scale * (min_X + (vx * width_X));
            //vert[1] = xf_scale * (min_Y + (vy * width_Y));
            //vert[2] = xf_scale * (height_scalar_field->scaledValue (vx, vy));

            vert[0] = xf_scale * vx;
            vert[1] = xf_scale * vy;
            vert[2] = xf_scale * (height_scalar_field->scaledValue (vx, vy));
            
            // ajust Z value by inverse of what we are no longer multiplying X
            // and Y by
            vert[2] *= scale_Z_by_this;
            
            q_xform (vert, xf_rot, vert);
            q_vec_add (vert, xf_xlate, vert);

            bool result = out_mesh.SetVertex (vertex_number,
                                              vert[0], vert[1], vert[2]);
            if (false == result) {
                cout << "error setting vertex #" << vertex_number << endl;
            }

            GLfloat nano_normal[3];
            vrml_compute_plane_normal (height_scalar_field, vx, vy,
                                       width_X, width_Y, 1.0, nano_normal);

            double normal[3];
            q_set_vec (normal, nano_normal[0], nano_normal[1], nano_normal[2]);
            q_xform (normal, xf_rot, normal);
            q_vec_normalize (normal, normal);
            result = out_mesh.SetNormal (vertex_number,
                                         normal[0], normal[1], normal[2]);
            if (false == result) {
                cout << "error setting normal for vertex #"
                     << vertex_number << endl;
            }

            result = out_mesh.SetTCoord (vertex_number, vx, vy);
            if (false == result) {
                cout << "error setting texture coordinate for vertex #"
                     << vertex_number << endl;
            }
            
            ++vertex_number;
        }
    }
    
    // Set the face info.  Because msvc++ doesn't honor correct for-scoping
    // rules, use iterators named differently than above.  The last 4
    // arguments to SetFace are the vertex indices, in counter-clockwise
    // order, of the 4 vertices of a quad.  If you want to specify a triangle
    // instead, simply leave off the last argument.
    //
    // In the loop, we will be specifing quads defined like this
    // 
    //    v3.____.v2
    //      |    |
    //      |    |
    //    v0.____.v1
    //
    // where v[0-3] are vertex indices.  We can compute, or carry, the correct
    // index for v0.  Then, v1 is v0+1, v3 is v0+num_X, and v2 is v3+1.

    for (int fy=0, face_number = 0;
         fy < num_faces_Y;
         ++fy)
    {
        for (int fx=0;  (fx < num_faces_X);  ++fx) {
            
            // Let's compute, not carry, the index for v0.  A good optimizing
            // compiler should be able to carry this for us, anyway. (I think)
            const int v0 = fx + (fy*num_X);

            out_mesh.SetFace (face_number,
                              v0, v0+1, v0+num_X+1, v0+num_X);

            ++face_number;
        }
    }
}



/** Builds the CRhinoViewport object.  The viewport matches the one currently
 * being used in nano, as much as possible.  Rhino viewport objects don't
 * currently support non-square frustums.
 */
static
void build_viewport_from_nano (CRhinoViewport &        out_v,
                               const v_viewport_type & viewport,
                               const int               viewport_num)
{
    /////////
    // Set the viewport name
    {
        // XXX better to use stringstream from iostream library
        static const char * viewport_base_name = "viewport";
        int str_size
            = (10  /* overestimate of overhead */
               + strlen(viewport_base_name) + (1 + (viewport_num / 10)));
        char * viewport_name = new char [1+str_size];
        sprintf (viewport_name,
                 "%s %d",
                 viewport_base_name, viewport_num);
        out_v.SetViewName (viewport_name);
    }

    /////////
    // We want a perspective projection
    out_v.SetProjection (CRhinoViewport::perspective_proj);
    
    // Where the camera is located
    // (note: q_vec_type is double[3])
    {
        // Really, we need to mimmic v_gl_draw_world (vogl/gl.c)
        v_xform_type xform_World_from_LeftEye;
        v_xform_type xform_World_from_RightEye;
        my_v_gl_calculate_inverse_view_xform (0 /*user_num*/,
                                              viewport_num,
                                              xform_World_from_LeftEye,
                                              xform_World_from_RightEye);
        
 
        q_vec_type origin = {0, 0, 0};
        q_vec_type neg_z = {0, 0, -1};

        q_vec_type camera_location;
        q_vec_type camera_direction;
        const double camera_up[] = {0, 0, 1};  // in world space

        v_x_xform_vector (camera_location, &xform_World_from_LeftEye, origin);

        v_x_xform_vector (camera_direction, &xform_World_from_LeftEye, neg_z);

        // I'm making the assumption that these need to be specified in world
        // coordinates.  Also making assumptions for up.
        out_v.SetCamera (camera_location, camera_direction, camera_up);
        out_v.SetCamera (camera_location, camera_direction, camera_up);
    }

    /////////
    // The viewing frustum
    //
    // openNURBS library seems to make the view frustum square.  It's not
    // really that way in the app. (non-square pixels?)
    //
    // Ok, looked into if further.  Looks like the write routine for the
    // viewport class will only output one number, which represents the size
    // of the frustum.  I.e., we get square frustums no matter what we do.
    //
    // Also discovered that (this version of the file format) ignores the near
    // and far clipping distances.
   {
        out_v.SetFrustum (viewport.windowOrigin  [0],
                          viewport.windowExtents [0],
                          viewport.windowOrigin  [1], 
                          viewport.windowExtents [1],
                          viewport.clipDistances [0],
                          viewport.clipDistances [1]);

        cout << "  vogl says viewport is ("
             << viewport.windowOrigin  [0] << ", "
             << viewport.windowExtents [0] << ", "
             << viewport.windowOrigin  [1] << ", "
             << viewport.windowExtents [1] << ", "
             << viewport.clipDistances [0] << ", "
             << viewport.clipDistances [1] << ")" << endl;

        double frus_left,frus_right,frus_bottom,frus_top,frus_near,frus_far;
        bool bValidFrustum = out_v.GetFrustum(&frus_left,&frus_right,
                                              &frus_bottom,&frus_top,
                                              &frus_near,&frus_far);
        
        if (bValidFrustum) {
            cout << "  frustum got set to ("
                 << frus_left   << ", " << frus_right << ", "
                 << frus_bottom << ", " << frus_top   << ", "
                 << frus_near   << ", " << frus_far   << ")"  << endl;
        } else {
            cout << "failed setting frustum" << endl;
        }


        /////////
        // From vogl/gl.c    glFrustum(left, right, bottom, top, near, far)
        /*
        glFrustum(vpPtr->windowOrigin[0], 
                  vpPtr->windowExtents[0],
                  vpPtr->windowOrigin[1], 
                  vpPtr->windowExtents[1],
                  vpPtr->clipDistances[0],
                  vpPtr->clipDistances[1]);
        
        // Where does this view get drawn in the frame buffer?
        glViewport((int) vpPtr->fbOrigin[0],
                   (int) vpPtr->fbOrigin[1],
                   (int) vpPtr->fbExtents[0],
                   (int) vpPtr->fbExtents[1]);
        */
        /////////
    }
    
    /////////
    // Where the viewport is located in it's parent window
    //
    // XXX I'm hardcoding this for now.  Should really get the aspect ratio
    // from the app.
    out_v.SetPortPosition (0.0, 1.0, 0.0, 1.0);
    
    /////////
    // Is this necessary?  Hmmm, I think we can look at {min,max}{X,Y} and set
    // the construction palne accordingly.
    {
        //const double origin[] = {0.0, 0.0, 0.0};
        //const double X_axis[] = {1.0, 0.0, 0.0};
        //const double Y_axis[] = {0.0, 1.0, 0.0};
        //out_v.SetConstructionPlane (origin, X_axis, Y_axis);
    }
    
    /////////
    //  Stuff that's not used.  Would be nice to set the target to the middle
    //  of the data.
    /*
    out_v.SetTargetDistance (target_distance);
    out_v.SetCameraAngle (angle);
    out_v.SetCamera35mmLenseLength (lense_length);
    */
    /////////
}


struct QuadCorners {
    q_vec_type bottom_left;
    q_vec_type bottom_right;
    q_vec_type top_left;
    q_vec_type top_right;

    QuadCorners() {
        reset();
    }
    
    void reset() {
        for (int i=0; i < 3; ++i) {
            bottom_left [i] = 0;
            bottom_right[i] = 0;
            top_left    [i] = 0;
            top_right   [i] = 0;
        }
    }
};

static
ostream& write_q_vec (ostream& out, const q_vec_type v)
{
    out << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
    return out;
}

static
ostream& operator << (ostream& out, const QuadCorners& qc)
{
    out << "[ bottom_left=";
    write_q_vec (out, qc.bottom_left);
    
    out << "\n  bottom_right=";
    write_q_vec (out, qc.bottom_right);
    
    out << "\n  top_left=";
    write_q_vec (out, qc.top_left);
    
    out << "\n  top_right=";
    write_q_vec (out, qc.top_right);
    
    out << " ]";
    return out;
}


/** Return a char string representing a rhino exception
 *
 * DO NOT MODIFY the return value from this function.
 */
static
const char * rhinoException_to_string (CRhinoException e)
{
    const char* s = NULL;
    switch ( e.m_type )
    {
    case CRhinoException::unable_to_write:
        s = "unable_to_write";
        break;
    case CRhinoException::unable_to_read:
        s = "unable_to_read";
        break;
    case CRhinoException::unable_to_seek:
        s = "unable_to_seek";
        break;
    case CRhinoException::unexpected_end_of_file:
        s = "unexpected_end_of_file";
        break;
    case CRhinoException::unexpected_value:
        s = "unexpected_value";
        break;
    case CRhinoException::not_supported:
        s = "not_supported";
        break;
    case CRhinoException::illegal_input:
        s = "illegal_input";
        break;
    case CRhinoException::out_of_memory:
        s = "out_of_memory";
        break;
    default:
        s = "unknow exception";
        break;
    }
    return s;
}


/** If P is one of the points we care about (based on it's name), then store
 * it in QC.
*/
static
bool set_quad_corner (QuadCorners &qc, const CRhinoPointSet & p)
{
    const char * const label = p.Label();
    q_vec_type * vec = 0;

    if (! strcmp ("bottom-left", label)) {
        vec = & (qc.bottom_left);
    }
    else if (! strcmp ("bottom-right", label)) {
        vec = & (qc.bottom_right);
    }
    else if (! strcmp ("top-left", label)) {
        vec = & (qc.top_left);
    }
    else if (! strcmp ("top-right", label)) {
        vec = & (qc.top_right);
    }

    if (! vec) {
        return false;
    }
    if (p.PointCount()==0) {
        cerr << "No points in point dataset.  Ignoring this PointSet" << endl;
        return false;
    }
    if (p.PointCount() > 1) {
        cerr << "Warning: PointSet \"" << label
             << "\" has more than one point.  Using first point." << endl;
    }

    // store the point's coords into *vec
    p.GetPoint (0, (*vec)[0], (*vec)[1], (*vec)[2]);

    return true;
}


/** Read from rhino file pRF, and set the out variable based on point data
 *  found in the file.
 */
static
bool get_corners_from_file (
    QuadCorners &       out_corners_of_model_in_rhino_frame,
    CRhinoFile *  const pRF)
{
    // set by pRF->Read()
    CRhinoFile::eStatus status = CRhinoFile::good;
        
    for ( ;; ) {
        CRhinoObject* pObj = pRF->Read (status);
        if (status != CRhinoFile::good)
            break;
            
        // Check if this object is on a layer we care about
        bool we_care = false;
        if (pObj->Layer() && pObj->Layer()->Name()) {
            we_care =
                (0 == strcmp("screen points",
                             pObj->Layer()->Name()));
        }

        // Check if it's a point object
        we_care = we_care && (pObj->TypeCode() == TCODE_RH_POINT);
        
        if (! we_care) {
            delete pObj;
            pObj = 0;
            continue;
        }

        // We care!  Now, extract the points defining the corners
        CRhinoPointSet *pPoint = dynamic_cast<CRhinoPointSet*>(pObj);
        if (! pPoint) {
            cerr << "\tError casting point object to point class" << endl;
        }
        else {
            set_quad_corner (out_corners_of_model_in_rhino_frame, *pPoint);
        }

        delete pObj;
        pObj = 0;
        
    } // for( ;; )
    
    return status == CRhinoFile::eof;
}



/** Construct the view xform
 *  
 *  The grid is embedded in the world X-Y plane.  It's origin is the world
 *  origin.  It's X and Y axes coincide with the world's X and Y axes.
 *  Each nanometer on the grid is one unit.
 *
 *  We want to xform the grid to it's place in the model.  The points in
 *  model_in_rhino_frame define the corners of where we want the grid to
 *  end up.
 *
 *  First, xform from the grid's local coordinates to lie in the unit
 *  square at the origin of the world X-Y plane.  By definition, there is
 *  no rotation involved.  So, we just have to scale.
 *  
 *  We will scale by 1/grid_size, where grid_size is the size in X and Y.
 *  This makes a uniform scaling assumption.  The topometrix software
 *  always gives us square datasets, but that may not be the case in the
 *  future, or with a different microscope.  Also, it may not be true when
 *  using a ppm file as a dataset.
 *
 *  To eliminate the square assumption, we need to do some padding.
 *  Perhaps use the max of length(X) and length(Y) as the scaling factor?
 *  
 *  Second, xform to the destination.  We can easily build the xform matrix
 *  for this as follows:
 *
 *    (all these are in the rhino model world coord sys)
 *  
 *    Orig is the origin of the destination
 *    TL   is the top left corner of the destination
 *    BR   is the bottom right corner of the destination
 *
 *         | Ax Bx Cx Tx |          A = BR - Orig
 *    xf = | Ay By Cy Ty |, where   B = TL - Orig
 *         | Az Bz Cz Tz |          T = Orig
 *
 *    C is a little more difficult:
 *      C is in the direction A cross B.
 *      The length of C defines the "z-scale".
 *
 *    We will assume that length(A) == length(B).  In that case,
 *       
 *        C = normalize (cross (A,B)) * length(A)  
 *    or (roughly)
 *        C = cross (A, B) * (1 / length(A))
 *
 *  Then, we use this xform to xform points in the grid to their
 *  locations in the model like this:
 *
 *     point_in_model = xf.apply (point_in_grid)
 */
static void compute_view_xform (
    double &       out_scale,
    q_type         out_quat,
    q_vec_type     out_xlate,
    const BCGrid & grid,
    QuadCorners &  model_in_rhino_frame)
{
    const int numX = grid.numX();
    const int numY = grid.numY();
    const int * which_num = &numX;
    
    if (numX != numY) {
        cout << "grid has different size in X and Y directions.  Using num";
        if (numX >= numY) {
            which_num = & numX;
            cout << "X";
        }
        else {
            which_num = & numY;
            cout << "Y";
        }
        cout << endl;
    }

    // Scale to unit square.
    out_scale = 1.0 / double(*which_num);
    
    //
    // build the rotation matrix
    //

    q_vec_type model_X;
    q_vec_subtract (model_X,
                    model_in_rhino_frame.bottom_right,
                    model_in_rhino_frame.bottom_left);

    q_vec_type model_Y;
    q_vec_subtract (model_Y,
                    model_in_rhino_frame.top_left,
                    model_in_rhino_frame.bottom_left);

    double len_X = q_vec_magnitude (model_X);
    double len_Y = q_vec_magnitude (model_Y);

    // check if len_X == len_Y
    if (1.01 < (len_X - len_Y)) {
        cerr << "model X and Y axes are different lengths.\n"
             << " Scaling mesh by the smaller one." << endl;
    }

    out_scale *= ( (len_X <= len_Y) ? len_X : len_Y );

    // these must be normalized so we can construct a UNIT rot matrix
    q_vec_normalize (model_X, model_X);
    q_vec_normalize (model_Y, model_Y);

    q_vec_type model_Z;
    q_vec_cross_product (model_Z, model_X, model_Y);
    q_vec_normalize     (model_Z, model_Z);
    //XXX wrong! q_vec_scale         (model_Z, double (*which_num), model_Z);
    //< NB: this may stretch in Z direction if X and Y are different lengths.
    
    q_matrix_type changeBasisMatrix_model_from_dataset;
    for (int row=0; row < 3; ++row) {
        changeBasisMatrix_model_from_dataset [row] [0] = model_X [row];
        changeBasisMatrix_model_from_dataset [row] [1] = model_Y [row];
        changeBasisMatrix_model_from_dataset [row] [2] = model_Z [row];

        // clear both the last column and the last row
        changeBasisMatrix_model_from_dataset [row] [3] = 0.0;
        changeBasisMatrix_model_from_dataset [3] [row] = 0.0;
    }
    // one more cell to clear
    changeBasisMatrix_model_from_dataset [3] [3] = 1.0;

    // out_scale alredy set
    q_set_vec (out_xlate,
               model_in_rhino_frame.bottom_left [0],
               model_in_rhino_frame.bottom_left [1],
               model_in_rhino_frame.bottom_left [2]);
    q_from_row_matrix (out_quat,
                       changeBasisMatrix_model_from_dataset);
    q_invert (out_quat, out_quat);

    // DEBUG
    q_vec_type vs;
    q_vec_type qd;
    
    q_set_vec (vs, 0, 1, 0);
    q_xform (qd, out_quat, vs);
    cout << "(" << vs[0] << ", " << vs[1] << ", " << vs[2]
         << ") xforms to ("
         << qd[0] << ", " << qd[1] << ", " << qd[2] << ")" << endl;

    q_set_vec (vs, 1, 0, 0);
    q_xform (qd, out_quat, vs);
    cout << "(" << vs[0] << ", " << vs[1] << ", " << vs[2]
         << ") xforms to ("
         << qd[0] << ", " << qd[1] << ", " << qd[2] << ")" << endl;

    q_vec_type md;
    q_matrix_type & m = changeBasisMatrix_model_from_dataset;
    for (int i=0; i<3; ++i) {
        double accum=0;
        for (int j=0; j<3; ++j) {
            accum += m[i][j] * vs[j];
        }
        md[i] = accum;
    }
    
    cout << "matrix mult gives ("
         << md[0] << ", " << md[1] << ", " << md[2] << ")" << endl;
            

}


/** Compute view xfrom based on point data in FILENAME
 *
 *  Read from FILENAME and find points that define where the plane is to go
 *  into world space.  Then, compute an xform that takes grid points to that
 *  place.  The xform is returned in OUT_QUAT and OUT_XLATE.  To xform a point
 *  from grid space to the model space, first apply the quat, then add the
 *  xlate.
 *
 *  GRID is needed to do proper scaling.  The xform that is returned has a
 *  scale component that is dependant on the grid size.
 *
 *  If an error occurs reading the file, then the out parameters are left
 *  unchanged.
 */
static
void get_view_xform (
    double &       out_scale,
    q_type         out_quat,
    q_vec_type     out_xlate,
    const char *   filename,
    const BCGrid & grid)
{
    cout << "getting reference points from \"" << filename << "\"..." << endl;

    QuadCorners model_in_rhino_frame;  // store the corners here
    bool bOK = true;

    FILE * pfile = fopen (filename, "rb");
    if ( !pfile ) {
        cerr << "Unable to open file \"" << filename << "\"." << endl;
        return;
    }

    try {
        CRhinoFile* pRF = new CRhinoFile( pfile, CRhinoFile::read );
        bOK = get_corners_from_file (model_in_rhino_frame, pRF);
        delete pRF;
    }
    catch( CRhinoException e ) {
        bOK = false;
        cerr << "threw CRhinoException: "
             << rhinoException_to_string(e) << "." << endl;
    }

    if (! bOK) {
        cerr << "Never reached end of file \"" << filename << "\"" << endl;
    }
    
    if (fclose (pfile)) {
        cerr << "An error occured closing \"" << filename << "\"" << endl;

    }
    
    cout << "...done reading reference points\n";
    cout << "This is what I read:\n" << model_in_rhino_frame << endl;

    compute_view_xform (out_scale, out_quat, out_xlate,
                        grid, model_in_rhino_frame);
}



/** Writes a .3dm file to `filename'.  It seems kind of messy passing both the
 * grid and the plane selection.  Perhaps the planeselection should have a
 * const pointer to the grid?  Or I can pass a nmb_Dataset intead.  */
void export_scene_to_openNURBS (
    const char * const         filename,
    const nmg_Graphics * const /*graphics*/,
    const BCGrid * const       grid,
    const nmb_PlaneSelection & plane_selection)
{
    /////////
    // Build all the objects before opening the file
    //

    double xf_scale = 0.0;

    q_type xf_quat;
    q_make (xf_quat, 0, 0, 1, 0);

    q_vec_type xf_xlate;
    q_set_vec (xf_xlate, 0, 0, 0);

    get_view_xform (xf_scale, xf_quat, xf_xlate, "nanodesk-2.3dm", *grid);

    // We use a CRhinoMesh object to store the height field
    CRhinoMesh my_mesh;
    build_mesh (my_mesh, grid, plane_selection, xf_quat, xf_xlate, xf_scale);

    // We store the viewports into CRhinoViewport objects
    // Let's find out how many viewports there are, among all users
    // Also, we hardcode the display index to be zero.
    const v_index display_index = 0;
    const v_display_type & display = v_display_table[display_index];
    const int total_num_viewports = display.numViewports;

    
    // Now, build the viewport objects
    //std::vector<CRhinoViewport> my_viewports (total_num_viewports);
    CRhinoViewport * my_viewports = new CRhinoViewport [total_num_viewports];
    for (int vn=0;  vn < total_num_viewports;  ++vn) {
        const v_viewport_type & v = display.viewports [vn];
        build_viewport_from_nano (my_viewports[vn], v, vn);
    }
    
    /////////
    // Now, finally, open the file and write to it
    //

    cout << "exporting scene to \"" << filename << "\"..." << flush;
    
    FILE * pfile = fopen (filename, "wb");
    if ( !pfile ) {
        cerr << "Unable to open file \"" << filename << "\"." << endl;
        return;
    }
    try {
        CRhinoFile rhino_file (pfile, CRhinoFile::write);
        rhino_file.SetUnitSystem (CRhinoFile::meters);
        
        for (int i=0;  i < total_num_viewports;  ++i) {
            my_viewports[i].Write (rhino_file);
        }

        my_mesh.Write (rhino_file);
    }
    catch (CRhinoException e) {
        cerr << "threw CRhinoException: "
             << rhinoException_to_string(e) << "." << endl;
    }

    fclose (pfile);
    // XXX error checking

    cout << "done." << endl;

    delete [] my_viewports;
}



static /*int*/
void  my_v_gl_calculate_inverse_view_xform(
    int          whichUser,
    v_index      displayIndex, 
    //v_matrix_type leftViewMatrix,
    //v_matrix_type rightViewMatrix
    v_xform_type & worldFromLeftEye,
    v_xform_type & worldFromRightEye)

    /*------------------------------------------------------------------ 
     * The following code is taken almost as-is from v_gl_calculate_view_matrices.
     * The only changes are in formatting (to my style), commenting out the
     * v_*_to_matrix calls, and making the worldFrom*Eye vars be parameters
     * instead of local vars.  I've also removed the return type and statements,
     * since the way that they are written is basically useless.  -juliano 07/2000
     *------------------------------------------------------------------ 
     */
{
    v_xform_type worldFromHead;
    v_xform_type roomFromHead;
    v_xform_type trackerFromHead;

    /* ----------- not used here --------------
     * v_xform_type worldFromLeftEye;
     * v_xform_type worldFromRightEye;
     * v_xform_type leftEyeFromWorld;
     * v_xform_type rightEyeFromWorld;
     * ----------------------------------------
     */

    v_object_type *userPtr = &v_users[whichUser];

    /********************************
     * left eye
     ********************************/
    v_x_compose(&trackerFromHead, &userPtr->xforms[V_TRACKER_FROM_HEAD_SENSOR],
                &userPtr->xforms[V_SENSOR_FROM_HEAD_UNIT]);

    v_x_compose(&roomFromHead, &userPtr->xforms[V_ROOM_FROM_HEAD_TRACKER],
                &trackerFromHead);

    v_x_compose(&worldFromHead, &v_world.users.xforms[whichUser],
                &roomFromHead);

    v_x_compose(&worldFromLeftEye, &worldFromHead,
                &userPtr->xforms[V_HEAD_FROM_LEFT_EYE]);

    /* ----------- not used here --------------
     * v_x_invert(&leftEyeFromWorld, &worldFromLeftEye);
     *
     * v_x_to_matrix(leftViewMatrix, &leftEyeFromWorld);
     * ----------------------------------------
     */

    /* all done if mono */
    if ( v_display_table[displayIndex].numViewports == 1 )
        return /*(V_OK)*/;

    /********************************
     * right eye
     ********************************/
    v_x_compose(&worldFromRightEye, &worldFromHead,
                &userPtr->xforms[V_HEAD_FROM_RIGHT_EYE]);

    /* ----------- not used here --------------
     * v_x_invert(&rightEyeFromWorld, &worldFromRightEye);
     *
     * v_x_to_matrix(rightViewMatrix, &rightEyeFromWorld);
     * ----------------------------------------
     */

    return /*(V_OK)*/;
}
