#include "aligner.h"
#include "transformSolve.h"

Aligner::Aligner(int n_images) 
{
		num_images = n_images;
		corr_set = VRPN_FALSE;
		xform = new ImageTransformPtr[n_images*n_images];
        is_surface = new vrpn_bool[n_images];
        height_scale = new double[n_images];
        xform_set = new vrpn_bool[n_images*n_images];
        dimension = new int[n_images];
        int i;
        //aligner_data_images = (nmb_Image **)new (void *)[n_images];
        aligner_data_images = new nmb_Image * [n_images];
        for (i = 0; i < n_images*n_images; i++){
            xform[i] = NULL;
            xform_set[i] = VRPN_FALSE;
        }

        for (i = 0; i < n_images; i++)
            aligner_data_images[i] = NULL;
}


int Aligner::updateTransform(int src, int dest) 
{
    int i,j;
    int dim_src, dim_dest;

    if (!corr_set) {
	fprintf(stderr, "Aligner: Error, correspondence hasn't been set\n");
	return -1;
    }
    // call the function that does the real work
	/* explanation of last argument:
			if the source image is specified as a surface in
			3 dimensions then that implies that we are supposed
			to use the extra information to compute a 3D 
			transformation
			the transformation from an image to a height
			field is not necessarily 1-1 so behavior is 
			undefined in this case
	*/
    double xform_matrix[16];
    double error;

    transformSolver(xform_matrix, &error, corr, src, dest, is_surface[src]);

    // package the result in a nice way so other people can use it
    //dim_src = dimension[src]+1;	// add one for the w component that
    // gives us translation
    //dim_dest = dimension[dest];
	dim_src = 4;
	dim_dest = 4;
	ImageTransformAffine transform(dim_src, dim_dest);

/*
	printf("updateTransform: debug\n");
	for (i = 0; i < 16; i++)
		printf("%g ", xform_matrix[i]);
	printf("\n");
*/

    for (i = 0; i < dim_dest; i++){
		for (j = 0; j < dim_src; j++){
		    transform.set(i,j,xform_matrix[j*4+i]);
		}
    }
    setTransform(src, dest, (ImageTransform *)(&transform));

/*
	fprintf(stderr, "Aligner::updateTransform, debug output:\n");
	corr_point_t test_pt, goal_pt;
	double dtest_pt[4], xd_pt[4];
	for (i = 0; i < corr.numPoints(); i++){
		corr.getPoint(src, i, &test_pt);
		dtest_pt[0] = test_pt.x;
		dtest_pt[1] = test_pt.y;
		dtest_pt[2] = test_pt.z;
		dtest_pt[3] = 1.0;
		transform.transform(dtest_pt, xd_pt);
		corr.getPoint(dest, i, &goal_pt);
		printf("T(%g,%g,%g) = (%g,%g), wanted: (%g,%g)\n",
			test_pt.x, test_pt.y, test_pt.z, 
			xd_pt[0], xd_pt[1], goal_pt.x, goal_pt.y);
		
	}
*/

	return 0;
}

void Aligner::transform(int src, double *p_src, int dest, double *p_dest) {

	if (xform[src + dest*num_images])
		xform[src + dest*num_images]->transform(p_src, p_dest);
	else {
		fprintf(stderr, "Aligner: Error, transform not initialized\n");
	}
}

// assumes the transformation can be represented by a 4x4 matrix and
// determines what that matrix must be; format for the matrix is that used by
// openGL
void Aligner::getGLtransform(int src, int dest, double *xform_mat) {

	double p_src[4] = {1,0,0,0};
	double p_dest[4] = {0,0,0,0};

	if (xform[src + dest*num_images]) {
		for (int i = 0; i < 4; i++) {
        	xform[src + dest*num_images]->transform(p_src, p_dest);
			xform_mat[i*4] = p_dest[0];
			xform_mat[i*4+1] = p_dest[1];
			xform_mat[i*4+2] = p_dest[2];
			xform_mat[i*4+3] = p_dest[3];
			p_src[i] = 0.0;
			p_src[(i+1)%4] = 1.0;
		}
	}
    else {
        fprintf(stderr, "Aligner: Error, transform not initialized\n");
    }
/*
	printf("getGLtransform: debug - should match updateTransform msg\n");
	for (int i = 0; i < 16; i++)
		printf("%g ", xform_mat[i]);
	printf("\n");
*/
}

