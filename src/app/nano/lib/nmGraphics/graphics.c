#include "graphics.h"

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

#include "graphics_globals.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)<(b)?(b):(a))
#endif

//switching for textures on sgi and FLOW
#ifdef sgi
#define sgi_or_flow 1
#endif

#ifdef FLOW
#define sgi_or_flow 1
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

// default position of the light:  overhead, at infinity
static GLfloat l0_position [4] = { 0.0, 1.0, 1.0, 0.0 };

static GLubyte contourImage [contourImageWidth][4];
static GLubyte checkImage [checkImageDepth][checkImageWidth]
                          [checkImageHeight][4];
static GLubyte rulerImage [rulerImageHeight][rulerImageWidth][4];


#ifdef FLOW
  #include <util/aux/kernel.h>
  #include <util/aux/image.h>
  #include <util/aux/aperror.h>
  #include <pxapi/pxfl.h>
#endif


// local functions:
//   int vec_cmp (const q_vec_type, const q_vec_type)
//   void createPyramid (float center, float width, int white_flag)
//   void makeTexture (void)
//   void makeCheckImage (void)
//   void makeRulerImage (void)




/* Quick function to compare two vectors since this isn't in quatlib.
 * Probably should be added there.
 */
int vec_cmp(q_vec_type a, q_vec_type b) {
   return ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]));
}





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
void createPyramid (float center, float width, int white_flag)
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
      opacity = min(opacity, 255.0f);

      //translate the value to center
      //use mod so we dont fall off the edge of the contourImage
      translation = (int)(center + i) % contourImageWidth;

      //the mod does not return a positive value if you have a neg value
      //so need to compensate for that by adding the translation (a neg value)
      //to the ImageWidth, so we "wrap around" the texture map
      if (translation < 0)
	translation = contourImageWidth + translation;
#ifndef __CYGWIN__
      contourImage[(int) translation][3] = (int) opacity;
#else
      contourImage[(int) translation][3] = 255;
      contourImage[(int)(translation)][0] = (int)
	((float)contourImage[(int)(translation)][0]*(float)opacity/255.0);
      contourImage[(int)(translation)][1] = (int)
	((float)contourImage[(int)(translation)][1]*(float)opacity/255.0);
      contourImage[(int)(translation)][2] = (int)
	((float)contourImage[(int)(translation)][2]*(float)opacity/255.0);
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
//  float g_contour_width
//  int g_contour_[rgb]

void makeTexture (void) {

  // num holds the (non-integer) number of texels between red lines
  double num = contourImageWidth / 10.0;

  double k;
  int i, j;

  int count = 1;		// Which red line we are on
  float width = g_contour_width;// Width of the lines in percentage
  //convert percentage to actual pixel values
  width = (width / 100) * (contourImageWidth / 10);

  // k holds the (non-integer) starting point for the red line we are currently
  // trying to draw.
  k = (float) count * num ;

  //printf("num: %f k: %f\n", num, k);

  // Fill the image with unsaturated red and completely transparent
  for(i = 0; i < contourImageWidth; i++) {
     contourImage[i][0] = g_contour_r;
     contourImage[i][1] = g_contour_g;
     contourImage[i][2] = g_contour_b;
#ifndef __CYGWIN__
     contourImage[i][3] = 0;
#else
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
//  g_alpha_[rgb]

void makeCheckImage (void)
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

            checkImage[0][i][j][0] = (GLubyte) (c * g_alpha_r * 255);
            checkImage[0][i][j][1] = (GLubyte) (c * g_alpha_g * 255);
            checkImage[0][i][j][2] = (GLubyte) (c * g_alpha_b * 255);
            checkImage[0][i][j][3] = 0;

            checkImage[1][i][j][0] = (GLubyte) (c * g_alpha_r * 255);
            checkImage[1][i][j][1] = (GLubyte) (c * g_alpha_g * 255);
            checkImage[1][i][j][2] = (GLubyte) (c * g_alpha_b * 255);
            checkImage[1][i][j][3] = (GLubyte) c * 255;
        }
    }
}

