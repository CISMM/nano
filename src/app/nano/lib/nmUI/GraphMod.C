/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <math.h>
#include <blt.h>  // for BLT vector class and functions.
#if defined (__CYGWIN__) || (linux) || (_WIN32)
#include <float.h> // for FLT_MAX
#endif


#include "GraphMod.h"

static const int IMAGEMODE = 0;
static const int MODMODE = 1;
static const int SCANLINEMODE = 2;

extern Tcl_Interp * get_the_interpreter (void);

// GraphMod
//


Tclvar_int graphmod_hasWindow ("graphmod_hasWindow", 0); 
                                // it won't work the other way unless
				// we correctly link the Tcl variable with
				// the C variable - AAS
//static int graphmod_hasWindow = 0;

GraphMod::GraphMod (void) :
    d_currentmode (IMAGEMODE),
    d_lastmode (MODMODE),
    d_interp (NULL), 
    d_total_num_points (0),
    d_num_points_graphed (0),
    d_max_points ("gm_max_num_points", 10000),
    d_stride ("gm_stride", 2),
    d_numDataVectors(0),
    d_num_scanlines(0),
    d_numscanline_channels(0),
    d_scanlength(0) {

 d_max_points.addCallback(handle_MaxPointsChange, this);
 d_stride.addCallback(handle_StrideChange, this);

//fprintf(stderr, "GraphMod constructor\n");
}

GraphMod::~GraphMod (void) {

}

// static
int GraphMod::EnterModifyMode (void * userdata) {
  GraphMod * me = (GraphMod *) userdata;
  char    command[300];

  if (me->d_currentmode == MODMODE) {
      // This is OK, sometimes happens when collaborating. 
//      fprintf(stderr, "GraphMod::EnterModifyMode:  "
//                      "Error switching from MODMODE to MODMODE.\n");
    return 0;
  }

  me->d_interp = get_the_interpreter();
  if (!graphmod_hasWindow) {
    me->ShowStripchart(NULL);
    graphmod_hasWindow = 1;
  }

  me->d_lastmode = me->d_currentmode;
  me->d_currentmode = MODMODE;

  sprintf(command, "set_for_point_mode");
  TCLEVALCHECK(me->d_interp, command);

  return 0;
}

// static
int GraphMod::EnterImageMode (void * userdata) {
  GraphMod * me = (GraphMod *) userdata;

  if (me->d_currentmode == IMAGEMODE) {
     return 0;
  }

  me->d_lastmode = me->d_currentmode;
  me->d_currentmode = IMAGEMODE;
  me->d_total_num_points = 0;
  me->d_num_points_graphed = 0;

  return 0;
}

int GraphMod::EnterScanlineMode (void *userdata) {
  GraphMod * me = (GraphMod *) userdata;
  char    command[300];

  printf("called GraphMod::EnterScanlineMode\n");
  if (me->d_currentmode == SCANLINEMODE) {
    fprintf(stderr, "GraphMod::EnterScanlineMode:  "
                    "Error switching from SCANLINEMODE to SCANLINEMODE.\n");
    return -1;
  }

  me->d_interp = get_the_interpreter();
  if (!graphmod_hasWindow) {
    me->ShowStripchart(NULL);
    graphmod_hasWindow = 1;
  }

  me->d_lastmode = me->d_currentmode;
  me->d_currentmode = SCANLINEMODE;

  sprintf(command, "set_for_scanline_mode");
  TCLEVALCHECK(me->d_interp, command);


  return 0;
}

