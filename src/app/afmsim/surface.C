#include "surface.h"

#include <time.h>
//#include <sys/time.h>
#include <vrpn_Types.h>  // for portable sys/time.h

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <math.h>

#include <vrpn_Connection.h>

#include <BCGrid.h>
#include <BCPlane.h>
#include <Topo.h>

#include "simulator_server.h"

#include "nmm_Microscope_Simulator.h"  // for connection

// Necessary to link with nmm_Microscope
static TopoFile GTF;

// Added Tom Hudson 10 June 99 to simplify
static BCPlane * g_myZPlane;

/****************************************************************************
 *  Steve's PARSER
 ****************************************************************************/

static short g_numX = 300, g_numY = 300;
static int g_port = 4500;
static char * g_imageName;

static int g_planeShape;

#define PSHAPE_SINUSOIDAL 0
#define PSHAPE_STEP 1
#define PSHAPE_RAMP 2
#define PSHAPE_HEMISPHERES 3
#define PSHAPE_WAVES 4
#define PSHAPE_CIRCULAR_WAVES 5
#define PSHAPE_STEPPED_RAMP 6

static vrpn_bool g_isWaiting = vrpn_FALSE;
static float g_waitTime = 0.0f;

static char * g_ipString = NULL;

void usage (const char * argv0) {
  fprintf(stderr,
    "Usage:  %s [-image <picture>] [-grid <x> <y>] [-port <port>]\n"
    "        [-surface <n>] [-if <ip>]\n", argv0);
  fprintf(stderr,
    "    -image:  Use picture specified (otherwise use function).\n"
    "    -grid:  Take x by y samples for the grid (default 300 x 300).\n"
    "    -port:  Port number for VRPN server to use (default 4500).\n");
  fprintf(stderr,
    "    -surface:  Use given surface function:  0 = sinusoidal (default),\n"
    "      1 = step functions, 2 = ramp, 3 = hemispheres, 4 = waves,\n"
    "      5 = circular waves, 6 = stepped ramp.\n");
  fprintf(stderr,
    "    -if:  IP address of network interface to use.\n");
  

  exit(0);
}



// Parse argv and write the values into s.
// Return nonzero on failure.
// TODO:  add more command-line options from topo.
      
int parse (int argc, char ** argv) {
  int ret = 0;
  int i;
  i = 1;
  
  while (i < argc) {
    fprintf(stderr, "parse:  arg %d %s\n", i, argv[i]);
    if (!strcmp(argv[i], "-image")) {
      if (++i >= argc) usage(argv[0]);
      g_imageName = argv[i];
    } else if (strcmp(argv[i], "-port") == 0) {
      if (++i >= argc) usage(argv[0]);
      g_port = atoi(argv[i]);
    } else if (strcmp(argv[i], "-grid") == 0) {
      if (++i >= argc) usage(argv[0]);
      g_numX = atoi(argv[i]);
      if (++i >= argc) usage(argv[0]);
      g_numY = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-surface")) {
      if (++i >= argc) usage(argv[0]);
      g_planeShape = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-if")) {
       if (++i >= argc)  usage(argv[0]);
       g_ipString = argv[i];
#if 0
    } else if (!strcmp(argv[i], "-latency")) {
      if (++i >= argc) usage(argv[0]);
      g_isWaiting = vrpn_TRUE;
      g_waitTime = atof(argv[i]);
    } else if (!strcmp(argv[i], "-rude")) {
      g_isRude = 1;
#endif
    } else {
      ret = 1;
    }
    i++;
  }

  // check for mandatory arguments
  //if (!s->microscopeName) 
  //  ret = 1;

  return ret;
}


/******************************************************************
 End Steve's Parser code
 *****************************************************************/




int getImageHeightAtXYLoc (float x, float y, float * z) {
  double zz;
  g_myZPlane->valueAt(&zz, x, y);
  *z = zz;
  return 1;
}