/* Aligner::resampleImageToDepthImage
	short description: projection is texture-mapped onto a surface given
			by depth_image and then reprojected onto the x,y plane for
			the depth_image 

	long description:
       projection_image - this is assumed to be a projection of some scalar
            value from the surface represented by depth_image
       depth_image - this is a height field
       xform - this transforms points from the depth_image into points in
            the projection_image
       orthogonal_projection_image - this should typically have the same
            world dimensions as the depth_image but may have a different
            resolution; the result is put in here as an orthographic
			projection along the z axis from the depth_image
       desired algorithm: Each pixel in the orthogonal_projection_image is
            converted into a patch of the surface from depth_image based
            on the x,y values at the pixel corners. The region formed by
            projecting this piece of the surface onto the projection_image
            (using xform) is computed and then a sample value is computed
            by integrating over this region.
       cheap algorithm (implemented): The center of each pixel in the
            orthogonal_projection_image is converted into a surface point
            using depth_image. This point is transformed into the
            projection_image and the sample value is computed using
            bilinear interpolation.
    */

void Aligner::resampleImageToDepthImage(
                        const nmb_Image &projection_image,
                        const nmb_Image &depth_image,
                        const ImageTransform &xform,
                        nmb_Image &orthogonal_projection_image)
{
    int i,j;
    int w,h;
    w = orthogonal_projection_image.width();
    h = orthogonal_projection_image.height();
    double i_center, j_center;
    double p_world[4] = {0,0,0,1}, p_projection[4];
    double i_depth, j_depth, i_proj, j_proj;
    double value;

    for (i = 0, i_center = 0.5; i < w; i++, i_center += 1.0){
        for (j = 0, j_center = 0.5; j < h; j++, j_center += 1.0){
            // find corresponding location in the depth image which may be
            // at a different resolution and offset
	    orthogonal_projection_image.pixelToWorld(i_center, j_center,
		p_world[0],p_world[1]);
	    depth_image.worldToPixel(p_world[0], p_world[1], i_depth, j_depth);
	    // i_depth, j_depth give the location in the depth image
	    if (i_depth > 0 && i_depth < depth_image.width() &&
		j_depth > 0 && j_depth < depth_image.height()) {

		p_world[2] = depth_image.getValueInterpolated(i_depth, j_depth);
		// now we transform p_world into the projection image
		xform.transform(p_world, p_projection);
		projection_image.worldToPixel(p_projection[0], p_projection[1],
			i_proj, j_proj);
		value = projection_image.getValueInterpolated(i_proj, j_proj);
	    } else {
		value = 0.0;
	    }
	    orthogonal_projection_image.setValue(i,j, value);
        }
    }
}

/* Aligner::computeMaxRequiredResampleResolution
	  projects corners of each pixel in projection image onto depth
        image and computes the size of the pixel in the orthogonal
        projection image; the dimensions of the smallest such transformed
        pixel are returned as a ratio to the size of pixels in
        the depth_image
        (to preserve nearly as much information as possible when using
         resampleImageToDepthImage(),
         you should multiply the x and y resolution of the depth image
         by the given factors or by the maximum of the two factors if you
         want to have square pixels)
    */
void Aligner::computeMaxRequiredResampleResolution(
                            const nmb_Image &projection_image,
                            const nmb_Image &depth_image,
                            const ImageTransform &xform,
                            double &x_factor,
                            double &y_factor)
{
	// temporary :
	x_factor = 1.0;
	y_factor = 1.0;
	return;
}