int GraphMod::ReceiveNewScanline(void *userdata, const Scanline_results *sr) {
      GraphMod * me = (GraphMod *) userdata;
      char    command[300];
      char    str[300];
      Blt_Vector *vecPtr = NULL;
      int i,j,k;
      int stride = 2;
      float t=0, s=0, x=0, y = 0;
      BCString *name, *units;

      if (me->d_currentmode != SCANLINEMODE)
      	return 0;

      me->d_interp = get_the_interpreter();

      int channels_changed = 0;
      if (me->d_numscanline_channels != sr->num_values())
	channels_changed = 1;
      // compare channel names in case they have changed
      else if (me->d_num_scanlines > 0){
      	for (i = 0; i < sr->num_values(); i++){
	    if ( (*(me->d_scanlines[0]->name(i))) != (*(sr->name(i))) ){
		channels_changed = 1;
		break;
	    }
	}
      }

      if (me->d_scanlength != sr->length() || 
	  channels_changed) {
        for (i = 0; i < me->d_num_scanlines; i++){
	    if (channels_changed){
		for (j = 0; j < me->d_numscanline_channels; j++){
		    name = me->d_scanlines[i]->name(j);
		    units = me->d_scanlines[i]->units(j);
		    sprintf(str,"gm_%s_%s%d", (const char *)(*name),
                	(const char *)(*units), i);
	           // remove spaces - they are bad for vector names.
		    for (unsigned int m = 0; m < strlen(str); m++){
                	if (str[m] == ' ' || str[m] == '-')
                            str[m] = '_';
		    }

		    sprintf(command, "remove_stripchart_element \"%s\"",
			str);
  		    TCLEVALCHECK(me->d_interp, command);
		    if (Blt_VectorExists(me->d_interp, str))  {
                	if (Blt_DeleteVectorByName(me->d_interp, str)
				!= TCL_OK) {
                    	    fprintf(stderr, "Tcl_Eval(BLT_CreateVector) "
				"failed: %s\n", me->d_interp->result);
                        return -1;
                    	}
            	    }

		}
	    }
            delete (me->d_scanlines[i]);
      	}
	me->d_scanlength = sr->length();
        me->d_numscanline_channels = sr->num_values();
	me->d_num_scanlines = 0;
      }


      // add this to our stored data, first shift everybody
      if (me->d_num_scanlines > 0) {
	if (me->d_num_scanlines < MAX_SCANLINES)
            me->d_num_scanlines++;
	else
      	    delete me->d_scanlines[MAX_SCANLINES - 1];
      	for (i = me->d_num_scanlines-1; i > 0; i--){
	    me->d_scanlines[i] = me->d_scanlines[i-1];
      	}
      }
      else {
	me->d_num_scanlines = 1;
      }	
      me->d_scanlines[0] = new Scanline_results(*sr);

      printf("GraphMod::ReceiveNewScanline, %d of them now\n", 
		me->d_num_scanlines);

      //set up the time vector first.
      sprintf(str, "gm_timevec");
      if (!Blt_VectorExists(me->d_interp, str))  {
         //create the vector
         if (Blt_CreateVector(me->d_interp, str,0,&vecPtr) != TCL_OK) {
            fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
                    me->d_interp->result);
            return -1;
         }
      }
      // chop it off to one element
      sprintf(command, "gm_timevec set {%f}", t);
      TCLEVALCHECK(me->d_interp, command);

      sprintf(str, "gm_svec");
      //set up the arclength vector.
      if (!Blt_VectorExists(me->d_interp, str))  {
         //create the vector
         if (Blt_CreateVector(me->d_interp, str,0,&vecPtr) != TCL_OK) {
            fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
                    me->d_interp->result);
            return -1;
         }
      }
      // chop it off to one element
      sprintf(command, "gm_svec set {%f}", s);
      TCLEVALCHECK(me->d_interp, command);

      //set up the surface x vector.
      sprintf(str, "gm_xsurfvec");
      if (!Blt_VectorExists(me->d_interp, str))  {
         //create the vector
         if (Blt_CreateVector(me->d_interp, str,0,&vecPtr) != TCL_OK) {
            fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
                    me->d_interp->result);
            return -1;
         }
      }
      // chop it off to one element
      sprintf(command, "gm_xsurfvec set {%f}", x);
      TCLEVALCHECK(me->d_interp, command);

      //set up the y surface vector.
      sprintf(str, "gm_ysurfvec");
      if (!Blt_VectorExists(me->d_interp, str))  {
         //create the vector
         if (Blt_CreateVector(me->d_interp, str,0,&vecPtr) != TCL_OK) {
            fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
                    me->d_interp->result);
            return -1;
         }
      }
      // chop it off to one element
      sprintf(command, "gm_ysurfvec set {%f}", y);
      TCLEVALCHECK(me->d_interp, command);


      stride = ((me->d_scanlength)/MAX_SCANLINE_GRAPH_PTS + 1);

