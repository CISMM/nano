/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * May 2001
 */

/* Input to the simulator */

#include <stdlib.h>		//stdlib.h vs cstdlib
#include <stdio.h>		//stdio.h vs cstdio
#include <iostream.h>
#include <vector>
#include <math.h>		//math.h vs cmath
#include <GL/glut.h>
#include "Vec3d.h"
#include "3Dobject.h"
#include "ConeSphere.h"
#include "Tips.h"
#include <string.h>
#include "defns.h"
#include "input.h"
#include "sim.h"

/* First the tip */
/* Here are our AFM tips */
// third arg is the default
// all units in nm
static SphereTip sp(5.);
static InvConeSphereTip ics(5.,1000.,DEG_TO_RAD*20.,tesselation);
Tip tip(&sp,&ics,tesselation,INV_CONE_SPHERE_TIP);
//Tip tip(sp,ics,tesselation,SPHERE_TIP);

Vec3d vertex[MAXVERTICES];

/* Now the objects */
/* Here are our object. Note we want our objects to be above the surface
 * i.e z >= 0
 */
void initObs( int numtoDraw )
{
  // We start with no objects.
  numObs = 0;
  
  if(numtoDraw > 0){
    addNtube( SPHERE,  Vec3d( 50., 60., 50.), 0., 0., 0., 0., 10.);
  }
}

/* Give me the unit in terms of nm. So if the unit assumed in the file is 
 * Angstrom, give me 0.1 (since 1 A = 0.1 nm)
 */
void addSpheresFromFile (char *filename, double no_of_nm_in_one_unit, 
			 bool rad_exists) {
  double x,y,z;
  int stop;
  double minx=0.,miny=0.,minz=0., maxx=0., maxy=0., maxz=0.;

  FILE *file = fopen(filename,"r"); 

  cout << "Loading file " << filename << endl;

  stop=0;
  while (!stop) {
    fscanf(file,"%lf",&x);
    if (!feof(file)) {
      fscanf(file,"%lf",&y);
      fscanf(file,"%lf",&z);
      // unit conversion - everything will be in nm from now on.
      x *= no_of_nm_in_one_unit;
      y *= no_of_nm_in_one_unit;
      z *= no_of_nm_in_one_unit;
      double rad;
      if(rad_exists){
	fscanf(file, "%lf", &rad);
      }
      else{
	// assume a radius of 1.5 A
	rad = 5*1.5*no_of_nm_in_one_unit;//I just made bigger
                                               //by mult. by a number (5)
      }
      // need to do some profiling for later.
      minx = ((!minx) || (x < minx)) ? x : minx;
      miny = ((!miny) || (z < miny)) ? y : miny;
      minz = ((!minz) || (z < minz)) ? z : minz;
      maxx = ((!maxx) || (x > maxx)) ? x : maxx;
      maxy = ((!maxy) || (y > maxy)) ? y : maxy;
      maxz = ((!maxz) || (z > maxz)) ? z : maxz;
      addNtube( SPHERE,  Vec3d( x, y, z), 0., 0., 0., 0., rad*2);
    }
    else
      stop=1;
  }

  //cout << "Done no of spheres = " << (numObs-1) << endl;


  /* now use the profiled data to translate and scale the values to lie in
   * our orthogonal volume
   */
  /*Place the "XY centroid" of the body at the centre. Also let the lowest
   * pt lie just on the surface.
   */
  Vec3d translate = Vec3d(-(minx+maxx)/2.,-(miny+maxy)/2.,-minz) +
    Vec3d(orthoFrustumLeftEdge+orthoFrustumWidth/2.,orthoFrustumBottomEdge+orthoFrustumHeight/2.,0.);
  for (int i=0;i<numObs;i++) {
    ob[i]->translate(translate);
  }
  Vec3d centroid = Vec3d((minx+maxx)/2.,(miny+maxy)/2.,(minz+maxz)/2.)+translate;
    
}

// give me the scale
void addTrianglesFromFile(char *filename, double scale) {
  FILE *file = fopen(filename,"r"); 
  int stop=0;
  char c;
  double x,y,z;
  int v1,v2,v3;
  double minx=0.,miny=0.,minz=0., maxx=0., maxy=0., maxz=0.;

  cout << "Loading file " << filename << endl;

  int cnt=1; // waste the first element
  while (!stop) {
    fscanf(file,"%c",&c);
    if (!feof(file)) {
      if (c == 'v') {
	// read the vertices
	fscanf(file,"%lf",&x);
	fscanf(file,"%lf",&y);
	fscanf(file,"%lf",&z);
	if (cnt < MAXVERTICES) {
	  vertex[cnt] = Vec3d(x,y,z);
	  cnt++;
	}
	else {
	  cout << "Error: Too many vertices\n";
	  exit(0);
	}
      }
      else if (c == 'f') {
	// read the vertex numbers
	fscanf(file,"%d",&v1);
	fscanf(file,"%d",&v2);
	fscanf(file,"%d",&v3);
	addTriangle(vertex[v1],  vertex[v2],  vertex[v3]);  
	// need to do some profiling for later.
	minx = ((!minx) || (vertex[v1].x < minx)) ? vertex[v1].x : minx;
	maxx = ((!maxx) || (vertex[v1].x > maxx)) ? vertex[v1].x : maxx;
	miny = ((!miny) || (vertex[v1].y < miny)) ? vertex[v1].y : miny;
	maxy = ((!maxy) || (vertex[v1].y > maxy)) ? vertex[v1].y : maxy;
	minz = ((!minz) || (vertex[v1].z < minz)) ? vertex[v1].z : minz;
	maxz = ((!maxz) || (vertex[v1].z > maxz)) ? vertex[v1].z : maxz;


      }
    }
    else {
      stop = 1;
    }
  }

  cout << "Done vertices = " << (cnt-1) << " triangles = " << (numObs-1) << endl;
  /* now use the profiled data to translate and scale the values to lie in
   * our orthogonal volume
   */
  /*Place the "XY centroid" of the body at the centre. Also let the lowest
   * pt lie just on the surface.
   */
#if 1
  Vec3d negcentroid = Vec3d(-(minx+maxx)/2.,-(miny+maxy)/2.,-(minz+maxz)/2.);
  Vec3d center = Vec3d(orthoFrustumLeftEdge+orthoFrustumWidth/2.,orthoFrustumBottomEdge+orthoFrustumHeight/2.,0.);
  for (int i=0;i<numObs;i++) {
    ob[i]->translate(negcentroid);
    ob[i]->scale(scale);
    // lift up so that body just sits on the surface
    ob[i]->translate(Vec3d(0,0,scale*(maxz-minz)/2.));
    // positon the centroid of the body at the centre.
    ob[i]->translate(center);
  }
#endif
}

