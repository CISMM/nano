/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "surface_util.h"

#include <math.h>  // fabs()
#ifdef _WIN32
#include <windows.h>  // This must go before <GL/gl.h>
#endif
#include <GL/gl.h>
#if defined(sgi) || defined(__CYGWIN__)
  #include <GL/glu.h>
#endif

#include <quat.h>
#include <v.h>  // v_world et al.
#include <display.h>  // v_display_table

#include <BCGrid.h>
#include <BCPlane.h>
#include <PPM.h>

#include <nmb_Dataset.h>

#include "nmg_State.h"

#ifndef NMG_MIN
#define NMG_MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef NMG_MAX
#define NMG_MAX(a,b) ((a)<(b)?(b):(a))
#endif


/************************
 *  rulergrid stuff
 ***********************/
// sizes of texture maps (in bytes)
#define contourImageWidth 512

#define checkImageWidth 64
#define checkImageHeight 64
#define checkImageDepth 2

#define rulerImageHeight 64
#define rulerImageWidth  64

static GLubyte contourImage [contourImageWidth][4];
static GLubyte checkImage [checkImageDepth][checkImageWidth]
                          [checkImageHeight][4];
static GLubyte rulerImage [rulerImageHeight][rulerImageWidth][4];


// local functions:
//   void createPyramid (float center, float width, int white_flag)
//   void makeTexture (void)


/********************************************************

Renee Maheshwari 4/16/97

 This is a helper function for makeTexture.  It creates a truncated
pyramid of opacity values centered around the center parameter, with 
a base width of width.  The height is in terms of opacity, so the apex has
opacity value of 255.  The equation for the sides of a pyramid of 
height 255 and width width is y=(255/(width/2))*x + 255 for the left side
and y = -(255/(width/2))*x + 255 for the right side

\     /\     /\     /\    /\    /
 \   /  \   /  \   /  \  /  \  /
|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|

each vertical line is a texel, the apex of the triangle is where the
red or white line should fall on the texture map, so nearby texels
opacity values must be adjusted to create the correct map 

*****************************************/
static void createPyramid (float center, float width, int white_flag)
{
  float slope, /* start, end, */ i, opacity, translation;
  //one side of the triangle spans width/2 in x
  slope = 255 / (width / 2);
  // start = ceil(center - width / 2);
  // end = floor(center + width / 2);

  /* compute heights based on pyramid at origin, so i goes from
    -width/2 to width/2 */
  for (i = -width / 2; i <= width / 2; i = i + 1)
    {
      //need to flip the sign of the slope when we cross zero
      if (i > 0 && slope > 0)
	{
	  slope = -slope;
	}

      //calculate the opacity for this pixel based on the slope
      opacity = slope * i + 255;

      //clamp the opacity to 255
      opacity = NMG_MIN(opacity, 255.0f);

      //translate the value to center
      //use mod so we dont fall off the edge of the contourImage
      translation = (int)(center + i) % contourImageWidth;

      //the mod does not return a positive value if you have a neg value
      //so need to compensate for that by adding the translation (a neg value)
      //to the ImageWidth, so we "wrap around" the texture map
      if (translation < 0)
	translation = contourImageWidth + translation;
//#ifndef _WIN32
#if 1
      contourImage[(int) translation][3] = (int) opacity;
#else
      contourImage[(int) translation][3] = 255;
      contourImage[(int)(translation)][0] = (int)
	(state->contour_r*(float)opacity/255.0);
      contourImage[(int)(translation)][1] = (int)
	(state->contour_g*(float)opacity/255.0);
      contourImage[(int)(translation)][2] = (int)
	(state->contour_b*(float)opacity/255.0);
//        contourImage[(int)(translation)][0] = (int)
//  	((float)contourImage[(int)(translation)][0]*(float)opacity/255.0);
//        contourImage[(int)(translation)][1] = (int)
//  	((float)contourImage[(int)(translation)][1]*(float)opacity/255.0);
//        contourImage[(int)(translation)][2] = (int)
//  	((float)contourImage[(int)(translation)][2]*(float)opacity/255.0);
#endif
      //if this is the white line, overwrite the white color
      if (white_flag == 1)
	{
	  contourImage[(int)(translation)][0] = 255;
	  contourImage[(int)(translation)][1] = 255;
	  contourImage[(int)(translation)][2] = 255;
	}
    }
}
  