//      printf("adding %d points\n", sr->length());
      for (i = stride; i < me->d_scanlength; i+= stride){
	t += stride;
	s += stride;
	x += stride;
	y += stride;
	sprintf(command, "gm_timevec append %f", t);
	TCLEVALCHECK(me->d_interp, command);
	sprintf(command, "gm_svec append %f", s);
	TCLEVALCHECK(me->d_interp, command);
	sprintf(command, "gm_xsurfvec append %f", x);
	TCLEVALCHECK(me->d_interp, command);
	sprintf(command, "gm_ysurfvec append %f", y);
	TCLEVALCHECK(me->d_interp, command);
      }

      float minval, maxval;	// computed per channel
      for (i = 0; i < me->d_numscanline_channels; i++){
#if defined (__CYGWIN__) || (linux) || (_WIN32)
	minval = FLT_MAX;
	maxval = -FLT_MAX;
#else
	minval = MAXFLOAT;
        maxval = -MAXFLOAT;
#endif
	for (j = 0; j < me->d_num_scanlines; j++) {
	    for (k = stride-1; k < me->d_scanlength; k+=stride){
                if (me->d_scanlines[j]->value(k, i) < minval)
                    minval = me->d_scanlines[j]->value(k, i);
                if (me->d_scanlines[j]->value(k, i) > maxval)
                    maxval = me->d_scanlines[j]->value(k, i);
            }
	}
	for (j = 0; j < me->d_num_scanlines; j++) {
	    // create a vector for each value returned from microscope
	    // and attach it to a graph in the stripchart
	    name = me->d_scanlines[j]->name(i); 
	    units = me->d_scanlines[j]->units(i);
	    sprintf(str,"gm_%s_%s%d", (const char *)(*name), 
		(const char *)(*units), j);

            // remove spaces - they are bad for vector names.
	    for (unsigned int m = 0; m < strlen(str); m++){
		if (str[m] == ' ' || str[m] == '-')
		    str[m] = '_';
	    }


//	    printf("adding vector %s\n", str);

	    if (!Blt_VectorExists(me->d_interp, str))  {
	    	if (Blt_CreateVector(me->d_interp, str, 0, &vecPtr) != TCL_OK) {
		    fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
                         me->d_interp->result);
		    return -1;
	    	}
	    }
	    // chop it off, and set it's first element
	    sprintf(command, "%s set {%12.9g}", str, 
		me->d_scanlines[j]->value(0, i));
	    TCLEVALCHECK(me->d_interp, command);

	    for (k = stride-1; k < me->d_scanlength; k+=stride){
	    	sprintf(command, "%s append %12.9g", 
			str, me->d_scanlines[j]->value(k, i));
	    	TCLEVALCHECK(me->d_interp, command);
            }

	    // Now, connect this vector to the graph.
	    // and add some controls to turn it on and off, 
	    // and change it's limits
	    printf("y axis is %f to %f\n", minval, maxval);
	    if (minval == maxval){
		minval -= 0.001;
		maxval += 0.001;
	    }
	    sprintf(command, "add_stripchart_element \"%s\" %d %f %f %f", 
		str, i,
		((float)(MAX_SCANLINES-j)/(float)MAX_SCANLINES),
		minval, maxval);
	    TCLEVALCHECK(me->d_interp, command);
	}
      }

    return 0;
}


