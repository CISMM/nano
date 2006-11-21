/**********************************************************
* Direct Step, contains functions that allow for user to take steps when in xy-lock
*
*
*
*
*
**/

#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <nmm_MicroscopeRemote.h>

#include "AFMState.h"
#include "directstep.h"

/* Do not include microscope.h or interaction.h here because this library is
   used in other applications besides the nanoManipulator.  For microscope
   controllers, this brings in a bunch of un-needed gunk!  Instead, pull out
   the definitions you need until we can re-factor the whole thing better. */
//#include "microscape.h"
//#include <interaction.h>
extern nmm_Microscope_Remote* microscope;
extern nmb_Dataset* dataset;
extern nmb_Decoration* decoration;


//for Tcl variables
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

double x;
double y;
double z_pos;

static q_vec_type X = {1,0,0};
static q_vec_type Y = {0,1,0};
static q_vec_type Z = {0,0,1};
static q_type ds_rot;
static q_vec_type step_vec;
int setting_axis = 0;
int keep_stepping = 0;

enum DS_last_direction { DS_NO_STEP = 0, DS_X_STEP = 1, DS_Y_STEP = 2, DS_Z_STEP = 3 };
DS_last_direction lastDir = DS_NO_STEP;


void set_axis(q_type rot) 
{
	q_copy(ds_rot, rot);
}


TclNet_float step_x("take_x_step",1.0,handle_take_x_step,NULL);
TclNet_float step_y("take_y_step",1.0,handle_take_y_step,NULL);
TclNet_float step_z("take_z_step",1.0,handle_take_z_step,NULL);
Tclvar_int go_to_pos("step_go_to_pos",1,handle_step_go_to_pos,NULL);
TclNet_int stepping( "keep_stepping", 1.0, handle_keep_stepping, NULL );



//takes a step in the +/- X direction when user presses button
void handle_take_x_step(vrpn_float64, void * /*_mptr*/)
{
	//variables for the min and max positions of microscope in real world
	//so that we don't step out of bounds
	
	double min_x,max_x;
	
	//make sure we are in xy_lock, if not, don't take step
	if(!xy_lock){
		return;
	}
	
	BCPlane * plane = dataset->inputGrid->getPlaneByName
		(dataset->heightPlaneName->string());
	
	//find min and max coordinates where we can send microscope
   	min_x = plane->xInWorld(0);
	max_x = plane->xInWorld(plane->numX()-1);
	
	Point_value * _point = 
		microscope->state.data.inputPoint->getValueByPlaneName
		(dataset->heightPlaneName->string());
	
	//this sets x,y to the current position of the microscope
	x = _point->results()->x();
	y = _point->results()->y();
	
	//bounds checking:
	if(min_x > x + step_x || max_x < x + step_x) {
		printf("out of bounds, no step taken");
		return;
	}
	
	//if microscope is has not been sent to a point yet, break
	if(_point->results()->x() == -1 || _point->results()->y() == -1){
		printf("need to set starting point");
		return;
	}
	
	//increment by step_x (step size set from Tcl)
	if(setting_axis) {
		//set the rotation
		if(microscope->state.modify.direct_step_param == DIRECT_STEP_PLANE ) {
			
			//NEXT: NORMALIZE FOR 2D VECTOR
			
			q_xform(step_vec,ds_rot, X);
			q_vec_normalize(step_vec, step_vec);
			x += step_vec[0] * step_x;
			y += step_vec[1] * step_x;
		} else {
			q_xform(step_vec,ds_rot, X);
			q_vec_normalize(step_vec, step_vec);
			x+= step_vec[0] *step_x;
			y+= step_vec[1] *step_x;
			z_pos+= step_vec[2] *step_x;
		}
		
	} else {
		x+=step_x;
	}
	
	//if we want only to step in x/y
	if(microscope->state.modify.direct_step_param == DIRECT_STEP_PLANE ) 
	{
		microscope->TakeModStep(x,y);
	} 
	else if(microscope->state.modify.direct_step_param == DIRECT_STEP_3D)
	{   //3D step
		microscope->TakeDirectZStep(x,y,z_pos);
	} 
	else 
	{
		printf("\nBad param, didn't take step");
	}
	lastDir = DS_X_STEP;
}


