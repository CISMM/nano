#include "imageTransform.h"

ImageTransformAffine::ImageTransformAffine(int d_src, int d_dest):
	ImageTransform(d_src, d_dest)
{
    int i,j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++){
            xform[i][j] = (double)(i==j);
            inverse_xform[i][j] = (double)(i==j);
        }
    buildIdentity(xform);
    buildIdentity(inverse_xform);
    inverse_needs_to_be_computed = vrpn_TRUE;
    inverse_valid = (d_src == d_dest);
}

void ImageTransformAffine::print()
{
    int i,j;
    printf("xform:\n");
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++)
            printf("%g ", xform[i][j]);
        printf("\n");
    }

    printf("inv_xform:\n");
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++)
            printf("%g ", inverse_xform[i][j]);
        printf("\n");
    }
}

void ImageTransformAffine::set(int i_dest, int i_src, double value) {
    assert(i_dest >= 0 && i_dest < 4 && i_src >= 0 && i_src < 4);
    xform[i_dest][i_src] = value;
    if (!inverse_needs_to_be_computed){
        printf("ImageTransformAffine::set: inverse dirty\n");
        inverse_needs_to_be_computed = vrpn_TRUE;
    }
}

void ImageTransformAffine::transform(double *p_src, double *p_dest) const {
    assert(dim_src > 0 && dim_src < 5 && dim_dest > 0 && dim_dest < 5);

    for (int i = 0; i < dim_dest; i++){
        p_dest[i] = 0;
        for (int j = 0; j < dim_src; j++){
            p_dest[i] += xform[i][j]*p_src[j];
        }
    }
}

void ImageTransformAffine::invTransform(double *p_src, double *p_dest) {
    assert(dim_src > 0 && dim_src < 5 && dim_dest > 0 && dim_dest < 5);
    if (!hasInverse()) {
        fprintf(stderr, "ImageTransformAffine::invTransform: Warning,"
               " failed use of inverse (non-invertible transform)\n");
        return;
    }
    printf("invTransform\n");
    print();

    double result[4] = {0,0,0,0};
    int i,j;
    for (i = 0; i < dim_dest; i++){
        for (j = 0; j < dim_src; j++){
            result[i] += inverse_xform[i][j]*p_src[j];
        }
    }
    for (i = 0; i < dim_dest; i++){
        p_dest[i] = result[i];
    }
}

ImageTransform *ImageTransformAffine::duplicate() const {
    ImageTransformAffine *ita = new ImageTransformAffine(dim_src, dim_dest);
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            ita->xform[i][j] = xform[i][j];
        }
    }
    return (ImageTransform *)ita;
}

void ImageTransformAffine::buildIdentity(double m[4][4])
{
    int i,j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            m[i][j] = (double)(i==j);
}


/* computeInverse() - adapted from code in Matrix44.cpp by Dave McAllister */
vrpn_bool ImageTransformAffine::computeInverse() {
    printf("before computeInverse\n");
    print();
    inverse_needs_to_be_computed = vrpn_FALSE;
    if (dim_src != dim_dest) {
       return vrpn_FALSE;
    }
    int i,j;
    double p[4][4];
    buildIdentity(p);
    // copy xform into inverse_xform
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++){
            inverse_xform[i][j] = xform[i][j];
        }
    }

    // Make it upper triangular using Gauss-Jordan with partial pivoting.
    for (i=0; i<4; i++) 
    {
        // Find largest row.
        double max=fabs(inverse_xform[i][i]);
        int row = i;
        int j;
        for (j=i+1; j<4; j++)
        {
            if (fabs(inverse_xform[j][i]) > max)
            {
                max = fabs(inverse_xform[j][i]);
                row = j;
            }
        }
        
        // Pivot around largest row.
        if (max <= 0) {
            return vrpn_FALSE;
        }
        if (row != i) {
            switch_rows(inverse_xform, i, row);
            switch_rows(p, i, row);
        }

        // Subtract scaled rows to eliminate column i.
        if (inverse_xform[i][i] == 0){
            // we're in trouble here
            return vrpn_FALSE;
        }
        double denom = 1./inverse_xform[i][i];
        for (j=i+1; j<4; j++)
        {
            double factor = inverse_xform[j][i] * denom;
            sub_rows(inverse_xform, j, i, factor);
            sub_rows(p, j, i, factor);
        }
    }

    // Diagonalize inverse_xform using Jordan.
    for (i=1; i<4; i++)
    {
        if (inverse_xform[i][i] == 0){
            // we're in trouble here
            return vrpn_FALSE;
        }
        double denom = 1./inverse_xform[i][i];
        for (int j=0; j<i; j++)
        {
            double factor = inverse_xform[j][i] * denom;
            sub_rows(inverse_xform, j, i, factor);
            sub_rows(p, j, i, factor);
        }
    }

    // Normalize inverse_xform to the identity and copy p over inverse_xform.
    for(i=0; i<4; i++)
    {
        if (inverse_xform[i][i] == 0){
            // we're in trouble here
            return vrpn_FALSE;
        }
        double factor = 1./inverse_xform[i][i];
        for (int j=0; j<4; j++)
        {
            // As if we were doing inverse_xform[i][j] *= factor
            p[i][j] *= factor;
            inverse_xform[i][j] = p[i][j];
        }
    }
    printf("end of computeInverse\n");
    print();

    return vrpn_TRUE;
}

vrpn_bool ImageTransformAffine::hasInverse() {
    if (inverse_needs_to_be_computed) {
        printf("computing inverse\n");
        inverse_valid = computeInverse();
    } else {
        printf("not computing inverse\n");
    }
    return inverse_valid;
}