int moveTipToXYLoc( float x , float y, float set_point ) {

  static int last_point_x = 0;
  static int last_point_y = 0;

  const int numsets = 1;
  int i, j, k;
  //j = (int)x;
  //k = (int)y;
  j = g_myZPlane->xInGrid(x);
  k = g_myZPlane->yInGrid(y);
  if(j>(g_numX-1))
	j = g_numX-1;
  if(j<0)
        j = 0;
  if(k>(g_numY-1))
        k = g_numY-1;
  if(k<0)
        k = 0;
  if(last_point_x)  		// Use Bresenhams Line algorithm to drag tip
  {				// Code by Kenny Hoff
    //------------------------------------------------------------------------
    // INITIALIZE THE COMPONENTS OF THE ALGORITHM THAT ARE NOT AFFECTED BY THE
    // SLOPE OR DIRECTION OF THE LINE
    //------------------------------------------------------------------------
    int Ax = last_point_x;
    int Ay = last_point_y;    
    int Bx = j;
    int By = k;
    int dX = abs(Ax-Bx);    // store the change in X and Y of the line endpoints
    int dY = abs(Ay-By);
        
    //------------------------------------------------------------------------
    // DETERMINE "DIRECTIONS" TO INCREMENT X AND Y (REGARDLESS OF DECISION)
    //------------------------------------------------------------------------
    int Xincr, Yincr;
    if (Ax > Bx) { Xincr=-1; } else { Xincr=1; }    // which direction in X?
    if (Ay > By) { Yincr=-1; } else { Yincr=1; }    // which direction in Y?
        
    //------------------------------------------------------------------------
    // DETERMINE INDEPENDENT VARIABLE (ONE THAT ALWAYS INCREMENTS BY 1 (OR -1) )
    // AND INITIATE APPROPRIATE LINE DRAWING ROUTINE (BASED ON FIRST OCTANT
    // ALWAYS). THE X AND Y'S MAY BE FLIPPED IF Y IS THE INDEPENDENT VARIABLE.
    //------------------------------------------------------------------------
    if (dX >= dY)   // if X is the independent variable
    {           
      int dPr         = dY<<1;           // amount to increment decision if right is chosen (always)
      int dPru        = dPr - (dX<<1);   // amount to increment decision if up is chosen
      int P           = dPr - dX;  	 // decision variable start value

      for (; dX>=0; dX--)            	 // process each point in the line one at a time (just use dX)
      {
        if((g_myZPlane->value( Ax, Ay) - set_point) < 0)
          g_myZPlane->setValue( Ax, Ay, 0);
        else
           g_myZPlane->setValue( Ax, Ay, (g_myZPlane->value( Ax, Ay) - set_point));
        //SetPixel(Ax, Ay, Color); 	 // plot the pixel
        if (P > 0)               	 // is the pixel going right AND up?
        { 
          Ax+=Xincr;	                 // increment independent variable
          Ay+=Yincr;        		 // increment dependent variable
          P+=dPru;          		 // increment decision (for up)
        }
        else	                         // is the pixel just going right?
        {
          Ax+=Xincr;		         // increment independent variable
          P+=dPr;           		 // increment decision (for right)
        }
      }               
    }
    else            		         // if Y is the independent variable
    {
      int dPr         = dX<<1;           // amount to increment decision if right is chosen (always)
      int dPru        = dPr - (dY<<1);   // amount to increment decision if up is chosen
      int P           = dPr - dY; 	 // decision variable start value

      for (; dY>=0; dY--)                // process each point in the line one at a time (just use dY)
      {
        if((g_myZPlane->value( Ax, Ay) - set_point) < 0)
          g_myZPlane->setValue( Ax, Ay, 0);
        else
          g_myZPlane->setValue( Ax, Ay, (g_myZPlane->value( Ax, Ay) - set_point));
	// SetPixel(Ax, Ay, Color);         // plot the pixel
        if (P > 0)                       // is the pixel going up AND right?
        { 
          Ax+=Xincr;                     // increment dependent variable
          Ay+=Yincr;        		 // increment independent variable
          P+=dPru;          		 // increment decision (for up)
        }
        else 	                         // is the pixel just going up?
        {
          Ay+=Yincr;		         // increment independent variable
          P+=dPr;           		 // increment decision (for right)
        }
      }               
    }               
  }
  else
  {
    //float point_value[numsets];
    for(int i = 0; i < numsets; i++)
    if((g_myZPlane->value( j, k) - set_point) < 0)
      g_myZPlane->setValue( j, k, 0);
    else
      g_myZPlane->setValue( j, k, (g_myZPlane->value( j, k) - set_point));
  }

  last_point_x = j;
  last_point_y = k;
  return 0;
}