//takes a step in the +/- Y direction when user presses button
void handle_take_y_step(vrpn_float64, void * /*_mptr*/)
{	
	
	//variables for the min and max coord's of where we can send microscope
	double min_y,max_y;
	
	//make sure we are in xy_lock, exit if not
	if(!xy_lock)
	{
		return;
	}
	
	BCPlane * plane = dataset->inputGrid->getPlaneByName
		(dataset->heightPlaneName->string());
	
	
	//min and max variables assigned from min and max in BCGrid
	min_y = plane->yInWorld(0);
	max_y = plane->yInWorld(plane->numY()-1);
	
	
	Point_value * _point = 
		microscope->state.data.inputPoint->getValueByPlaneName
		(dataset->heightPlaneName->string());
	
	//set the x/y position to that of microscope
	x = _point->results()->x();
	y = _point->results()->y();
	
	//if we don't know where microscope is, break
	if(_point->results()->x() == -1 || _point->results()->y() == -1){
		return;
	}
	
	//test to make sure that taking a step does not put us out of bounds
	if(min_y > y + step_y || max_y < y + step_y) {
		printf("out of bounds, no step taken");
		return;
	}
	
	if(setting_axis) {
		//set the rotation
		if(microscope->state.modify.direct_step_param == DIRECT_STEP_PLANE ) {
			
			//NEXT: NORMALIZE FOR 2D VECTOR
			
			q_xform(step_vec,ds_rot, Y);
			q_vec_normalize(step_vec, step_vec);
			x += step_vec[0] * step_y;
			y += step_vec[1] * step_y;
		} else {
			q_xform(step_vec,ds_rot, Y);
			q_vec_normalize(step_vec, step_vec);
			x+= step_vec[0] *step_y;
			y+= step_vec[1] *step_y;
			z_pos+= step_vec[2] *step_y;
		}
		
	} else {
		y+=step_y;
	}
	
	/*  OLD
	//increment y position
	y+=step_y;
	*/
	//take a regular step if in directstep plane mode
	if(microscope->state.modify.direct_step_param == DIRECT_STEP_PLANE)
	{
		microscope->TakeModStep(x,y);
	} 
	else if(microscope->state.modify.direct_step_param == DIRECT_STEP_3D) 
	{ //3D step
		microscope->TakeDirectZStep(x,y,z_pos);
	} 
	else 
	{
		printf("\nBad param, didn't take step");
	}
	lastDir = DS_Y_STEP;
}


//takes a step in the +/- Z direction when user presses button
void handle_take_z_step(vrpn_float64, void * /*_mptr*/)
{
	//make sure we are in xy_lock. return if not
	if(!xy_lock) 
	{
		return;
	}
	Point_value * _point = 
		microscope->state.data.inputPoint->getValueByPlaneName
		(dataset->heightPlaneName->string());
	
	x = _point->results()->x();
	y = _point->results()->y();
	
	if(_point->results()->x() == -1 || _point->results()->y() == -1){
		return;	
	}
	
	if(setting_axis) 
	{
		//set the rotation
		q_xform(step_vec,ds_rot, Z);
		q_vec_normalize(step_vec, step_vec);
		x+= step_vec[0] *step_z;
		y+= step_vec[1] *step_z;
		z_pos+= step_vec[2] *step_z;
	} 
	else 
	{
		z_pos+=step_z;
	}
	microscope->TakeDirectZStep(x,y,z_pos);
	lastDir = DS_Z_STEP;
}


//sends microscope to a point when user presses "G0 To Position" button from Tcl
void handle_step_go_to_pos(vrpn_int32, void * /*_mptr*/)
{
	//make sure we are in xy_lock. return if not
	if(!xy_lock) 
	{
		return;
	}
	
	//the microscope coordinates are different than input coordinates.
	double min_x, max_x, min_y, max_y;
	double x_pos,y_pos;
	
	BCPlane * plane = dataset->inputGrid->getPlaneByName
		(dataset->heightPlaneName->string());
	
	
	//find the size of the sample being scanned
	min_x = dataset->inputGrid->minX();
	min_y = dataset->inputGrid->minY();
	max_x = dataset->inputGrid->maxX();
	max_y = dataset->inputGrid->maxY();
	
	//put into microscope coordinates
	x_pos = microscope->state.modify.step_x_pos + plane->xInWorld(0);
	y_pos = microscope->state.modify.step_y_pos + plane->yInWorld(0);
	
	//bounds check:
	if (min_x > x_pos || max_x < x_pos || min_y > y_pos || max_y < y_pos) {
		printf("trying to go to a position out of bounds\n");
		return;
	}
	
	if(microscope->state.modify.direct_step_param == DIRECT_STEP_PLANE)
	{
		microscope->TakeModStep(x_pos,y_pos);
	}
	else if(microscope->state.modify.direct_step_param == DIRECT_STEP_3D)
	{
		z_pos = microscope->state.modify.step_z_pos;
		microscope->TakeDirectZStep(x_pos,y_pos,z_pos);
	} 
	else 
	{
		printf("\nBad param, didn't go to pos\n");
	}
}


void handle_keep_stepping( vrpn_int32 val, void* )
{
	keep_stepping = val;
	lastDir = DS_NO_STEP; 
}


void directStep_mainloop( )
{
	if( keep_stepping ) 
	{
		if( lastDir == DS_X_STEP )
		{
			handle_take_x_step( 0, NULL );
		}
		else if( lastDir == DS_Y_STEP )
		{
			handle_take_y_step( 0, NULL );
		}
		else if( lastDir == DS_Z_STEP )
		{
			handle_take_z_step( 0, NULL );
		}
		// else do nothing
	}
}

