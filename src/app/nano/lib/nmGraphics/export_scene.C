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
    leave it here.
*/
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


/** Builds the CRhinoMesh object
 */
static
void build_mesh (
    CRhinoMesh & mesh,
    const BCGrid * const grid,
    const nmb_PlaneSelection & plane_selection)
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
    //CRhinoColor min_color (g_minColor[0], g_minColor[1], g_minColor[2]);

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
    mesh.SetLabel ("height_data");
    mesh.SetMaterial (material);
    mesh.SetWireColor (white);
    
    // Must set these before filling in the vertex/face/norma/texcoord arrays
    mesh.SetVertexCount (num_X * num_Y);
    
    mesh.SetFaceCount (num_faces_X * num_faces_Y);
    mesh.SetNormalsExist (true);
    mesh.SetTCoordsExist (true);
    
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
        
        mesh.SetTextureDomain (low_left, high_right);
    }
    

    // Set the vertices
    for (int vy=0, vertex_number=0;  (vy < num_Y);  ++vy)
    {
        for (int vx=0;  (vx < num_X);  ++vx)
        {
            bool result =
                mesh.SetVertex (vertex_number,
                                min_X + (vx * width_X),
                                min_Y + (vy * width_Y),
                                height_scalar_field->scaledValue (vx, vy));

            if (false == result) {
                cout << "error setting vertex #" << vertex_number << endl;
            }

            GLfloat normal[3];
            vrml_compute_plane_normal (height_scalar_field, vx, vy,
                                       width_X, width_Y, 1.0, normal);
            
            result = mesh.SetNormal (vertex_number,
                                     normal[0], normal[1], normal[2]);
            
            if (false == result) {
                cout << "error setting normal for vertex #"
                     << vertex_number << endl;
            }

            result = mesh.SetTCoord (vertex_number, vx, vy);
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

            mesh.SetFace (face_number,
                          v0, v0+1, v0+num_X+1, v0+num_X);

            ++face_number;
        }
    }
}



/** Builds the CRhinoViewport object.
 */
static
void build_viewport (CRhinoViewport & out_v,
                     const v_viewport_type & viewport,
                     const int viewport_num)
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
        q_normalize (camera_direction, camera_direction);

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


/** REturns a char string representing a rhino exception
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


static
bool set_quad_corner (QuadCorners &qc, const CRhinoPointSet & p)
{
    const char * const label = p.Label();
    q_vec_type * vec = 0;

    if (! strcmp ("bottom-left", label)) {
        vec = & (qc.bottom_left);
        cout << "setting corner bottom-left" << endl;
    }
    else if (! strcmp ("bottom-right", label)) {
        vec = & (qc.bottom_right);
        cout << "setting corner bottom-right" << endl;
    }
    else if (! strcmp ("top-left", label)) {
        vec = & (qc.top_left);
        cout << "setting corner top-left" << endl;
    }
    else if (! strcmp ("top-right", label)) {
        vec = & (qc.top_right);
        cout << "setting corner top-right" << endl;
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
    
    p.GetPoint (0, (*vec)[0], (*vec)[1], (*vec)[2]);
    return true;
}



/** Figures out view xform Reads from FILENAME and finds points that define
 *  where the plane is to go into world space.  Then, computes an xform that
 *  takes grid points to that place.
 */
static void get_view_xform (const char * filename)
{
    cout << "getting view xform from \"" << filename << "\"..." << endl;

    QuadCorners model_in_rhino_frame;

    FILE * pfile = fopen (filename, "rb");
    if ( !pfile ) {
        cerr << "Unable to open file \"" << filename << "\"." << endl;
        return;
    }
    bool bOK = true;
    try {
        CRhinoFile* pRF = new CRhinoFile( pfile, CRhinoFile::read );
        
        CRhinoFile::eStatus status = CRhinoFile::good;
        
        for ( ;; ) {
            CRhinoObject* pObj = pRF->Read( status );
            if ( status != CRhinoFile::good )
                break;
            
            // check if this object is on a layer we care about
            bool we_care = false;
            if (pObj->Layer() && pObj->Layer()->Name()) {
                we_care =
                    (0 == strcmp("screen points",
                                 pObj->Layer()->Name()));
            }
            if (! we_care) {
                // wrong layer
                delete pObj;
                pObj = 0;
                continue;
            }

            // if it's a point object, extract the points we care about
            switch (pObj->TypeCode())
            {
            default:              // throw away stuff we don't want
                delete pObj;
                pObj = 0;

            case TCODE_RH_POINT:  // CRhinoPointSet ( 3d point list )

                cout << "\t\"" << pObj->Label() << "\""
                     << " from layer \"" << pObj->Layer()->Name() << "\""
                     << " (" << pObj->ClassName() << ")"
                     << endl;
                
                CRhinoPointSet *pPoint = dynamic_cast<CRhinoPointSet*>(pObj);
                if (! pPoint) {
                    cerr << "\tError casting point object to point class"
                         << endl;
                }
                else {
                    set_quad_corner (model_in_rhino_frame, *pPoint);
                }
                delete pObj;
                pObj = 0;

            } // switch (pObj->TypeCode())

        } // for( ;; )

        bOK = ( status == CRhinoFile::eof );
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
    
    fclose (pfile);
    // XXX error checking
    
    cout << "...done reading view xform" << endl;

    cout << "This is what I read:\n" << model_in_rhino_frame << endl;
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

    // We use a CRhinoMesh object to store the height field
    CRhinoMesh my_mesh;
    build_mesh (my_mesh, grid, plane_selection);

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
        build_viewport (my_viewports[vn], v, vn);
    }
    

    get_view_xform ("nanodesk-2.3dm");

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
