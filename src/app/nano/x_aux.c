#include <stdlib.h>
#include <math.h>

#include <nmb_Globals.h>
#include <nmb_Dataset.h>
#include "BCPlane.h"

#include "x_aux.h"
#include "x_util.h"

#include "microscape.h"  // for xPlaneName

void pcopy(TwoDPoint,TwoDPoint);

void init_linestrip(TwoDLineStrip *strip);

void add_a_vertex(TwoDLineStrip *strip, TwoDPoint point);

float distance_of_points(TwoDPoint one,TwoDPoint two);

float distance_of_linestrip(TwoDLineStrip strip,int seg);

int construct_float_from_linestrip(float a[], TwoDLineStrip strip, int size, 
				   float (* f) (TwoDPoint,TwoDPoint,float));

float scale_array(float array[],int size,int hi);

void delete_linestrip(TwoDLineStrip **);

float test(TwoDPoint,TwoDPoint,float);

float test(TwoDPoint p1,TwoDPoint p2,float s)
{
  //double points_per_pixel;
  float dis;
  float x,y;
  int ix,iy;
  float a,b;
  float z;

  BCPlane* plane = dataset->inputGrid->getPlaneByName((const char*)xPlaneName);	
  if (plane == NULL) {
      fprintf(stderr, "Error in test: could not get plane!\n");
      return 0.0;
  }
  //points_per_pixel = ((float )WINSIZE) /
	      //max(plane->numX(),plane->numY());
  
  dis=distance_of_points(p1,p2);
  dis=s/dis;
  x=p1[0]+(p2[0]-p1[0])*dis;
  y=p1[1]+(p2[1]-p1[1])*dis;

  x=(x-plane->minX())*plane->numX()/(plane->maxX()- plane->minX());
  y=(y-plane->minY())*plane->numY()/(plane->maxY()- plane->minY());

  ix=(int) x;
  iy=(int) y;
    
  if( ( ix >= (plane->numX()-1) )||( iy >= (plane->numY()-1) ) ) {
    return (0.0);
  }

  // Find the value at the point using bilinear interpolation between the
  // surrounding gridpoints.
  a=x-ix;
  b=y-iy;
  z = 	plane->value(ix,  iy  )*(1-a)*(1-b)
    + 	plane->value(ix+1,iy  )*(  a)*(1-b)
    + 	plane->value(ix,  iy+1)*(1-a)*(  b)
    + 	plane->value(ix+1,iy+1)*(  a)*(  b);
  return(z);
}

void delete_linestrip(TwoDLineStrip **strip)
{
  TwoDVertexPtr head, prev;
  
  head=(*strip)->vertices;
  prev=head;
  while(head!=NULL)
    {
      head=head->next;
      free((char*)prev);
      prev=head;
    }
  free((char*)*strip);
  *strip=NULL;
}

void init_linestrip(TwoDLineStrip *strip)
{
  strip->num_segments=0;
  strip->vertices=NULL;
  strip->tot_dis=0;
}

void pcopy(TwoDPoint p1,TwoDPoint p2)
{
  p1[0]=p2[0];
  p1[1]=p2[1];
}

void add_a_vertex(TwoDLineStrip *strip, TwoDPoint p)
{
  TwoDVertexPtr head, prev;

  head=strip->vertices;
  if(head==NULL)
    {
      if ((strip->vertices=(TwoDVertexPtr)malloc(sizeof(TwoDVertexNode)))
	  ==NULL)
	{
	  printf("add_a_vertex(): can not malloc a new vertex\n");
	  exit(-1);
	}
      pcopy(strip->vertices->point,p);
      strip->vertices->next=NULL;
    }
  else
    {
      prev=head;
      while(head!=NULL)
	{
	  prev=head;
	  head=head->next;
	}
      if((head=(TwoDVertexPtr) malloc(sizeof(TwoDVertexNode)))==NULL)
	{
	  printf("add_a_vertex(): can not malloc a new vertex\n");
	  exit(-1);
	}
      pcopy(head->point,p);
      head->next=NULL;
      prev->next=head;
      strip->num_segments++;
      strip->tot_dis+=distance_of_points(prev->point,head->point);
    }
}

float distance_of_points(TwoDPoint one,TwoDPoint two)
{
  float temp;
  temp=sqrt((one[1]-two[1])*(one[1]-two[1])
		 +(one[0]-two[0])*(one[0]-two[0]));
  return(temp);
}


float distance_of_linestrip(TwoDLineStrip strip,int seg)
{
  int i;
  TwoDVertexPtr cur,next;
  float temp=0;

  if(seg>strip.num_segments)
    {
      printf("distance_of_linestrip: the number of line segments requested exceeds the maximum line segments in the strip\n");
      exit(-1);
    }
  else if(seg<=0)
    return(0);

  cur=strip.vertices;
  next=cur->next;

  for(i=1;i<=seg;i++)
    {
      temp+=distance_of_points(cur->point,next->point);
      cur=next;
      next=cur->next;
    }
  return(temp);
}

/* function construct a float array from a line strip for display using Tk;
   return number of floats initiated if successful, return -1 otherwise 
   function f takes two points and a float as arguments
   only catch: the size has to been smaller than the size of the array.
 */
int construct_float_from_linestrip(float a[], TwoDLineStrip strip, int wi,
				   float (* f) (TwoDPoint,TwoDPoint,float))
{
  TwoDVertexPtr cur,next;
  int index=0;
  float cur_dis=0;
  float cur_line_dis;
  float inc;

  cur=strip.vertices;
  if(cur==NULL || cur->next==NULL)
    {
      printf("construct_float_from_linestrip(): strip has less than point\n");
      return(-1);
    }
  next=cur->next;

  cur_line_dis=distance_of_points(cur->point,next->point);
  inc=strip.tot_dis/((float) wi);
  
  while(next!=NULL)
    {
      a[index]=f(cur->point,next->point,cur_dis);
//fprintf(stderr, "Construct:  at %.4f got z = %.4f\n", cur_dis, a[index]);
      index++;
      cur_dis+=inc;
      if(cur_dis>cur_line_dis)
	{
	  cur_dis-=cur_line_dis;
	  cur=next;
	  next=cur->next;
	  if(next!=NULL)
	    cur_line_dis=distance_of_points(cur->point,next->point);
	  else
	    cur_line_dis=0;
	}
    }
  return(index-1);
}

/* Scale the array so that its values all run from -hi/2 to hi/2.  Then, return
 * the scale factor needed to map the values back to distance values.
 * This produces and offset in the values, but differences between two
 * values will still be valid. */

void scale_array(float array[],int size,int hi, float *yscale, float *yoffset)
{
  double max,min,temp;
  double dist, normed, scaled;
  int i;

  if(size<=0) {
      printf("scale_array(): 0 or negative elements in this array\n");
  }

  // Find the range of values in the array
  max=min=array[0];
  for(i=1;i<size;i++) {
      temp=array[i];
      if (temp > max) { max = temp; }
      if (temp < min) { min = temp; }
  }
  dist = max - min;

  // Scale the array to map from -hi/2 to +hi/2
  if (dist != 0) {
	for(i=0;i<size;i++) {
		normed = (array[i]-min) / dist;
		scaled = normed*hi - hi/2;
		array[i] = scaled;
	}
  }

  // Fill in the parameters so that you can take an entry and turn
  // it back into physical units using:  physical = array[i] * yscale + yoffset
  *yscale = dist/hi;
  *yoffset = (hi/2)*(*yscale) + min;
}

