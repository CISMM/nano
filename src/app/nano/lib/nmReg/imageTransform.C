#include "imageTransform.h"

void ImageTransformAffine::transform(double *p_src, double *p_dest) const {
    assert(dim_src > 0 && dim_src < 5 && dim_dest > 0 && dim_dest < 5);

    for (int i = 0; i < dim_dest; i++){
        p_dest[i] = 0;
        for (int j = 0; j < dim_src; j++){
            p_dest[i] += xform[i][j]*p_src[j];
        }
    }
}

