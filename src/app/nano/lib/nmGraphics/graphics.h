#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifndef Q_INCLUDED
#include <quat.h>
#endif

class PPM;  // from PPM.h

void setupMaterials (void);  // for initialization

void buildVisualizationTexture(int width, int height, unsigned char *texture);
void buildContourTexture (void);  // for update
void buildRemoteRenderedTexture (int width, int height, void *);
void makeCheckImage (void);
void buildAlphaTexture (void);
void makeRulerImage (void);
void buildRulergridTexture (void);

void compute_texture_matrix(double translate_x, double translate_y,
                double rotation, double scale_x, double scale_y,
                double shear_x, double shear_y,
                double xform_center_tex_x, double xform_center_tex_y,
                double *mat);

int setup_lighting (int nothing);

void setLightDirection (const q_vec_type &);
void getLightDirection (q_vec_type *);
void resetLightDirection (void);

void getViewportSize (int * width, int * height);

#define CYGWIN_TEXTURE_FUNCTION GL_DECAL
// This seems to be a reasonable choice on a pc that doesn't handle alpha
// correctly (on my pc, GL_DECAL becomes GL_REPLACE) (AAS), 
// I changed some of the details of the way textures are generated so that
// they still look okay with this change instead of causing the surface
// to disappear - see
// makeRulerImage()
// buildAndInstallRulerImage()

#endif  // GRAPHICS_H