// globals:
//  g_ruler_[rgb]
//  g_ruler_width_[xy]
//  g_ruler_opacity

// HACK
//   make sure ruler_[rgb] is initially set to ruler_color_rgb


void makeRulerImage (void) {

  int rwidth_x, rwidth_y;
  int i, j;
  
#ifndef __CYGWIN__
  // Colors are OK at this point
  //printf("mkRulerImage %d %d %d\n", g_ruler_r, g_ruler_g, g_ruler_b);
  for (i = 0;i < rulerImageHeight; i++) {
    for (j = 0;j < rulerImageWidth; j++) {
      rulerImage[i][j][0] = g_ruler_r;
      rulerImage[i][j][1] = g_ruler_g;
      rulerImage[i][j][2] = g_ruler_b;
      //rulerImage[i][j][3] = 0;
    }
  }

  //   convert percentages to actual texel values
  // use 200 because width is adjusted from both ends
  // use ceil so scaling less than 1 gets rounded up
  rwidth_x = (int)(ceil((g_ruler_width_x / 200) * rulerImageWidth));
  rwidth_y = (int)(ceil((g_ruler_width_y / 200) * rulerImageHeight));

  // draw vertical lines
  for (i = 0; i < rulerImageHeight; i++)
    for (j = 0; j < rulerImageWidth; j++)
      if ((j < rwidth_x) ||
          (rwidth_x > (rulerImageWidth - 1 - j)))
        rulerImage[i][j][3] = (GLubyte)g_ruler_opacity;
      else 
        rulerImage[i][j][3] = 0;
 
  // draw horizontal lines
  for (j = 0; j < rulerImageWidth; j++)
    for (i = 0; i < rulerImageHeight; i++)
      if (i < rwidth_y) {
        rulerImage[i][j][3] = (GLubyte)g_ruler_opacity;
        rulerImage[rulerImageHeight - 1 - i][j][3] = (GLubyte)g_ruler_opacity;
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
  rwidth_x = (int)(ceil((g_ruler_width_x / 200) * rulerImageWidth));
  rwidth_y = (int)(ceil((g_ruler_width_y / 200) * rulerImageHeight));

  // draw vertical and horizontal lines
  for (i = 0; i < rulerImageHeight; i++)
    for (j = 0; j < rulerImageWidth; j++)
      if ((j < rwidth_x) ||
          (rwidth_x > (rulerImageWidth - 1 - j)) || (i < rwidth_y)){
	rulerImage[i][j][0] = (GLubyte)((float)g_ruler_r*g_ruler_opacity/255.0);
	rulerImage[i][j][1] = (GLubyte)((float)g_ruler_g*g_ruler_opacity/255.0);
	rulerImage[i][j][2] = (GLubyte)((float)g_ruler_b*g_ruler_opacity/255.0);
      }
      else {
	rulerImage[i][j][0] = 0;
	rulerImage[i][j][1] = 0;
	rulerImage[i][j][2] = 0;
      }

#endif

}

void buildRemoteRenderedTexture (int width, int height, void * tex) {
  GLenum errval;
//fprintf(stderr, "Building remotely rendered texture.\n");

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  glBindTexture(GL_TEXTURE_2D, tex_ids[RULERGRID_TEX_ID]);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifdef __CYGWIN__
  float tex_color[4] = {1.0, 1.0, 1.0, 1.0};
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, CYGWIN_TEXTURE_FUNCTION);
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, tex_color);
#else
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
#endif

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