// Build the 1-dimensional texture map (including Alpha) that is to be used
// to draw contours.  This is a red map, with 10 stripes on it.  The last
// stripe is white.  This means that the step from white mark to white mark
// is 1 texture_scape unit and from red to red is 0.1 units.

// globals:
//  float state->contour_width
//  int state->contour_[rgb]

static void makeTexture (nmg_State * state) {

  // num holds the (non-integer) number of texels between red lines
  double num = contourImageWidth / 10.0;

  double k;
  int i, j;

  int count = 1;		// Which red line we are on
  float width = state->contour_width;// Width of the lines in percentage
  //convert percentage to actual pixel values
  width = (width / 100) * (contourImageWidth / 10);

  // k holds the (non-integer) starting point for the red line we are currently
  // trying to draw.
  k = (float) count * num ;

  //printf("num: %f k: %f\n", num, k);

  // Fill the image with unsaturated red and completely transparent
  for(i = 0; i < contourImageWidth; i++) {
//#ifndef _WIN32
#if 1
     contourImage[i][0] = state->contour_r;
     contourImage[i][1] = state->contour_g;
     contourImage[i][2] = state->contour_b;
     contourImage[i][3] = 0;
#else
     contourImage[i][0] = 0;
     contourImage[i][1] = 0;
     contourImage[i][2] = 0;
     contourImage[i][3] = 255;
#endif
  }

  //fill in the correct pixels for each of the ten lines
  for (j=0; j<10; j++)
    {
      k = (float)j*num;
      //create the white line
      if (j == 0)
	{
	  createPyramid(k, width, 1);
	}
      //create the red lines
      else
	{
	  createPyramid(k, width, 0);
	}
    }

}


// globals:
//  state->alpha_[rgb]

void makeCheckImage (nmg_State * state)
{
    int i, j, c;
     
    //fprintf(stderr, "In makeCheckImage().\n");

    for (i = 0; i < checkImageHeight; i++) {
        for (j = 0; j < checkImageWidth; j++) {

            // [juliano 2/2000]: the next line is correct.  It's taken out of
            // the OpenGL red book.  It relies on bool being represented as
            // the nubmer one, which is guaranteed by the C and C++ standards.
            // Semantically equivalent and faster (but untested) would be:
            //
            //   !(((i & 0x4) ^ (j & 0x4)) >>2)
            //
            // which wouldn't generate a warning and would be faster.  But,
            // since this change is untested, I'll leave it alone for now.
            c = ((i & 0x4) == 0) ^ ((j & 0x4) == 0);

            checkImage[0][i][j][0] = (GLubyte) (c * state->alpha_r * 255);
            checkImage[0][i][j][1] = (GLubyte) (c * state->alpha_g * 255);
            checkImage[0][i][j][2] = (GLubyte) (c * state->alpha_b * 255);
            checkImage[0][i][j][3] = 0;

            checkImage[1][i][j][0] = (GLubyte) (c * state->alpha_r * 255);
            checkImage[1][i][j][1] = (GLubyte) (c * state->alpha_g * 255);
            checkImage[1][i][j][2] = (GLubyte) (c * state->alpha_b * 255);
            checkImage[1][i][j][3] = (GLubyte) c * 255;
        }
    }
}

// globals:
//  state->ruler_[rgb]
//  state->ruler_width_[xy]
//  state->ruler_opacity

// HACK
//   make sure ruler_[rgb] is initially set to ruler_color_rgb