void initializePlane (BCPlane * zPlane, int planeShape) {
  int x, y;
  double point;
  double rx, ry;
  double radius;
  double targetradius, tr2;
  double interval, in2;
  double sx, sy;
  int step;

  switch (planeShape) {

    case PSHAPE_SINUSOIDAL :
      // sinusoidal surface with 50 nm amplitude and 40*pi nm wavelength
      fprintf(stderr, "Setting up sinusoidal surface.\n");
      for (x = 0; x < g_numX; x++) {
        for (y = 0; y < g_numY; y++) {
          point = fabs(50.0f * (sin((x + y) / 40.0f)));
          zPlane->setValue(x, y, point);
        }
      } 
      break;
    case PSHAPE_STEP :
      // surface with 50 nm steps every 40 nm
      fprintf(stderr, "Setting up stepped surface.\n");
      for (x = 0; x < g_numX; x++) {
        for (y = 0; y < g_numY; y++) {
          point = 50.0f * ceil((x + y) / 40.0f);
          zPlane->setValue(x, y, point);
        }
      }
      break;
    case PSHAPE_RAMP :
      // surface with smooth slope 1.25
      fprintf(stderr, "Setting up ramped surface.\n");
      for (x = 0; x < g_numX; x++) {
        for (y = 0; y < g_numY; y++) {
          point = 1.2f * (x + y);
          zPlane->setValue(x, y, point);
        }
      }
      break;
    case PSHAPE_HEMISPHERES :
      // flat surface with hemispheres of radius <targetradius> nm every <interval> nm
      fprintf(stderr, "Setting up regular-hemisphere surface.\n");
      targetradius = 20.0f;
      tr2 = targetradius * targetradius;
      interval = 75.0f;
      in2 = interval / 2;
      for (x = 0; x < g_numX; x++) {
        rx = (fmod(x, interval)) - in2;
        for (y = 0; y < g_numY; y++) {
          ry = (fmod(y, interval)) - in2;
          radius = sqrt(rx * rx + ry * ry);
          if (radius < targetradius) {
            point = sqrt((tr2 - radius * radius) / tr2) * targetradius;
          } else {
            point = 0.0f;
          }
          zPlane->setValue(x, y, point);
        }
      }
      break;
  case PSHAPE_WAVES:
    //waves maybe??
    fprintf(stderr, "setting up waves rv 1.\n");
    for(x = 0;x< g_numX; x++){
      for(y = 0; y < g_numY; y++){
	point = 50.0f * ((sin((x/ 20.0f))*sin((x/20.0f)))+(cos((y/20.0f))*cos((y/20.0f))));
	zPlane-> setValue(x,y,point);
      }
    }
    break;

    case PSHAPE_CIRCULAR_WAVES :

      fprintf(stderr, "Setting up circular waves.\n");
      targetradius = 20.0;
      interval = 150.0;
      in2 = interval / 2.0;
      tr2 = in2 * in2 * 0.1;
      for (x = 0; x < g_numX; x++) {
         rx = fmod(x, interval) - in2;
         for (y = 0; y < g_numY; y++) {
            ry = fmod(y, interval) - in2;
            point = targetradius * cos((rx * rx + ry * ry) / tr2);
            zPlane->setValue(x, y, point);
         }
      }

    break;		 

    case PSHAPE_STEPPED_RAMP :

    fprintf(stderr, "Setting up stepped ramp.\n");
    interval = 50.0;
    for (x = 0; x < g_numX; x++) {
       for (y = 0; y < g_numY; y++) {
          step = (x + y) / interval;
          if (step % 2) {
             point = step * interval;
          } else {
             point = (step - 1) * interval + fmod(x + y, interval);
          }
          zPlane->setValue(x, y, point);
       }
    }
    break;


    default:
      fprintf(stderr, "Unimplemented plane shape %d\n", planeShape);
      exit(0);

  }
}


int main (int argc, char ** argv) {

  BCGrid * mygrid;

  int readmode;
  int retval;

  retval = parse(argc, argv);
  if (retval) {
    usage(argv[0]);
    exit(0);
  }

  if (g_imageName) {	// CODE USED IF IMAGE IS TO BE USED

    // create new grid
    readmode = READ_FILE;
    mygrid = new BCGrid (g_numX, g_numY, 0.0, 300.0, 0.0, 300.0,
                         readmode, g_imageName, GTF);

    // add plane for Z data
    // Double huh?
    g_myZPlane = mygrid->getPlaneByName("Topography-Forward");

    // Huh?
    g_myZPlane = mygrid->getPlaneByName(g_imageName);

  } else {			// CODE USED IF MATH SURFACE TO BE USED

    mygrid = new BCGrid (g_numX,g_numY,0,300,0,300);  
    mygrid->addNewPlane("Topography-Forward","nm", 0);
    g_myZPlane = mygrid->getPlaneByName("Topography-Forward");

    initializePlane(g_myZPlane, g_planeShape);
  }

  retval = initJake(g_numX, g_numY, g_port, g_ipString);
  while (!retval) {
    jakeMain(.1, g_isWaiting, g_waitTime);
  }
}










