#include "surface.h"
#include "vrpn_Connection.h"
#include <time.h>
#include <sys/time.h>

//#ifndef NMM_MICROSCOPE_SIMULATOR_H
//#include "nmm_Microscope_Simulator.h"
//#endif

BCGrid * mygrid;  // Added Tom Hudson 10 June 99 to simplify
BCPlane * myZPlane;

/****************************************************************************
 *  Steve's PARSER
 ****************************************************************************/

static int num_x, num_y;
static int port = 4500;
static char * image_name;
int last_point_x = NULL;
int last_point_y = NULL;


//        crib the text directly from there for familiarity's sake.

void usage (const char * argv0) {
  fprintf(stderr,
    "Usage:  %s [-image <picture>] [-grid <x> <y>] [-port port] [-latency <t>]\n", argv0);
  fprintf(stderr,
    "    -image:  Use picture specified (otherwise use function).\n");
  fprintf(stderr,
    "    -grid:  Take x by y samples for the grid.\n");
  fprintf(stderr,
    "    -latency:  Simulate t seconds of network latency.\n");
  fprintf(stderr,
    "    -port:  Port number for VRPN server to use (default 4500).\n");
  fprintf(stderr,
    "    -rude:  Don't sleep;  use 100% of the CPU.\n");

  exit(0);
}

void open_image (char * image_name) {
  fprintf(stderr, "working with a function -open_image\n");
  cout << "image_name -> " << image_name << endl;
}

void get_grid_info( int num_x, int num_y) {
  fprintf(stderr, "working with a function get_grid_info\n");
  cout << "num_x -> " << num_x << endl;
  cout << "num_y -> " << num_y << endl;
}


// Parse argv and write the values into s.
// Return nonzero on failure.
// TODO:  add more command-line options from microscape.
      
