#ifndef NMG_TYPES_H
#define NMG_TYPES_H

#include "nmg_Vector.h"
#include "nmg_Point.h"
#include "nmg_RaySegment.h"

#if(1) 

typedef float nmg_Float;
typedef nmg_Vector_3f nmg_Vector;
typedef nmg_Point_3f nmg_Point;
typedef nmg_Ray_f nmg_Ray;
typedef nmg_RaySegment_f nmg_RaySegment;
typedef nmg_Edge_f nmg_Edge;

#else

typedef double nmg_Float;
typedef nmg_Vector_3d nmg_Vector;
typedef nmg_Point_3d nmg_Point;
typedef nmg_Ray_d nmg_Ray;
typedef nmg_RaySegment_d nmg_RaySegment;
typedef nmg_Edge_d nmg_Edge;

#endif

#endif
