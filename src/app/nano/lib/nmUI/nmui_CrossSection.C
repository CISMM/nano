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


#include <BCGrid.h>
#include <BCPlane.h>
#include <nmb_String.h>
#include <Tcl_Interpreter.h>
#include "nmui_CrossSection.h"


nmui_CrossSection::nmui_CrossSection (void) :
    d_max_points ("xs_max_num_points", 4000),
    d_stride ("xs_stride", 3),
    d_numDataVectors(0),
    d_first_call(1)
{
 d_max_points.addCallback(handle_MaxPointsChange, this);
 d_stride.addCallback(handle_StrideChange, this);

//fprintf(stderr, "GraphMod constructor\n");
}

nmui_CrossSection::~nmui_CrossSection (void) {
  d_max_points.bindConnection(NULL);
  d_stride.bindConnection(NULL);
}


// static
int nmui_CrossSection::ShowCrossSection(BCGrid* grid, 
                                        nmb_ListOfStrings * plane_names, 
                                        int id, int enable,  
                    float center_x,float center_y, float width, 
                    float angle)
{

  //fprintf(stderr, "nmui_CrossSection::ReceiveNewPoint\n");

   //int	numValues;
   char    command[300];
   char    str[300];
   Blt_Vector *dataVecPtr = NULL;
   Blt_Vector *pathVecPtr = NULL;
   int i,j;

      //set up the path vector first.
      sprintf(str, "xs0_Path");
      if (!Blt_VectorExists(Tcl_Interpreter::getInterpreter(), str))  {
	 //create the vector
	 if (Blt_CreateVector(Tcl_Interpreter::getInterpreter(), str,0,&pathVecPtr) != TCL_OK) {
	    fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
		    Tcl_Interpreter::getInterpreter()->result);
	    return -1;
	 }
      } else {
          if (Blt_GetVector(Tcl_Interpreter::getInterpreter(), str, &pathVecPtr) != TCL_OK) {
              fprintf(stderr, "Tcl_Eval(BLT_GetVector) failed: %s\n",
                      Tcl_Interpreter::getInterpreter()->result);
              return -1;
          }
      }
      double step = 2*width/300;
      for ( j=0; j<300; j++) {
          path[j] = j*step;
      }
      if (Blt_ResetVector(pathVecPtr, path, 300, 300,
                          TCL_STATIC) != TCL_OK) {
          fprintf(stderr, "Tcl_Eval(BLT_ResetVector) failed: %s\n",
                  Tcl_Interpreter::getInterpreter()->result);
      }
   
      // create a vector for each data plane
      // and attach it to a graph in the stripchart
      for ( i = 0; i < plane_names->numEntries() ; i++) {
          BCPlane * plane = grid->getPlaneByName(plane_names->entry(i));
          if (!plane) continue;
          // make sure we include name and units in the vector name. 
          sprintf(str,"xs0_%s_%s", plane->name()->Characters(), 
		 plane->units()->Characters());

	 // remove spaces and "-", they are bad for vector names.
	 for (unsigned int k = 0; k < strlen(str); k++){
	    if (str[k] == ' '||str[k] == '-')
	       str[k] = '_';
	 }

	 if (!Blt_VectorExists(Tcl_Interpreter::getInterpreter(), str))  {
	    if (Blt_CreateVector(Tcl_Interpreter::getInterpreter(), str, 0, &dataVecPtr) != TCL_OK) {
	       fprintf(stderr, "Tcl_Eval(BLT_CreateVector) failed: %s\n",
		       Tcl_Interpreter::getInterpreter()->result);
	       return -1;
	    }
            printf("created %s\n", str);
	 } else {
             if (Blt_GetVector(Tcl_Interpreter::getInterpreter(), str, &dataVecPtr) != TCL_OK) {
	       fprintf(stderr, "Tcl_Eval(BLT_GetVector) failed: %s\n",
		       Tcl_Interpreter::getInterpreter()->result);
	       return -1;
	    }
         }

//           strcpy(me->d_dataVectorNames[me->d_numDataVectors], str);
//           me->d_numDataVectors++;
//           if (me->d_numDataVectors == MAX_DATA_VECTORS) {
//              fprintf(stderr, "Stripchart Error: out of space (>32 data vectors)\n");
//  	    break;
//           }
         double stepx = (2*width*cos(angle))/300;
         double stepy = (2*width*sin(angle))/300;
         // Read data values from along the cross section. 
         for (int j=0; j<300; j++) {
             data[i][j] = plane->interpolatedValueAt((j-150)*stepx + center_x,
                                                   (j-150)*stepy + center_y);
         }
         // Send data to tcl to be displayed. 
         if (Blt_ResetVector(dataVecPtr, &(data[i][0]), 300, 300,
                             TCL_STATIC) != TCL_OK) {
	       fprintf(stderr, "Tcl_Eval(BLT_ResetVector) failed: %s\n",
		       Tcl_Interpreter::getInterpreter()->result);
         }
      }
      if (d_first_call) {
          d_first_call = 0;
          sprintf(command, "create_new_cross_section xs0 1");
          TCLEVALCHECK(Tcl_Interpreter::getInterpreter(), command);
      }
   
   return 0;
}


// static
void nmui_CrossSection::handle_MaxPointsChange(vrpn_int32 new_value, void * userdata)
{
    /*   int i;
   nmui_CrossSection *me = (nmui_CrossSection *)userdata;
   char command[100];

   int num_extra_points = me->d_num_points_graphed - me->d_max_points;
   if (num_extra_points > 0) {
      sprintf(command, "gm_timevec delete 0:%d", num_extra_points-1);
      TCLEVALCHECK2(Tcl_Interpreter::getInterpreter(), command);
      sprintf(command, "gm_svec delete 0:%d", num_extra_points-1);
      TCLEVALCHECK2(Tcl_Interpreter::getInterpreter(), command);
      sprintf(command, "gm_Surface_X_Axis delete 0:%d", num_extra_points-1);
      TCLEVALCHECK2(Tcl_Interpreter::getInterpreter(), command);
      sprintf(command, "gm_Surface_Y_Axis delete 0:%d", num_extra_points-1);
      TCLEVALCHECK2(Tcl_Interpreter::getInterpreter(), command);
      if (me->d_is3D) {
          sprintf(command, "gm_Surface_Z_Axis delete 0:%d", num_extra_points-1);
          TCLEVALCHECK2(Tcl_Interpreter::getInterpreter(), command);
      }
      for (i = 0; i < me->d_numDataVectors; i++) {
         sprintf(command, "%s delete 0:%d", me->d_dataVectorNames[i], num_extra_points-1);
         TCLEVALCHECK2(Tcl_Interpreter::getInterpreter(), command);
      }
      me->d_num_points_graphed -= num_extra_points;
   }
    */
}

// static
void nmui_CrossSection::handle_StrideChange(vrpn_int32 new_value, void * userdata)
{
   nmui_CrossSection *me = (nmui_CrossSection *)userdata;
   
}

void nmui_CrossSection::SetupSynchronization(nmui_Component * container)
{
  container->add(&d_max_points);
  container->add(&d_stride);
}

void nmui_CrossSection::TeardownSynchronization(nmui_Component * container)
{
  container->remove(&d_max_points);
  container->remove(&d_stride);
}