// static
int GraphMod::ReceiveNewPoint (void * userdata, const Point_results * p) {
  GraphMod * me = (GraphMod *) userdata;

  //fprintf(stderr, "GraphMod::ReceiveNewPoint\n");

   //int	numValues;
static   double	last_x,last_y;
static   long	first_sec,first_usec;
   static double	s;
   double time;
   Point_value	*value;
   char    command[300];
   char    str[300];
   Blt_Vector *vecPtr = NULL;

   if (me->d_currentmode != MODMODE)
     return 0;

   me->d_interp = get_the_interpreter();

   if (me->d_total_num_points == 0) {
      // Find out how many values are in the first point.  Used for
      // comparison later.  Also find the starting location and time
      // of the first point.
      //numValues = p->_num_values;
      last_x = p->x();
      last_y = p->y();
      //last_z = p->z();
      //is3D = p->is3D();
      first_sec = p->sec();
      first_usec =p->usec();
      s = 0; time = 0;	// No distance yet.
      
      //set up the time vector first.
      sprintf(str, "gm_timevec");
      if (!Blt_VectorExists(me->d_interp, str))  {
	 //create the vector
	 if (Blt_CreateVector(me->d_interp, str,0,&vecPtr) != TCL_OK) {
	    fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
		    me->d_interp->result);
	    return -1;
	 }
      }
      // chop it off to one element
      sprintf(command, "gm_timevec set {%f}", time);
      TCLEVALCHECK(me->d_interp, command);

      sprintf(str, "gm_svec");
      //set up the arclength vector.
      if (!Blt_VectorExists(me->d_interp, str))  {
	 //create the vector
	 if (Blt_CreateVector(me->d_interp, str,0,&vecPtr) != TCL_OK) {
	    fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
		    me->d_interp->result);
	    return -1;
	 }
      }
      // chop it off to one element
      sprintf(command, "gm_svec set {%f}", s);
      TCLEVALCHECK(me->d_interp, command);

      //set up the surface x vector.
      sprintf(str, "gm_xsurfvec");
      if (!Blt_VectorExists(me->d_interp, str))  {
	 //create the vector
	 if (Blt_CreateVector(me->d_interp, str,0,&vecPtr) != TCL_OK) {
	    fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
		    me->d_interp->result);
	    return -1;
	 }
      }
      // chop it off to one element
      sprintf(command, "gm_xsurfvec set {%f}", p->x());
      TCLEVALCHECK(me->d_interp, command);

      //set up the y surface vector.
      sprintf(str, "gm_ysurfvec");
      if (!Blt_VectorExists(me->d_interp, str))  {
	 //create the vector
	 if (Blt_CreateVector(me->d_interp, str,0,&vecPtr) != TCL_OK) {
	    fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
		    me->d_interp->result);
	    return -1;
	 }
      }
      // chop it off to one element
      sprintf(command, "gm_ysurfvec set {%f}", p->y());
      TCLEVALCHECK(me->d_interp, command);

      int j = 0;
      // Attach the X and Y surface elements to the y axis of the
      // stripchart. 
      sprintf(command, "add_stripchart_element \"%s\" %d 1.0 0.0 0.0",
		"gm_xsurfvec", j);
      TCLEVALCHECK(me->d_interp, command);
      j++;
      sprintf(command, "add_stripchart_element \"%s\" %d 1.0 0.0 0.0",
		"gm_ysurfvec", j);
      TCLEVALCHECK(me->d_interp, command);
      j++;

      // create a vector for each value returned from microscope
      // and attach it to a graph in the stripchart
      for (value = p->head(); 
	   value != NULL; 
	   value = value->next(), j++) {
	 sprintf(str,"gm_%s_%s", value->name()->Characters(), 
		 value->units()->Characters());

	 // remove spaces - they are bad for vector names.
	 for (unsigned int i = 0; i < strlen(str); i++){
	    if (str[i] == ' ')
	       str[i] = '_';
	 }

	 //printf("graphmod : %s\n", str);

	 if (!Blt_VectorExists(me->d_interp, str))  {
	    if (Blt_CreateVector(me->d_interp, str, 0, &vecPtr) != TCL_OK) {
	       fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
		       me->d_interp->result);
	       return -1;
	    }
	 }
	 // chop it off, and set it's first element
	 sprintf(command, "%s set {%12.9g}", str, value->value());
	 TCLEVALCHECK(me->d_interp, command);

	 // Now, connect this vector to the graph.
	 // and add some controls to turn it on and off, and change it's limits.
	 sprintf(command, "add_stripchart_element \"%s\" %d 1.0 0.0 0.0", 
		str, j);
	 TCLEVALCHECK(me->d_interp, command);
      } // for

      me->d_total_num_points = 1;
      me->d_num_points_graphed = 1;
   } else {
      // this isn't the first point, so we can add data to each vector
      // to graph it. 

      me->d_total_num_points++;
       // If the stride is greater than one, we may skip this point. 
      // Example: d_stride = 2, graph point 0, 2, 4, 6.... 
      // point 0 is always graphed as a special case above. 
      if (((me->d_total_num_points - 1) % me->d_stride) != 0) {
	  return 0;
      }
      me->d_num_points_graphed++;

      // Calculate s by accumulating from the start
      s += sqrt( (p->x() - last_x) * (p->x() - last_x) +
		 (p->y() - last_y) * (p->y() - last_y) );
      last_x = p->x();
      last_y = p->y();
      
      // Calculate time by difference from the start.  Time is in
      // seconds, but is stored in a double
      time = (p->sec()-first_sec) +(p->usec()-first_usec)*0.000001;
      
      // update the time vector
      sprintf(command, "gm_timevec append %f", time);
      TCLEVALCHECK(me->d_interp, command);

      // update the arclength vector
      sprintf(command, "gm_svec append %f", s);
      TCLEVALCHECK(me->d_interp, command);

      // update the x surface vector
      sprintf(command, "gm_xsurfvec append %f", p->x());
      TCLEVALCHECK(me->d_interp, command);

      // update the y surface vector
      sprintf(command, "gm_ysurfvec append %f", p->y());
      TCLEVALCHECK(me->d_interp, command);

      // sanity check on d_max_points
      if (me->d_max_points < 2) {
	  me->d_max_points = 2;
      }
      // If the data vector has become too long, chop off the  
      // first element. 
      if (me->d_num_points_graphed > me->d_max_points) {
	  sprintf(command, "gm_timevec delete 0");
	  TCLEVALCHECK(me->d_interp, command);
	  sprintf(command, "gm_svec delete 0");
	  TCLEVALCHECK(me->d_interp, command);
	  sprintf(command, "gm_xsurfvec delete 0");
	  TCLEVALCHECK(me->d_interp, command);
	  sprintf(command, "gm_ysurfvec delete 0");
	  TCLEVALCHECK(me->d_interp, command);
      }
      // plot the next point for each data value
      me->d_numDataVectors = 0;
      for (value = p->head(); value != NULL; value = value->next()) {
	 // make sure we have a vector to put the value in.
	 sprintf(str,"gm_%s_%s", value->name()->Characters(), 
		 value->units()->Characters());

	 // remove spaces - they are bad for vector names.
	 for (unsigned int i = 0; i < strlen(str); i++) {
	    if (str[i] == ' ')
	       str[i] = '_';
	 }
	 
	 if (!Blt_VectorExists(me->d_interp, str))  {
	    fprintf(stderr, "GraphMod: extra value %s, %s\n", str,
		    me->d_interp->result);
	    break;
	 }
         strcpy(me->d_dataVectorNames[me->d_numDataVectors], str);
         me->d_numDataVectors++;
         if (me->d_numDataVectors == MAX_DATA_VECTORS) {
            fprintf(stderr, "Stripchart Error: out of space (>32 data vectors)\n");
	    break;
         }
	 //Set the value in the vector, so it gets plotted.
	 sprintf(command, "%s append %12.9g", str, value->value());
	 TCLEVALCHECK(me->d_interp, command);
	 // If the data vector has become too long, chop off the
	 // first element.
	 if (me->d_num_points_graphed > me->d_max_points) {
	     sprintf(command, "%s delete 0", str);
	     TCLEVALCHECK(me->d_interp, command);
	 }
      }
      if (me->d_num_points_graphed > me->d_max_points) {
          me->d_num_points_graphed--;
      }

   } // end if-else (me->d_num_points = 0)
   
   return 0;
}