#if defined(sgi) || defined(__CYGWIN__)
  //retval = gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width,
                             //height, GL_RGBA,
                             //GL_UNSIGNED_BYTE, tex);
  //if (retval) {
    //fprintf(stderr, " Didn't make mipmaps, using texture instead.\n");
    while ((errval = glGetError()) != GL_NO_ERROR) {
      fprintf(stderr, " Error before making ruler texture: %s.\n", gluErrorString(errval));
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 tex);
    while ((errval = glGetError()) != GL_NO_ERROR) {
      fprintf(stderr, " Error making ruler texture: %s.\n", gluErrorString(errval));
    }
  //}
#endif 

  g_tex_image_width[RULERGRID_TEX_ID] = 
           g_tex_installed_width[RULERGRID_TEX_ID] = width;
  g_tex_image_height[RULERGRID_TEX_ID] = 
           g_tex_installed_height[RULERGRID_TEX_ID] = height;
}



void buildContourTexture (void) {

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  makeTexture();

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifdef __CYGWIN__
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, CYGWIN_TEXTURE_FUNCTION);
#else
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); 
#endif
  glBindTexture(GL_TEXTURE_1D, tex_ids[CONTOUR_1D_TEX_ID]);

  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#if defined(sgi) || defined(__CYGWIN__)
  if (gluBuild1DMipmaps(GL_TEXTURE_1D, 4, contourImageWidth,
                        GL_RGBA, GL_UNSIGNED_BYTE, contourImage)!=0) {
      fprintf(stderr, "There is error in constructing the mipmaps.  "
                      "Texture Image is used instead\n");
      glTexImage1D(GL_TEXTURE_1D, 0, 4, contourImageWidth, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, contourImage);
  }
  if (glGetError()!=GL_NO_ERROR) {
    fprintf(stderr, "what a mess, man\n");
  }
#endif
        /********************88888888888_____________________
#ifdef FLOW
  glTexImage1D(GL_TEXTURE_1D, 0, 4, contourImageWidth, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, contourImage);
  if (glGetError()!=GL_NO_ERROR) {
    fprintf(stderr, "what a mess, man\n");
  }
#endif
_______________________________________*/
}



void buildRulergridTexture (void) {
  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  printf("building rulergrid texture\n");
  glBindTexture(GL_TEXTURE_2D, tex_ids[RULERGRID_TEX_ID]);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifdef __CYGWIN__
  float tex_color[4] = {1.0, 1.0, 1.0, 1.0};
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, CYGWIN_TEXTURE_FUNCTION);
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, tex_color);
#else
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
#endif

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

#if defined(sgi) || defined(__CYGWIN__)
  if (gluBuild2DMipmaps(GL_TEXTURE_2D, 4, rulerImageWidth,
                       rulerImageHeight, GL_RGBA,
                       GL_UNSIGNED_BYTE, rulerImage)!=0) { 
    fprintf(stderr, " Didn't make mipmaps, using texture instead.\n");
    glTexImage2D(GL_TEXTURE_2D, 0, 4,
                 rulerImageWidth, rulerImageHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 rulerImage);
    if (glGetError() != GL_NO_ERROR)
      fprintf(stderr, " Error making ruler texture.\n");
  }
#endif 
#ifdef FLOW
  /****************____________________________
  glTexImage2D(GL_TEXTURE_2D, 0, 4,
               rulerImageWidth, rulerImageHeight,
               0, GL_RGBA, GL_UNSIGNED_BYTE,
               rulerImage);
  if (glGetError() != GL_NO_ERROR)
    fprintf(stderr, " Error making ruler texture.\n");
  else
    fprintf(stderr, "made ruler textures\n");
___________________**********************/
#endif

}

void buildAlphaTexture (void) {

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifdef __CYGWIN__
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, CYGWIN_TEXTURE_FUNCTION);
#else
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
#endif 
  /* GL_TEXTURE_3D_EXT is not available on other platforms... i.e. WIN32 */
#if defined(sgi)
  glBindTexture(GL_TEXTURE_3D, tex_ids[ALPHA_3D_TEX_ID]);

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
#ifndef FLOW
  if (glGetError() != GL_NO_ERROR) {
          fprintf(stderr, "error in making checker board texture.\n");
  }
