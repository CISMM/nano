#include "imageTransform.h"

ImageTransformAffine::ImageTransformAffine(int d_src, int d_dest):
	ImageTransform(d_src, d_dest)
{
    int i,j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            xform[i][j] = (double)(i==j);
}

void ImageTransformAffine::set(int i_dest, int i_src, double value) {
    assert(i_dest >= 0 && i_dest < 4 && i_src >= 0 && i_src < 4);
    xform[i_dest][i_src] = value;
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

ImageTransform *ImageTransformAffine::duplicate() const {
    ImageTransformAffine *ita = new ImageTransformAffine(dim_src, dim_dest);
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            ita->xform[i][j] = xform[i][j];
        }
    }
    return (ImageTransform *)ita;
}