void makeRulerImage (nmg_State * state) {

  int rwidth_x, rwidth_y;
  int i, j;
  
//#ifndef _WIN32
#if 1
  // Colors are OK at this point
  //printf("mkRulerImage %d %d %d\n", state->ruler_r, state->ruler_g, state->ruler_b);
  for (i = 0;i < rulerImageHeight; i++) {
    for (j = 0;j < rulerImageWidth; j++) {
      rulerImage[i][j][0] = state->ruler_r;
      rulerImage[i][j][1] = state->ruler_g;
      rulerImage[i][j][2] = state->ruler_b;
      //rulerImage[i][j][3] = 0;
    }
  }

  //   convert percentages to actual texel values
  // use 200 because width is adjusted from both ends
  // use ceil so scaling less than 1 gets rounded up
  rwidth_x = (int)(ceil((state->ruler_width_x / 200) * rulerImageWidth));
  rwidth_y = (int)(ceil((state->ruler_width_y / 200) * rulerImageHeight));

  // draw vertical lines
  for (i = 0; i < rulerImageHeight; i++)
    for (j = 0; j < rulerImageWidth; j++)
      if ((j < rwidth_x) ||
          (rwidth_x > (rulerImageWidth - 1 - j)))
        rulerImage[i][j][3] = (GLubyte)state->ruler_opacity;
      else 
        rulerImage[i][j][3] = 0;
 
  // draw horizontal lines
  for (j = 0; j < rulerImageWidth; j++)
    for (i = 0; i < rulerImageHeight; i++)
      if (i < rwidth_y) {
        rulerImage[i][j][3] = (GLubyte)state->ruler_opacity;
        rulerImage[rulerImageHeight - 1 - i][j][3] = (GLubyte)state->ruler_opacity;
      }

#else	// CYGWIN
  // here we assume that alpha-blending doesn't work and that we are using
  // the texture rasterization function BLEND instead of DECAL

  for (i = 0;i < rulerImageHeight; i++) {
    for (j = 0;j < rulerImageWidth; j++) {
      rulerImage[i][j][3] = 255;
    }
  }

  //   convert percentages to actual texel values
  // use 200 because width is adjusted from both ends
  // use ceil so scaling less than 1 gets rounded up
  rwidth_x = (int)(ceil((state->ruler_width_x / 200) * rulerImageWidth));
  rwidth_y = (int)(ceil((state->ruler_width_y / 200) * rulerImageHeight));

  // draw vertical and horizontal lines
  for (i = 0; i < rulerImageHeight; i++)
    for (j = 0; j < rulerImageWidth; j++)
      if ((j < rwidth_x) ||
          (rwidth_x > (rulerImageWidth - 1 - j)) || (i < rwidth_y)||
          (rwidth_y > (rulerImageHeight - 1 - i)) ){
	rulerImage[i][j][0] = (GLubyte)((float)state->ruler_r*state->ruler_opacity/255.0);
	rulerImage[i][j][1] = (GLubyte)((float)state->ruler_g*state->ruler_opacity/255.0);
	rulerImage[i][j][2] = (GLubyte)((float)state->ruler_b*state->ruler_opacity/255.0);
      }
      else {
	rulerImage[i][j][0] = 0;
	rulerImage[i][j][1] = 0;
	rulerImage[i][j][2] = 0;
      }

#endif

}

void buildRemoteRenderedTexture (nmg_State * state, 
                                 int width, int height, void * tex) {
//fprintf(stderr, "Building remotely rendered texture.\n");
  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();
  state->remoteDataTexture.installTexture(width, height, tex, GL_RGBA, GL_RGBA,
	  GL_UNSIGNED_BYTE, GL_CLAMP);
}



void buildContourTexture (nmg_State * state) {

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  makeTexture(state);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); 

  glBindTexture(GL_TEXTURE_1D, state->contourTextureID);

  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#if defined(sgi) || defined(_WIN32)
  if (gluBuild1DMipmaps(GL_TEXTURE_1D, 4, contourImageWidth,
                        GL_RGBA, GL_UNSIGNED_BYTE, contourImage)!=0) {
      fprintf(stderr, "There is error in constructing the mipmaps.  "
                      "Texture Image is used instead\n");
      glTexImage1D(GL_TEXTURE_1D, 0, 4, contourImageWidth, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, contourImage);
  }
  if (glGetError()!=GL_NO_ERROR) {
    fprintf(stderr, "Error creating contour texture image\n");
  }
#endif
}

void buildVisualizationTexture(nmg_State * state, int width, int height, unsigned char *texture) {
// make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  state->visualizationTexture.installTexture(width, height, texture, 4, GL_RGB, 
	  GL_UNSIGNED_BYTE, GL_REPEAT);
}

void buildRulergridTexture (nmg_State * state) {
  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  state->rulergridTexture.installTexture(rulerImageWidth, rulerImageHeight, rulerImage,
	  4, GL_RGBA, GL_UNSIGNED_BYTE, GL_REPEAT);
}

