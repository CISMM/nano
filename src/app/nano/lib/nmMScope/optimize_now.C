/* 
This file contains the code for optimize now. Optimize_now takes in the coordinates of 2 points, and
optimizes the box between the 2 points or the line defined by the 2 points, depending on the mode.


 */



#include <BCPlane.h>

#include <nmb_Globals.h>

#include <nmm_Globals.h>
#include <nmm_MicroscopeRemote.h>

#include "optimize_now.h"

void computeOptimizeMinMax(int type, int x0, int y0, int x1, int y1,
			   double *x_max_coord, double *y_max_coord) {

  float x,y;
  double value = -1.0e33; // variable where current value will be stored
  double _max_value = -1.0e33; //the maximum value over the currently optimized points
  int temp,_max_value_x_coord, _max_value_y_coord; //variables where the the cooridinates of the maximum value will be stored, and a temp var.

  BCPlane * plane = dataset->inputGrid->getPlaneByName
    (dataset->heightPlaneName->string());

  Point_value * z_value = 
    microscope->state.data.inputPoint->getValueByPlaneName
    (dataset->heightPlaneName->string());

  //put microscope at point to start optimizing and wait for microscope to get there
    if (type == BCPLANE_OPTIMIZE_AREA) {
      //code for optimizing the area

      //sort so that x0 = xmin, x1 = xmax, y0 = ymin and y1 = ymax;
      if (y0 > y1) { temp = y0; y0 = y1; y1 = temp; }
      if (x0 > x1) { temp = x0; x0 = x1; x1 = temp; }

      //make sure box is in bounds, if not, set boundry to grid boundry
      if(x0 < 0) {x0 = 0;}
      if(y0 < 0) {y0 = 0;}
      if(x1 > plane->numX()) {x1 = plane->numX()-1;}
      if(y1 > plane->numY()) {y1 = plane->numY()-1;}
      
      //in order to do over larger areas, every certain number of points
      //the data must be read in
      //crashing...without this, if too many points are done at one time,
      //nano will crash.. the number of loops is an arbitrary number that works

      int loop_count = 0;

      //these are all the points in the region defined by the box
      for (y = y0; y <= y1; y++) {
	for (x = x0; x <= x1; x++) {
	  loop_count ++;
	  
	  //send microscope over area.
	  microscope->TakeFeelStep(x,y,z_value,0);
	  if((loop_count == 1000) || ((y == (y1)) && (x == (x1)))){
	    //after a number of points (loop_count), let the data be sent from the
	    //microscope and read it into the dataset, find highest point and
	    //then continue with the rest of the points...or if microscope has gone over all points, read in
	    //the remaining points and optimize
	    // don't know what maximum number is, this is just a number that works

	    //reset loop count
	    loop_count = 0;
	    
	    //reads data into dataset(since we are in Optimize_now_mode)
	    microscope->TakeFeelStep(x,y,z_value,1);
	  }
	}
      }
      
      //optimize over newly aquired points
      for (y = y0; y <= y1; y++) {
	for (x = x0; x <= x1; x++) {
	  plane->valueAt(&value,x,y); // gets value at a point
	  dataset->range_of_change.AddPoint(x,y); // this will redraw new points on the screen
	  if (value > _max_value) { 
	    _max_value = value; 
	    _max_value_x_coord = x;
	    _max_value_y_coord = y;
	  }
	}
      }
      
    } else if (type == BCPLANE_OPTIMIZE_LINE) {
      float line_position_param = 0.0;
      // 45-45-90 triangle, longest side is hypotnuse
      // so anything shorter falls within this range
      float step_size = 1.0 / (5.0 * plane->numX() );
      
      while (line_position_param < 1.0) {
	x = x1 * line_position_param + x0 * (1.0 - line_position_param);
	y = y1 * line_position_param + y0 * (1.0 - line_position_param);
	
	microscope->TakeFeelStep(x,y,z_value,0);
	line_position_param += step_size;
   
	if (value > _max_value) { 
	  _max_value = value; 
	  _max_value_x_coord = x;
	  _max_value_y_coord = y;
	}
      }
      //let data be read in from microscope
      microscope->TakeFeelStep(x,y,z_value,1);
      //reset line param to go over newly aquired points
      line_position_param = 0.0;
      
      //go over newly aquired points and optimize
      while (line_position_param < 1.0){
	x = x1 * line_position_param + x0 * (1.0 - line_position_param);
	y = y1 * line_position_param + y0 * (1.0 - line_position_param);

	//gets value at a point
	plane -> valueAt(&value,x,y);
	//draw updated data set on screen
	dataset->range_of_change.AddPoint(x,y);
	
	if (value > _max_value) { 
	  _max_value = value; 
	  _max_value_x_coord = x;
	  _max_value_y_coord = y;
	}
	line_position_param += step_size;
      }
    } else {  // bad tool param spec, shouldn't be here
      return;
    }
    // following returns type double
    *x_max_coord = plane -> xInWorld(_max_value_x_coord);
    *y_max_coord = plane -> yInWorld(_max_value_y_coord);
}

//this updates the Dataset if we are in optimize_now mode, so the display
//is updated and we are optimizing over current data.
int optimize_now_ReceiveNewPoint (void * _mptr, const Point_results * p)
{
  if(microscope->state.modify.tool == OPTIMIZE_NOW){
   BCPlane* plane = dataset->inputGrid->getPlaneByName(
                            dataset->heightPlaneName->string());
   //set new height value
   plane -> setValue(p->x(),p->y(),(p->getValueByPlaneName(
		   dataset->heightPlaneName->string()))->value());

  }
  return 0;
}