int parse (int argc, char ** argv) {
  int ret = 0;
  int i;
  i = 1;
  
  while (i < argc) {
    fprintf(stderr, "parse:  arg %d %s\n", i, argv[i]);
    if (!strcmp(argv[i], "-image")) {
      if (++i >= argc) usage(argv[0]);
      image_name = argv[i];
      open_image ( image_name );
    } else if (strcmp(argv[i], "-port") == 0) {
      if (++i >= argc) usage(argv[0]);
      port = atoi(argv[i]);
    } else if (strcmp(argv[i], "-grid") == 0) {
      if (++i >= argc) usage(argv[0]);
      num_x = atoi(argv[i]);
      if (++i >= argc) usage(argv[0]);
      num_y = atoi(argv[i]);
      get_grid_info(num_x, num_y);
    }// else if (!strcmp(argv[i], "-latency")) {
     // if (++i >= argc) usage(argv[0]);
     // g_isWaiting = 1;
     // g_waitTime = atof(argv[i]);
  //  } else if (!strcmp(argv[i], "-rude")) {
  //    g_isRude = 1;
  //  }
    else
      ret = 1;
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




int getImageHeightAtXYLoc( float x , float y , float* z ) {
  *z = myZPlane->value( (int)x, (int)y );
  return 1;
}

int moveTipToXYLoc( float x , float y, float set_point ) {
  const int numsets = 1;
  int j,k;
  j = (int)x;
  k = (int)y;
  if(j>(num_x-1))
	j = num_x-1;
  if(j<0)
        j = 0;
  if(k>(num_y-1))
        k = num_y-1;
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
        if((myZPlane->value( Ax, Ay) - set_point) < 0)
          myZPlane->setValue( Ax, Ay, 0);
        else
           myZPlane->setValue( Ax, Ay, (myZPlane->value( Ax, Ay) - set_point));
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
        if((myZPlane->value( Ax, Ay) - set_point) < 0)
          myZPlane->setValue( Ax, Ay, 0);
        else
          myZPlane->setValue( Ax, Ay, (myZPlane->value( Ax, Ay) - set_point));
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
    float point_value[numsets];
    for(int i = 0; i < numsets; i++)
    if((myZPlane->value( j, k) - set_point) < 0)
      myZPlane->setValue( j, k, 0);
    else
      myZPlane->setValue( j, k, (myZPlane->value( j, k) - set_point));
  }
  

  float point_value[numsets];

#if 0

  // Original COMP 145 solution - gives the value at the corner of the
  // current grid square.  This creates a surface that feels like a
  // summation of step functions - since it is.

  for(int i = 0; i < numsets; i++)
  	point_value[i]=myZPlane->value(j,k);

#else

  // TCH June 1999 - instead, interpolate linearly within the current
  // grid square.

  // NOTE k is y, j is x!

  float xF, yF;
  int jprime, kprime;
  int i;

  // Make sure [j+1][k+1] is a legal grid coordinate:
  //   make sure (j, k) is in [(0, 0), (num_x - 2, num_y - 2)]
  //   so that (j+1, k+1) is <= (num_x - 1, num_y - 1).
  // If we're off the square to the right or down, this should
  //   reduce to interpolating along the right or bottom edge
  //   (with a couple of extra multiplies/adds thrown in for
  //   things that will be weighted 0).

  jprime = j;
  kprime = k;
  if (jprime > num_x - 2) jprime = num_x - 2;
  if (kprime > num_y - 2) kprime = num_y - 2;

  // Fractional part of x.
  //  REVERSE IT to get the appropriate weights for the interpolation.

  xF = 1.0f - (x - jprime);
  yF = 1.0f - (y - kprime);

  if (xF > 1.0f) xF = 1.0f;
  if (xF < 0.0f) xF = 0.0f;
  if (yF > 1.0f) yF = 1.0f;
  if (yF < 0.0f) yF = 0.0f;

  for (i = 0; i < numsets; i++)
    point_value[i] = yF * xF * (myZPlane->value(jprime,kprime))
                   + (1.0f - yF) * xF * (myZPlane->value(jprime+1,kprime))
                   + yF * (1.0f - xF) * (myZPlane->value(jprime,kprime+1))
                   + (1.0f - yF) * (1.0f - xF)
                                 * (myZPlane->value(jprime+1,kprime+1));

//fprintf(stderr, "Interpolating at %.5f, %.5f with weights %.5f, %.5f:\n",
//x, y, xF, yF);
//fprintf(stderr, "   x[y = %d][x = %d] = %.5f\n",
//kprime, jprime, surface[kprime][jprime]);
//fprintf(stderr, "   x[%d][%d] = %.5f\n",
//kprime + 1, jprime, surface[kprime + 1][jprime]);
//fprintf(stderr, "   x[%d][%d] = %.5f\n",
//kprime, jprime + 1, surface[kprime][jprime + 1]);
//fprintf(stderr, "   x[%d][%d] = %.5f\n",
//kprime + 1, jprime + 1, surface[kprime + 1][jprime + 1]);
//fprintf(stderr, "   Output %.5f\n", point_value[0]);

#endif

  last_point_x = j;
  last_point_y = k;
  return 0;
}




int main( int argc, char ** argv)
{
  FILE * outputfile;
  float point;
  int x, y;
  int retval;
  retval = parse(argc, argv);
  if (retval) {
    usage(argv[0]);
    exit(0);
  }
  if(image_name)	// CODE USED IF IMAGE IS TO BE USED
  {
    mygrid = new BCGrid (num_x,num_y,0,300,0,300, READ_FILE, image_name); // creates new grid
    myZPlane = mygrid->getPlaneByName("Topography-Forward");	// adds plane for Z data
    myZPlane = mygrid->getPlaneByName(image_name);	// sets data in Z grid
  }
  else			// CODE USED IF MATH SURFACE TO BE USED
  {
    mygrid = new BCGrid (num_x,num_y,0,300,0,300);  
    mygrid->addNewPlane("Topography-Forward","nm", 0);
    myZPlane = mygrid->getPlaneByName("Topography-Forward");
    for(x = 0; x < num_x; x++) 	// SETS VALUES OF PLANE
      {
        for(y = 0; y < num_y; y++)
          {
            point = (fabs(50.0 * (sin((x + y) / 40.0))));
            myZPlane->setValue(x, y, (float)(point));
          }
      } 
  }
  initJake(num_x, num_y);
  while (1) {
    jakeMain();
  }
}










