#include "nmb_ImageTransform.h"

nmb_ImageTransformAffine::nmb_ImageTransformAffine(int d_src, int d_dest):
	nmb_ImageTransform(d_src, d_dest)
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

void nmb_ImageTransformAffine::print()
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

void nmb_ImageTransformAffine::set(int i_dest, int i_src, double value) {
    assert(i_dest >= 0 && i_dest < 4 && i_src >= 0 && i_src < 4);
    xform[i_dest][i_src] = value;
    if (!inverse_needs_to_be_computed){
//        printf("nmb_ImageTransformAffine::set: inverse dirty\n");
        inverse_needs_to_be_computed = vrpn_TRUE;
    }
}

void nmb_ImageTransformAffine::setMatrix(vrpn_float64 *matrix)
{
    int i,j,k = 0;
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            xform[i][j] = matrix[k];
            k++;
        }
    }
    inverse_needs_to_be_computed = vrpn_TRUE;
}

void nmb_ImageTransformAffine::getMatrix(vrpn_float64 *matrix)
{
/*
    matrix[0,1,2,3] = whatever we transform (1,0,0,0) into which is
    xform[0][0], xform[1][0], xform[2][0], xform[3][0]
    matrix[4,5,6,7] = whatever we transform (0,1,0,0) into which is
    xform[0][1], xform[1][1], xform[2][1], xform[3][1]
    ...
*/
    int i,j,k = 0;
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            matrix[k] = xform[i][j];
            k++;
        }
    }
}

void nmb_ImageTransformAffine::compose(nmb_ImageTransformAffine &m)
{
    int i,j,k;
    double p[4][4];
    for (i = 0; i < 4; i++){
        for (j = 0; j < 4; j++){
            p[i][j] = 0.0;
            for (k = 0; k < 4; k++){
                 p[i][j] += xform[k][j]*m.xform[i][k];
            }
        }
    }
    for (i = 0; i < 4; i++){
        for (j = 0; j < 4; j++){
            xform[i][j] = p[i][j];
        }
    }
    inverse_needs_to_be_computed = vrpn_TRUE;
}

void nmr_ImageTransformAffine::transform(double *p_src, double *p_dest) const {
    assert(dim_src > 0 && dim_src < 5 && dim_dest > 0 && dim_dest < 5);

    for (int i = 0; i < dim_dest; i++){
        p_dest[i] = 0;
        for (int j = 0; j < dim_src; j++){
            p_dest[i] += xform[i][j]*p_src[j];
        }
    }
}

void nmr_ImageTransformAffine::invTransform(double *p_src, double *p_dest) {
    assert(dim_src > 0 && dim_src < 5 && dim_dest > 0 && dim_dest < 5);
    if (!hasInverse()) {
        fprintf(stderr, "nmr_ImageTransformAffine::invTransform: Warning,"
               " failed use of inverse (non-invertible transform)\n");
        return;
    }
//    printf("invTransform\n");
//    print();

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

void nmr_ImageTransformAffine::invert() {
    assert(dim_src > 0 && dim_src < 5 && dim_dest > 0 && dim_dest < 5);
    if (!hasInverse()) {
        fprintf(stderr, "nmr_ImageTransformAffine::invert: Warning,"
               " failed use of invert (non-invertible transform)\n");
        return;
    }
    double swap;
    int i,j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            swap = xform[i][j];
            xform[i][j] = inverse_xform[i][j];
            inverse_xform[i][j] = swap;
        }
    }

}

nmr_ImageTransform *nmr_ImageTransformAffine::duplicate() const {
    nmr_ImageTransformAffine *ita = new nmr_ImageTransformAffine(dim_src, dim_dest);
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            ita->xform[i][j] = xform[i][j];
        }
    }
    return (nmr_ImageTransform *)ita;
}

void nmr_ImageTransformAffine::buildIdentity(double m[4][4])
{
    int i,j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            m[i][j] = (double)(i==j);
}


/* computeInverse() - adapted from code in Matrix44.cpp by Dave McAllister */
vrpn_bool nmr_ImageTransformAffine::computeInverse() {
//    printf("before computeInverse\n");
//    print();
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
//    printf("end of computeInverse\n");
//    print();

    return vrpn_TRUE;
}

vrpn_bool nmr_ImageTransformAffine::hasInverse() {
    if (inverse_needs_to_be_computed) {
//        printf("computing inverse\n");
        inverse_valid = computeInverse();
    } else {
//        printf("not computing inverse\n");
    }
    return inverse_valid;
}
