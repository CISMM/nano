#ifndef _SIM_H_
#define _SIM_H_

#include "3Dobject.h"

#define MAXOBS 1000
#define MAXVERTICES 10000
#define MAXGROUPS 100

#define NO_AFM 0
#define SEMI_SOLID_AFM 1
#define SOLID_AFM 2

typedef int Bool;
extern int selectedOb;
extern OB *ob[MAXOBS];
extern int numObs;
extern OB **group_of_obs[MAXGROUPS];
extern int numGroups;
extern int NANOTUBE_GROUP;
extern int number_in_group[MAXGROUPS];
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
