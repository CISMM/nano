#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifndef INCLUDED_V_H
#include <v.h>  // for GLfloat
#define INCLUDED_V_H
#endif

class PPM;  // from PPM.h

void setupMaterials (void);  // for initialization
void buildAllTextures (void);

void buildContourTexture (void);  // for update
void makeCheckImage (void);
void buildAlphaTexture (void);
void makeRulerImage (void);
void buildRulergridTexture (void);
void makeAndInstallRulerImage (PPM *);

void compute_texture_matrix(double translate_x, double translate_y,
                double rotation, double scale_x, double scale_y,
                double shear_x, double shear_y,
                double xform_center_tex_x, double xform_center_tex_y,
                double *mat);

int setup_lighting (int nothing);

void setLightDirection (const q_vec_type &);
void getLightDirection (q_vec_type *);
void resetLightDirection (void);

#define CYGWIN_TEXTURE_FUNCTION GL_BLEND 
// This seems to be a reasonable choice on a pc that doesn't handle alpha
// correctly (on my pc, GL_DECAL becomes GL_REPLACE) (AAS), 
// I changed some of the details of the way textures are generated so that
// they still look okay with this change instead of causing the surface
// to disappear - see
// makeRulerImage()
// buildAndInstallRulerImage()

#endif  // GRAPHICS_H
