#ifndef _SIM_H_
#define _SIM_H_


#define NO_AFM 0
#define SEMI_SOLID_AFM 1
#define SOLID_AFM 2

typedef int Bool;
extern int tesselation;
extern int buttonpress;
extern int draw_objects;
extern int selected_triangle_side;
extern int afm_scan;
extern int mainWindowID, viewWindowID, depthWindowID;

extern double orthoFrustumLeftEdge;
extern double orthoFrustumBottomEdge;
extern double orthoFrustumWidth;
extern double orthoFrustumHeight;

extern float TipSize;

extern int uncertainty_mode;

#endif