void GraphMod::ShowStripchart (const char * ) {
   char command [100];
   d_interp = get_the_interpreter();
   sprintf(command, "show.stripchart");
   if (Tcl_Eval(d_interp, command) != TCL_OK) {
      fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
	      d_interp->result);
   }	
}

// static
void GraphMod::handle_MaxPointsChange(vrpn_int32 new_value, void * userdata)
{
   int i;
   GraphMod *me = (GraphMod *)userdata;
   char command[100];

   int num_extra_points = me->d_num_points_graphed - me->d_max_points;
   if (num_extra_points > 0) {
      sprintf(command, "gm_timevec delete 0:%d", num_extra_points-1);
      TCLEVALCHECK2(me->d_interp, command);
      sprintf(command, "gm_svec delete 0:%d", num_extra_points-1);
      TCLEVALCHECK2(me->d_interp, command);
      sprintf(command, "gm_xsurfvec delete 0:%d", num_extra_points-1);
      TCLEVALCHECK2(me->d_interp, command);
      sprintf(command, "gm_ysurfvec delete 0:%d", num_extra_points-1);
      TCLEVALCHECK2(me->d_interp, command);
      for (i = 0; i < me->d_numDataVectors; i++) {
         sprintf(command, "%s delete 0:%d", me->d_dataVectorNames[i], num_extra_points-1);
         TCLEVALCHECK2(me->d_interp, command);
      }
      me->d_num_points_graphed -= num_extra_points;
   }
}

// static
void GraphMod::handle_StrideChange(vrpn_int32 new_value, void * userdata)
{
   GraphMod *me = (GraphMod *)userdata;
   
}