void buildAlphaTexture (nmg_State * state) {

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  /* GL_TEXTURE_3D_EXT is not available on other platforms... i.e. WIN32 */
#if defined(sgi)
  glBindTexture(GL_TEXTURE_3D, state->tex_ids[ALPHA_3D_TEX_ID]);

  glTexParameterf(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R_EXT, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_3D_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_3D_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
  /* the GL_RGBA8_EXT isn't working on flow */
  glTexImage3DEXT(GL_TEXTURE_3D_EXT, 0, GL_RGBA8_EXT,
          checkImageWidth, checkImageHeight, checkImageDepth,
          0, GL_RGBA, GL_UNSIGNED_BYTE, &checkImage[0][0][0][0]);
#endif
  if (glGetError() != GL_NO_ERROR) {
          fprintf(stderr, "error in making checker board texture.\n");
  }
}



// rotation is in degrees
void compute_texture_matrix(double translate_x, double translate_y,
		double rotation, double scale_x, double scale_y,
 		double shear_x, double shear_y,
		double xform_center_tex_x, double xform_center_tex_y,
		double *mat)
{
    double sinA = sin(-rotation);
    double cosA = cos(-rotation);

/*
    mat[0 + 0*4] = cosA/scale_x + sinA*(-shear_x)/scale_y;
    mat[0 + 1*4] = (-sinA)/scale_x + -shear_x*cosA/scale_y;
    mat[0 + 2*4] = 0.0;
    mat[0 + 3*4] = xform_center_tex_x +
	(cosA/scale_x + sinA*(-shear_x)/scale_y)*
		(translate_x - xform_center_model_x) +
	((-sinA)/scale_x + -shear_x*cosA/scale_y)*
		(translate_y - xform_center_model_y); 
    mat[1 + 0*4] = cosA*(-shear_y)/scale_x + sinA/scale_y;
    mat[1 + 1*4] = (cosA)/scale_y + (-sinA)*(-shear_y)/scale_x;
    mat[1 + 2*4] = 0.0;
    mat[1 + 3*4] = xform_center_tex_y +
	(cosA*(-shear_y)/scale_x + sinA/scale_y)*
		(translate_x - xform_center_model_x) +
	((cosA)/scale_y + (-sinA)*(-shear_y)/scale_x)*
		(translate_y - xform_center_model_y);
    mat[2 + 0*4] = 0.0;
    mat[2 + 1*4] = 0.0;
    mat[2 + 2*4] = 1.0;
    mat[2 + 3*4] = 0.0;
    mat[3 + 0*4] = 0.0;
    mat[3 + 1*4] = 0.0;
    mat[3 + 2*4] = 0.0;
    mat[3 + 3*4] = 1.0;
*/

// DEBUG7299 - this works without the ugliness above now that I fixed stuff 
// in other parts of the program
    mat[0 + 0*4] = cosA/scale_x + sinA*(-shear_x)/scale_y;
    mat[0 + 1*4] = (-sinA)/scale_x + -shear_x*cosA/scale_y;
    mat[0 + 2*4] = 0.0;
    mat[0 + 3*4] = xform_center_tex_x +
        (cosA/scale_x + sinA*(-shear_x)/scale_y)*
                (-translate_x) +
        ((-sinA)/scale_x + -shear_x*cosA/scale_y)*
                (-translate_y);
    mat[1 + 0*4] = cosA*(-shear_y)/scale_x + sinA/scale_y;
    mat[1 + 1*4] = (cosA)/scale_y + (-sinA)*(-shear_y)/scale_x;
    mat[1 + 2*4] = 0.0;
    mat[1 + 3*4] = xform_center_tex_y +
        (cosA*(-shear_y)/scale_x + sinA/scale_y)*
                (-translate_x) +
        ((cosA)/scale_y + (-sinA)*(-shear_y)/scale_x)*
                (-translate_y);
    mat[2 + 0*4] = 0.0;
    mat[2 + 1*4] = 0.0;
    mat[2 + 2*4] = 1.0;
    mat[2 + 3*4] = 0.0;
    mat[3 + 0*4] = 0.0;
    mat[3 + 1*4] = 0.0;
    mat[3 + 2*4] = 0.0;
    mat[3 + 3*4] = 1.0;


    return;
}


// Local Variables:
// mode:c++
// End:
