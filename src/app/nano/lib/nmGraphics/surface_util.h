#ifndef SURFACE_UTIL_H
#define SURFACE_UTIL_H

#ifndef Q_INCLUDED
#include <quat.h>
#endif

class PPM;  // from PPM.h
class nmg_State;

void buildVisualizationTexture(nmg_State * state, 
                               int width, int height, unsigned char *texture);
void buildContourTexture (nmg_State * state);  // for update
void buildRemoteRenderedTexture (nmg_State * state, 
                                 int width, int height, void *);
void makeCheckImage (nmg_State * state);
void buildAlphaTexture (nmg_State * state);
void makeRulerImage (nmg_State * state);
void buildRulergridTexture (nmg_State * state);

void compute_texture_matrix(double translate_x, double translate_y,
                double rotation, double scale_x, double scale_y,
                double shear_x, double shear_y,
                double xform_center_tex_x, double xform_center_tex_y,
                double *mat);

#define CYGWIN_TEXTURE_FUNCTION GL_DECAL
// This seems to be a reasonable choice on a pc that doesn't handle alpha
// correctly (on my pc, GL_DECAL becomes GL_REPLACE) (AAS), 
// I changed some of the details of the way textures are generated so that
// they still look okay with this change instead of causing the surface
// to disappear - see
// makeRulerImage()
// buildAndInstallRulerImage()

#endif  // SURFACE_UTIL_H