#endif
}



void setupMaterials (void) {

#ifdef FLOW
// This sets up the material parameters for the programmable shader that
// we are using.
// XXX Dont know if the setup_lighting() calls actually do anything
// anymore.  Since the picture is over-specular when we raise the specular
// component below, probably they do not.
//
// XXX Since the phong shader was not working properly when we needed to
// get an image, we set the material properties and just use them, ignoring
// the openGL lighting specifications.  We need to get a real shader that
// does this (should call Phong routine?)
    {
    /********************************************************************/
    /* Select the PxFl programmable shader here */
    /********************************************************************/
    GLfloat mat_specular[3] = { 0.4, 0.4, 0.4 };
    GLfloat phong_shininess[1] = {64.0};
    GLuint loadh;
    int l;
    char filename[256];

    fprintf(stderr,"\n*** Setting up PxFl programmable shader...***\n\n");

    // make sure gl calls are directed to the right context
    v_gl_set_context_to_vlib_window();

    glEnable(GL_TEXTURE_2D);

    loadh = glLoadExtensionCodeEXT(GL_SHADER_FUNCTION_EXT,"/gouraud");
    nM_diffuse = glNewShaderEXT(loadh);
    glEndShaderEXT();

    char shadername[15] = "nMShader";
    loadh = glLoadExtensionCodeEXT(GL_SHADER_FUNCTION_EXT, shadername);
    fprintf(stderr,"DEBUG: loading shader %s.\n", shadername);

    nM_shader = glNewShaderEXT(loadh);

    // print out the shader parameters
    {
      GLuint  num_params;
      GLenum *pname_list;

      num_params = glGetNumMaterialParametersEXT(nM_shader);
      pname_list = new GLenum[num_params];
      glGetMaterialParametersEXT(nM_shader, pname_list);

      fprintf(stderr, "Params for shader '%s' = \n", shadername);
      for ( GLuint k=0; k<num_params; k++ ) {
        fprintf(stderr, "\t%s\n",
                glGetMaterialParameterStringEXT(pname_list[k]));
      }
    }

    // normal, color, texture coordinates, and position will vary
    glShaderParameterBindingEXT(
            glGetMaterialParameterNameEXT("px_material_diffuse"),
            GL_MATERIAL_EXT);
    glShaderParameterBindingEXT(
            glGetMaterialParameterNameEXT("px_material_normal"),
            GL_MATERIAL_EXT);
    glShaderParameterBindingEXT(
            glGetMaterialParameterNameEXT("px_shader_texcoord"),
            GL_MATERIAL_EXT);
    glEndShaderEXT();

    // ********** Set the uniform parameters of the shader ***************

    // texture ids. you have to pass floats
    float j[N_SHADER_TEX];
    for (l = 0; l < N_SHADER_TEX; l++)
      j[l] = shader_tex_ids[l];
    glBoundMaterialfvEXT(nM_shader,
                         glGetMaterialParameterNameEXT("tex_id"),
                         j);

    // active mask
    glBoundMaterialiEXT(nM_shader,
                        glGetMaterialParameterNameEXT("active_mask"),
                        shader_mask);

    // for time varying stuff
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("framenum"),
                        px_framenum);

    //=====Bump Shader=====
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("bumpiness"),
                        bumpiness);
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("bump_pulserate"),
                        bump_pulserate);
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("bump_scale"),
                        bump_scale);

    //=====Pattern Blen Shader=====
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("pattern_blend"),
                        pattern_blend);
    glBoundMaterialfvEXT(nM_shader,
                         glGetMaterialParameterNameEXT("pattern_blend_color"),
                         pattern_color);
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("pattern_scale"),
                        pattern_scale);
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("pattern_pulserate"),
                        pattern_pulserate);

    //=====Spot Noise Shader
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("spotnoise_tex_size"),
                        spotnoise_tex_size);
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("spot_pulserate"),
                        spot_pulserate);
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("spot_blend"),
                        spot_blend);
    glBoundMaterialiEXT(nM_shader,
                        glGetMaterialParameterNameEXT("whichspot"),
                        whichspot);
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("contrast"),
                        contrast);
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("rotate_amount"),
                        rotate_amount);
    glBoundMaterialiEXT(nM_shader,
                        glGetMaterialParameterNameEXT("swap_kernel"),
                        swap_kernel);
    glBoundMaterialiEXT(nM_shader,
                        glGetMaterialParameterNameEXT("kernel_size"),
                        kernel_size);
    glBoundMaterialiEXT(nM_shader,
                        glGetMaterialParameterNameEXT("dense"),
                        dense);
    glBoundMaterialfEXT(nM_shader,
                        glGetMaterialParameterNameEXT("wobblerate"),
                        wobblerate);

    //VERBOSE(1, "Finished loading shaders here\n");

    // *************************** texture mode 
      g_texture_mode = GL_TEXTURE_2D;

    }
