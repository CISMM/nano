#include "GL/glut.h"
#include <stdio.h>
#include "patternEditor.h"
#include "nmr_ImageTransform.h"
#include "nmr_Util.h"

PatternEditor *pe = NULL;

const int TEX_W = 512;
const int TEX_H = 512;
const int CAP_W = 128;
const int CAP_H = 128;
static unsigned char texture[TEX_W*TEX_H*4];
static unsigned char linInterp[TEX_W*TEX_H*3];
static unsigned char screenCapture[CAP_W*CAP_H*3];
GLdouble tex_mat[] =
        {1.0, 0.0, 0.0, 0.0,
         0.0, 1.0, 0.0, 0.0,
         0.0, 0.0, 1.0, 0.0,
         0.0, 0.0, 0.0, 1.0};


int main(int argc, char **argv)
{

    int i,j;
    unsigned char red = 0, green = 0, blue = 0;
    for (i = 0; i < TEX_W; i++){
        for (j = 0; j < TEX_H; j++){
            red = 2; green = 2; blue = 2;
            if (j > 0 && j < TEX_H-5 && i%8 == 0){
               red = 255;
               green = 255;
               blue = 255;
            }
            if (i > 0 && i < TEX_W-5 && j%8 == 0){
               red = 255;
               green = 255;
               blue = 255;
            }
            texture[j*TEX_W*4+i*4] = red;
            texture[j*TEX_W*4+i*4+1] = green;
            texture[j*TEX_W*4+i*4+2] = blue;
            texture[j*TEX_W*4+i*4+3] = 255;
        }
    }

    nmb_ImageArray<float> *source, *target; 
    source = new nmb_ImageArray<float>("source", "", TEX_W, TEX_H);
    target = new nmb_ImageArray<float>("target", "", TEX_W, TEX_H);

    target->setBounds(nmb_ImageBounds(0.0, 0.0, 1.0, 1.0));

    for (i = 0; i < TEX_W; i++) {
       for (j = 0; j < TEX_H; j++){
         target->setValue(i,j,(float)(texture[j*TEX_W*4+i*4])/255.0);
       }
    }

    nmr_ImageTransformAffine xform(4,4);
    xform.setMatrix(tex_mat);
    nmr_Util::createResampledImageWithImageSpaceTransformation(
          *(nmb_Image *)target, xform,
          *(nmb_Image *)source);

    unsigned char val;
    for (i = 0; i < TEX_W; i++) {
       for (j = 0; j < TEX_H; j++){
          val = (unsigned char)(source->getValue(i,j));
          linInterp[j*TEX_W*3+i*3] = val;
          linInterp[j*TEX_W*3+i*3+1] = val;
          linInterp[j*TEX_W*3+i*3+2] = val;
       }
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    

    pe = new PatternEditor();

    pe->addImage(target);

    pe->show();

    glutMainLoop();
    return 0;
}

