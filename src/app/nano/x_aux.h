#ifndef X_AUX_H
#define X_AUX_H


#include <stdio.h>
#include <math.h>

typedef float TwoDPoint[2];

typedef struct twodnode * TwoDVertexPtr;

typedef struct twodnode {
  TwoDPoint point;
  TwoDVertexPtr next;
} TwoDVertexNode;

typedef struct {
  int num_segments;
  TwoDVertexPtr vertices;
  float tot_dis;
} TwoDLineStrip;

extern void pcopy(TwoDPoint,TwoDPoint);

extern void init_linestrip(TwoDLineStrip *strip);

extern void add_a_vertex(TwoDLineStrip *strip, TwoDPoint point);

extern void delete_linestrip(TwoDLineStrip **);

extern float distance_of_points(TwoDPoint one,TwoDPoint two);

extern float distance_of_linestrip(TwoDLineStrip strip,int seg);

extern int construct_float_from_linestrip(float a[], TwoDLineStrip strip, 
				      int size, 
				      float (* f) (TwoDPoint,TwoDPoint,float));

extern void scale_array(float array[], int size, int hi,
			float *yscale, float *yoffset);

float test(TwoDPoint,TwoDPoint,float);

extern int showgraph (float [], int, int, float, float, float, const char *);

#endif  // X_AUX_H