#endif  //end FLOW specific stuff

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



//---------------------------------------------------------------------------
// This routine sets up the lighting and some of the surface material
// properties.  It should be the one used in both openGL and PixelFlow.
// Some effort should be made to bring all of the material parameters
// together here.

int setup_lighting (int)
{
    static	int	was_smooth_shading = -1;

    GLfloat l0_ambient[4] = { 0.2, 0.2, 0.2, 1.0 };
//    GLfloat l0_diffuse[4] = { 0.4, 0.4, 0.4, 1.0 };
    GLfloat l0_diffuse[4] = { g_diffuse, g_diffuse, g_diffuse, 1.0 };
/*     GLfloat l0_specular[4] = { 0.4, 0.4, 0.4, 1.0 }; */
    GLfloat l0_specular[4] = { 0.2, 0.2, 0.2, 1.0 };
    // l0_position defined at the top of this file as a global variable

    // make sure gl calls are directed to the right context
    v_gl_set_context_to_vlib_window();

    glLightfv(GL_LIGHT0, GL_AMBIENT, l0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, l0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, l0_specular);
    glPushMatrix();
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, l0_position);
    glPopMatrix();
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0);

    if (g_config_filled_polygons) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

#ifndef FLOW
    if (g_config_smooth_shading != was_smooth_shading) {
	was_smooth_shading = g_config_smooth_shading;
	if (g_config_smooth_shading) {
	    glShadeModel(GL_SMOOTH);	/* Gouraud shading */
	} else {
	    glShadeModel(GL_FLAT);	/* Flat shading */
	}
    }
#endif

  if (!g_PRERENDERED_COLORS && !g_PRERENDERED_TEXTURE) {
    // With prerendered colors we don't have any normals;  this REALLY
    // slows us down, since GL_NORMALIZE special-cases that.
    glEnable(GL_NORMALIZE);                 /* Re-Normalize normals */
  }

  // No default ambient lighting other than specified in the light
  {       
	GLfloat global_ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
  }
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glEnable(GL_LIGHT0);

  // Is this reasonable?  TCH 10 Jan 99
  if (g_PRERENDERED_COLORS || g_PRERENDERED_TEXTURE) {
    glDisable(GL_LIGHTING);
  } else {
    glEnable(GL_LIGHTING);
  }

    return 0;
}

void setLightDirection (const q_vec_type & newValue) {
  l0_position[0] = newValue[0];
  l0_position[1] = newValue[1];
  l0_position[2] = newValue[2];
}

void getLightDirection (q_vec_type * v) {
  (*v)[0] = l0_position[0];
  (*v)[1] = l0_position[1];
  (*v)[2] = l0_position[2];
}

// Put the light back where it was when the program started.

void resetLightDirection (void) {
  l0_position[0] = 0.0;
  l0_position[1] = 1.0;
  l0_position[2] = 0.5;
  l0_position[3] = 0.0;
}

