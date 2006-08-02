/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
/* 
   code that implement the functionalities of microscape in openGL 
   created by Qiang Liu 08/10/95

  Change log:
    Tom Hudson 17 Mar 97:  Minimized exposure of internals in header
         and with static declarations.  Commented what I could.
*/

#include <BCPlane.h>
#include <Position.h>

#include <nmb_Dataset.h>
#include <nmb_Types.h>
#include <nmb_Globals.h>
#include <nmb_String.h>

#include <quat.h>

#include "nmg_Funclist.h"
#include "globjects.h"
#include "nmg_State.h" // actually, this is all we need for VERBOSE
#include "openGL.h"
#include "font.h"
#include "chartjunk.h"

#include "Timer.h"
#include "nmg_Globals.h"  // for RegMode enum


//JM, for haptic graphics
#include "nmg_haptic_graphics.h"


#include "nmm_Types.h"  // for OPTIMIZE_NOW

#define ARROW_SCALE  0.005
#define xx .525731112119133606
#define zz .850650808352039932
#define SPHERE_DEPTH 2

// M_PI not defined for VC++, for some reason. 
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

#include <v.h>  // to define GLfloat
typedef GLfloat VertexType[3];

#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif


struct marker_type {
  GLint id;
  marker_type * next;
};


static int poly_rubber_line_id;
static int poly_sweep_rubber_line_id[4];

// Lists of functions to call to draw arbitrary objects in various
// coordinate spaces.
// Set up in init_world_modechange() and rendered in myworld(), myroom(), ...

static nmg_Funclist * vir_world;
static nmg_Funclist * v_room;
static nmg_Funclist * v_head;
static nmg_Funclist * v_hand;
static nmg_Funclist * v_screen;

// indicies of display lists created by similarly-named functions

static GLint vx_quarter_down;
static GLint vx_half_down;
static GLint vx_quarter_up;
static GLint vx_half_up;
static GLint rubber_corner;
static GLint region_box;
static GLint aim_struct;

static GLint red_line_struct;
static GLint green_line_struct;
static GLint blue_line_struct;

static int red_line_struct_id;
static int green_line_struct_id;
static int blue_line_struct_id;

//variables to set up axis for direct step.
static GLint ds_sphere_axis_struct;
static int ds_sphere_axis_struct_id;

//JM haptic
static GLint feelPlane_struct;
static int feelPlane_struct_id;

static GLint feelGrid_struct;
static int feelGrid_struct_id;

static GLint collab_hand_struct;

static marker_type * marker_list; // linked list of markers for selected area 

static GLint sweep_struct;
static GLint sphere; /* dim */

static int aim_struct_id;

static int rubber_corner_id;
static int region_box_id;
static int xs_struct_id;
static int sweep_struct_id;

// ID for marker of user's hand position
static int hand_id;

// ID for marker of actual position of microscope tip
static int trueTip_id;

static int sphere_id;

static int scanline_id;

static int collabHand_id;


//JM additions from TCH branch for showing plane
static int FeelGrid (void *);
static int feelGrid_id;

// nonzero if we're using a CRT and should play with the definition of
// screen space
static int g_CRT_correction = 0;

/*functions prototypes*/
/* remember that v_dlist_ptr_type is a int func_name(int) */

// used in openGL.c
int myworld (void);

static void myobjects (nmg_State * state);
static int draw_list (void *);
static int grabhand (void *);
static int lighthand (void * data);

// User's hand position
static int Tip (void *);

// Last known position of actual microscope tip
// Needed for ad-hoc latency compensation
static int TrueTip (void *);

static int measure_hand (void *);
static int vx_down_icon (void *);
static int vx_up_icon (void *);
static int draw_north_pointing_arrow (void *);

static int make_line (const float a [], const float b []);
static int make_line (const double a [], const double b []);
int my_line_mark (void *);
int my_rubber_line (void *);
int my_scanline_indicator (void *);


int replaceDefaultObjects (nmg_State * state);
int make_aim (const float a [], const float b []);
int make_sweep (nmg_State * state, const float a [], const float b [],
		const float c [], const float d [] );
int make_rubber_corner (float, float, float, float, int);
int make_region_box (float, float, float, float, float, int);

static int draw_cross_section(void * data );


static int selecthand (void *);
static int mycube (void);
static void init_sphere (void); /* dim */

void position_sphere (nmg_State * state,float, float, float);

static float sphere_x, sphere_y, sphere_z;
q_vec_type fp_origin_j, fp_normal_j;
int config_feelPlane_temp;
int config_feelGrid_temp;

/* When the user changes modes, clear the world of any 
 * icons dependent on that mode. */
int clear_world_modechange(nmg_State * state, 
                           int mode, int style, int tool_param)
{
  switch(mode) {
  case USER_LIGHT_MODE:
    removeFunctionFromFunclist(&vir_world,hand_id);
    break;
  case USER_FLY_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    break;
  case USER_MEASURE_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    break;
    //  case USER_PULSE_MODE:
    //    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    //    removeFunctionFromFunclist(&vir_world,sphere_id);
    //    break;
  case USER_PLANE_MODE:
    removeFunctionFromFunclist(&vir_world,sphere_id);
    removeFunctionFromFunclist(&v_hand,hand_id);
    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    break;
  case USER_LINE_MODE:
  case USER_PLANEL_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    removeFunctionFromFunclist(&vir_world,sphere_id);
    removeFunctionFromFunclist(&vir_world, aim_struct_id);
    if (state->config_trueTip) {
      removeFunctionFromFunclist(&vir_world, trueTip_id);
    }
    if (style == SWEEP) {
      removeFunctionFromFunclist(&vir_world,sweep_struct_id);
    }

    break;
    //  case USER_SWEEP_MODE:
    //    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    //    removeFunctionFromFunclist(&vir_world,sweep_struct_id);
    //    break;
    //  case USER_BLUNT_TIP_MODE:
    //    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    //    break;
    //  case USER_COMB_MODE:
    //    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    //    removeFunctionFromFunclist(&vir_world,sweep_struct_id);
    //    break;
  case USER_SCALE_UP_MODE:
    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    removeFunctionFromFunclist(&v_hand,hand_id);
    break;
  case USER_SCALE_DOWN_MODE:
    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    removeFunctionFromFunclist(&v_hand,hand_id);
    break;
  case USER_SCANLINE_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    break;
  case USER_SERVO_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    removeFunctionFromFunclist(&vir_world,rubber_corner_id);
    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    if (tool_param == OPTIMIZE_NOW_AREA) {
      removeFunctionFromFunclist(&vir_world,sphere_id);
    }
    break;
  case USER_GRAB_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    break;
  case USER_CENTER_TEXTURE_MODE:
    removeFunctionFromFunclist(&vir_world,sphere_id);
    break;
  case USER_REGION_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    removeFunctionFromFunclist(&vir_world,region_box_id);
    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    break;    
  case USER_CROSS_SECTION_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    removeFunctionFromFunclist(&vir_world,xs_struct_id );
    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    break;    
  default:
    break;
  }

  // We added them in init_world_modechange, so we should remove them here.
  removeFunctionFromFunclist(&vir_world, red_line_struct_id);
  removeFunctionFromFunclist(&vir_world, green_line_struct_id );
  removeFunctionFromFunclist(&vir_world, blue_line_struct_id );
  scanline_id = removeFunctionFromFunclist(&vir_world, scanline_id );

  return(0);
}

/* This function is called the first time we enter a mode.
 * It adds icons appropriate to the mode.
 * If the icon is added to the v_hand funclist, it moves with
 * the hand/phantom, with no further intervention.
 * If the icon is added to the vir_world funclist, it is
 * stationary with respect to the surface. It can be moved 
 * explicitly, though, see make_aim() and make_sweep()
 */
int init_world_modechange(nmg_State * state, 
                          int mode, int style, int tool_param)
{
  if (state->config_planeonly) {
    // display NOTHING but the plane - used by nmg_RenderServer
    return 0;
  }
  switch(mode) {
  case USER_LIGHT_MODE:
    hand_id = addFunctionToFunclist(&vir_world,lighthand,state, "hand_scale");
    break;
  case USER_FLY_MODE:
    hand_id = addFunctionToFunclist(&v_hand, Tip, state, "Tip");
    break;
  case USER_MEASURE_MODE:
    hand_id = addFunctionToFunclist(&v_hand,measure_hand, state,
                                                "measure_hand"); 
    break;
  case USER_PLANE_MODE:
    sphere_id = addFunctionToFunclist(&vir_world,mysphere,state, "mysphere");
    hand_id = addFunctionToFunclist(&v_hand, Tip, state, "Tip"); 
    aim_struct_id = addFunctionToFunclist(&vir_world,draw_list,&aim_struct,
                                                "draw_list(aim_struct)");
    break;
  case USER_LINE_MODE:
  case USER_PLANEL_MODE:
    hand_id = addFunctionToFunclist(&v_hand, Tip, state, "Tip"); 
    sphere_id = addFunctionToFunclist(&vir_world, mysphere, state, "mysphere");
    aim_struct_id = addFunctionToFunclist(&vir_world, draw_list, &aim_struct,
					  "draw_list(aim_struct)");
    if (state->config_trueTip) {
      trueTip_id = addFunctionToFunclist(&vir_world, TrueTip, state, "true tip");
    }
	//JM - addition from TCH branch , pass in state data also
    /*
	if (state->config_feelGrid) {
       feelGrid_id = addFunctionToFunclist(&vir_world, FeelGrid, state,
	     "feel grid");
    }
    */

    break;
  case USER_SCALE_UP_MODE:
    aim_struct_id = addFunctionToFunclist(&vir_world, draw_list, &aim_struct,
					  "draw_list(aim_struct)");
    hand_id = addFunctionToFunclist(&v_hand, vx_up_icon, NULL, "vx_up_icon");
    break;
  case USER_SCALE_DOWN_MODE:
    aim_struct_id = addFunctionToFunclist(&vir_world, draw_list, &aim_struct,
					  "draw_list(aim_struct)");
    hand_id = addFunctionToFunclist(&v_hand, vx_down_icon, NULL,
                                    "vx_down_icon");
    break;
  case USER_SERVO_MODE:
    hand_id = addFunctionToFunclist(&v_hand, selecthand, NULL, "selecthand");
    rubber_corner_id = addFunctionToFunclist(&vir_world, draw_list,
					     &rubber_corner,
                                             "draw_list(rubber_corner)");
    aim_struct_id = addFunctionToFunclist(&vir_world, draw_list, &aim_struct,
                                          "draw_list(aim_struct)");
    if (tool_param == OPTIMIZE_NOW_AREA) {
      sphere_id = addFunctionToFunclist(&vir_world, mysphere, state, "mysphere");
    }
    break;
  case USER_GRAB_MODE:
    aim_struct_id = addFunctionToFunclist(&vir_world, draw_list, &aim_struct,
					  "draw_list(aim_struct)");
    hand_id = addFunctionToFunclist(&v_hand,grabhand,state, "grabhand");   
    break;
  case USER_SCANLINE_MODE:
    hand_id = addFunctionToFunclist(&v_hand, selecthand, NULL, "selecthand");
    break;
  case USER_CENTER_TEXTURE_MODE:
    sphere_id = addFunctionToFunclist(&vir_world,mysphere,state,"mysphere"); 
    fprintf( stderr, "Sphere added to functionlist\n" );
    break;
  case USER_REGION_MODE:
    hand_id = addFunctionToFunclist(&v_hand, selecthand, NULL, "selecthand");
    region_box_id = addFunctionToFunclist(&vir_world, draw_list,
					     &region_box,
                                             "draw_list(region_box)");
    aim_struct_id = addFunctionToFunclist(&vir_world, draw_list, &aim_struct,
                                          "draw_list(aim_struct)");
    break;
  case USER_CROSS_SECTION_MODE:
    hand_id = addFunctionToFunclist(&v_hand, selecthand, NULL, "selecthand");
    xs_struct_id = addFunctionToFunclist(&vir_world, draw_cross_section, 
                                         state, "draw_cross_section");
    aim_struct_id = addFunctionToFunclist(&vir_world, draw_list, &aim_struct,
                                          "draw_list(aim_struct)");
    break;
  default:
    break;
  }

  /* a few icons get added every time, no matter what mode we are
   * entering */
  addFunctionToFunclist(&vir_world,draw_north_pointing_arrow, state,
                        "draw_north_pointing_arrow");  /* dim */
  if (state->config_measurelines) {
    red_line_struct_id = addFunctionToFunclist(&vir_world, draw_list, &red_line_struct,
					       "draw_list(red_line_struct)"); 
    green_line_struct_id = addFunctionToFunclist(&vir_world, draw_list, &green_line_struct,
					       "draw_list(green_line_struct)"); 
    blue_line_struct_id = addFunctionToFunclist(&vir_world, draw_list, &blue_line_struct,
					       "draw_list(blue_line_struct)"); 
  }
  if (state->scanline_display_enabled) {
	scanline_id = addFunctionToFunclist(&vir_world, my_scanline_indicator,
		(void *)state, "scanline_indicator");
  }

  return 0;
}

void enableCollabHand (nmg_State * state, int enable) {
  state->draw_collab_hand = enable;
  if (enable) {
    collabHand_id =
        addFunctionToFunclist(&vir_world, draw_list, &collab_hand_struct,
			      "draw_list(collab_hand_struct)");
  } else {
    removeFunctionFromFunclist(&vir_world, collabHand_id);
  }
}

// collabHand_id:
//   add in enableCollabHand(VRPN_TRUE)
//   remove in clear_world_modechange()
//   add in init_world_modechange()
//   ...
//   remove in enableCollabHand(VRPN_FALSE)

// function to toggle display of the scanline position icon:
void enableScanlinePositionDisplay(nmg_State * state, const int enable) {

    state->scanline_display_enabled = enable;
    if (enable){
	scanline_id = addFunctionToFunclist(&vir_world,my_scanline_indicator,
                                (void *)state, "scanline_indicator");
    } else {
	removeFunctionFromFunclist(&vir_world, scanline_id);
    }
}

/* objects in the world space*/
void myobjects(nmg_State * state)
{
  int i;
  static VertexType downshaft[4] =   {{ 20,  3,    0 },
				      {  2,  0.01, 0 },
				      {  2, -0.01, 0 },
				      { 20, -3,    0 }};
  
  static VertexType downarrowhead[3] =   {{ 3,  2, 0 },     
					  { 1,  0, 0 },     
					  { 3, -2, 0 }};

  static VertexType upshaft[] =   {{ 10,  1,    0 },         
				   {  0,  0.01, 0 },         
				   {  0, -0.01, 0 },        
				   { 10, -1,    0 }};
    
  static VertexType uparrowhead[] =   {{ 10,  3, 0 },
				       { 10, -3, 0 }, 
				       { 14,  0, 0 }};

  v_gl_set_context_to_vlib_window(); 
  glNewList(aim_struct,GL_COMPILE);
		/* empty list?*/
  glEndList();
 
  glNewList(red_line_struct, GL_COMPILE);
		/* empty list?*/
  glEndList();
 
  glNewList(green_line_struct, GL_COMPILE);
		/* empty list?*/
  glEndList();

  glNewList(blue_line_struct, GL_COMPILE);
		/* empty list?*/
  glEndList();

  glNewList(collab_hand_struct, GL_COMPILE);
  glEndList();

  //show lines where old selected areas were
  glNewList(marker_list->id, GL_COMPILE); /* empty list?*/
  glEndList();

  glNewList(rubber_corner,GL_COMPILE);
		/*empty list?*/
  glEndList();

  glNewList(region_box,GL_COMPILE);
  glEndList();

  glNewList(sweep_struct,GL_COMPILE);
		/*empty list?*/
  glEndList();

  glNewList(ds_sphere_axis_struct, GL_COMPILE);
  glEndList();
  
  glNewList(feelPlane_struct, GL_COMPILE);
  glEndList();

  glNewList(feelGrid_struct, GL_COMPILE);
  glEndList();


  /* vx_quarter_up structure */
  glNewList(vx_quarter_up,GL_COMPILE);
    glPushMatrix();
    glScalef(ARROW_SCALE * state->icon_scale,
	     ARROW_SCALE * state->icon_scale,
	     ARROW_SCALE * state->icon_scale);
    glRotatef(180.0, 0.0,0.0,1.0);

    glBegin(GL_POLYGON);  
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      for(i=0;i<4;i++)
        glVertex3fv(upshaft[i]);
      VERBOSE(20, "          glEnd()");
    glEnd();
  
    glBegin(GL_POLYGON);  
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      for(i=0;i<3;i++)
        glVertex3fv(uparrowhead[i]);
      VERBOSE(20, "          glEnd()");
    glEnd();
    glPopMatrix();
  glEndList();

  /* vx_half_up structure */
  glNewList(vx_half_up,GL_COMPILE);
    glPushMatrix();
    glCallList(vx_quarter_up);
    glRotatef(90.0, 1.0,0.0,0.0);
    glCallList(vx_quarter_up);
    glPopMatrix();
  glEndList();

  /* vx_quarter down structure */
  glNewList(vx_quarter_down,GL_COMPILE);
    glPushMatrix();
    glScalef(ARROW_SCALE * state->icon_scale,
	     ARROW_SCALE * state->icon_scale,
	     ARROW_SCALE * state->icon_scale);
    glRotatef(180.0, 0.0,0.0,1.0);

    glBegin(GL_POLYGON);  
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      for(i=0;i<4;i++)
        glVertex3fv(downshaft[i]);
      VERBOSE(20, "          glEnd()");
    glEnd();
  
    glBegin(GL_POLYGON);  
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      for(i=0;i<3;i++)
        glVertex3fv(downarrowhead[i]);
      VERBOSE(20, "          glEnd()");
    glEnd();
    glPopMatrix();
  glEndList();

  /*gl_half_down structure */
  glNewList(vx_half_down,GL_COMPILE);
    glPushMatrix();
    glCallList(vx_quarter_down);
    glRotatef(90.0, 1.0,0.0,0.0);
    glCallList(vx_quarter_down);
    glPopMatrix();
  glEndList();

}

int make_rubber_corner(nmg_State * state, 
                       float x_min,float y_min, float x_max,float y_max, 
                       int highlight_mask)
{
        VertexType Points[4];
        float normal_color[4] = {1.0,0.0,0.0,0.5};
        float highlight_color[4] = {1.0,0.0,0.0,0.15};
	float z_min,z_max;
        float x_offset, y_offset;
	int i;

	BCPlane* plane = state->inputGrid->getPlaneByName
                    (state->heightPlaneName);
	if (plane == NULL)
	{
	    fprintf(stderr, "Error in make_rubber_corner: could not get plane!\n");
	    return -1;
	}

        // Init with reasonable value. 
	z_min=z_max= plane->scaledValue(0,0);
        // If min max equal, plane has no real data.
        if (plane->maxNonZeroValue() > plane->minNonZeroValue()) {
            z_min=plane->minNonZeroValue()*plane->scale();
            z_max=plane->maxNonZeroValue()*plane->scale();
        }
        // Add 10 nm to edges to make sure it's visible. 
	z_min=z_min-10*plane->scale();
	z_max=z_max+10*plane->scale();

        x_offset = 0.4*fabs(x_max - x_min);
        y_offset = 0.4*fabs(y_max - y_min);

	v_gl_set_context_to_vlib_window(); 
	glNewList(rubber_corner,GL_COMPILE);

        if ((highlight_mask & REG_SIZE)||(highlight_mask & REG_SIZE_HEIGHT)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
  	Points[0][Z] = z_min;
	Points[0][X]=x_min;
	Points[0][Y]=y_min;

	Points[1][Z] = z_min;
	Points[1][X]=x_max;
	Points[1][Y]=y_min;

  	Points[3][Z] = z_max;
	Points[3][X]=x_min;
	Points[3][Y]=y_min;

	Points[2][Z] = z_max;
	Points[2][X]=x_max;
	Points[2][Y]=y_min;

	glBegin(GL_POLYGON);
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++)
	  glVertex3fv(Points[i]);
        VERBOSE(20, "          glEnd()");
	glEnd();

        // Central target
        if ((highlight_mask & REG_TRANSLATE)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
	Points[0][X]+=x_offset;
	Points[0][Y]+=y_offset;

	Points[1][X]-=x_offset;
	Points[1][Y]+=y_offset;

	Points[3][X]+=x_offset;
	Points[3][Y]+=y_offset;

	Points[2][X]-=x_offset;
	Points[2][Y]+=y_offset;

	glBegin(GL_POLYGON);
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++)
	  glVertex3fv(Points[i]);
        VERBOSE(20, "          glEnd()");
	glEnd();

        if ((highlight_mask & REG_SIZE)||(highlight_mask & REG_SIZE_WIDTH)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
  	Points[0][Z] = z_min;
	Points[0][X]=x_max;
	Points[0][Y]=y_min;

	Points[1][Z] = z_min;
	Points[1][X]=x_max;
	Points[1][Y]=y_max;

  	Points[3][Z] = z_max;
	Points[3][X]=x_max;
	Points[3][Y]=y_min;

	Points[2][Z] = z_max;
	Points[2][X]=x_max;
	Points[2][Y]=y_max;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

        // Central target
        if ((highlight_mask & REG_TRANSLATE)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
	Points[0][X]-=x_offset;
	Points[0][Y]+=y_offset;

	Points[1][X]-=x_offset;
	Points[1][Y]-=y_offset;

	Points[3][X]-=x_offset;
	Points[3][Y]+=y_offset;

	Points[2][X]-=x_offset;
	Points[2][Y]-=y_offset;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

        if ((highlight_mask & REG_SIZE)||(highlight_mask & REG_SIZE_HEIGHT)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
  	Points[0][Z] = z_min;
	Points[0][X]=x_max;
	Points[0][Y]=y_max;

	Points[1][Z] = z_min;
	Points[1][X]=x_min;
	Points[1][Y]=y_max;

  	Points[3][Z] = z_max;
	Points[3][X]=x_max;
	Points[3][Y]=y_max;

	Points[2][Z] = z_max;
	Points[2][X]=x_min;
	Points[2][Y]=y_max;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

        // Central target
        if ((highlight_mask & REG_TRANSLATE)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
	Points[0][X]-=x_offset;
	Points[0][Y]-=y_offset;

	Points[1][X]+=x_offset;
	Points[1][Y]-=y_offset;

	Points[3][X]-=x_offset;
	Points[3][Y]-=y_offset;

	Points[2][X]+=x_offset;
	Points[2][Y]-=y_offset;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

        if ((highlight_mask & REG_SIZE)||(highlight_mask & REG_SIZE_WIDTH)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
  	Points[0][Z] = z_min;
	Points[0][X]=x_min;
	Points[0][Y]=y_max;

	Points[1][Z] = z_min;
	Points[1][X]=x_min;
	Points[1][Y]=y_min;

  	Points[3][Z] = z_max;
	Points[3][X]=x_min;
	Points[3][Y]=y_max;

	Points[2][Z] = z_max;
	Points[2][X]=x_min;
	Points[2][Y]=y_min;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();
  
        // Central target
        if ((highlight_mask & REG_TRANSLATE)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
	Points[0][X]+=x_offset;
	Points[0][Y]-=y_offset;

	Points[1][X]+=x_offset;
	Points[1][Y]+=y_offset;

	Points[3][X]+=x_offset;
	Points[3][Y]-=y_offset;

	Points[2][X]+=x_offset;
	Points[2][Y]+=y_offset;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

	glEndList();
	return(0);
}

int make_region_box(nmg_State * state, 
                    float center_x,float center_y, float width,float height, 
                    float angle, int highlight_mask)
{
        VertexType Points[4];
        float normal_color[4] = {0.6,0.0,0.6,0.5};
        float highlight_color[4] = {0.8,0.0,0.8,0.15};
	float z_min,z_max;
        float x_offset, y_offset;
	int i;

	BCPlane* plane = state->inputGrid->getPlaneByName
                    (state->heightPlaneName);
	if (plane == NULL)
	{
	    fprintf(stderr, "Error in make_region_box: could not get plane!\n");
	    return -1;
	}
        
        // Init with reasonable value. 
	z_min=z_max= plane->scaledValue(0,0);
        // If min max equal, plane has no real data.
        if (plane->maxNonZeroValue() > plane->minNonZeroValue()) {
            z_min=plane->minNonZeroValue()*plane->scale();
            z_max=plane->maxNonZeroValue()*plane->scale();
        }
        // Add 10 nm to edges to make sure it's visible. 
	z_min=z_min-10*plane->scale();
	z_max=z_max+10*plane->scale();

        x_offset = 0.8*fabs(width);
        y_offset = 0.8*fabs(height);

	v_gl_set_context_to_vlib_window(); 
	glNewList(region_box,GL_COMPILE);
        glTranslatef(center_x, center_y, 0);
        glRotatef(angle, 0.0,0.0,1.0);
        glTranslatef(-center_x, -center_y, 0);
        
        if ((highlight_mask & REG_SIZE)|(highlight_mask & REG_PREP_SIZE)|
            (highlight_mask & REG_SIZE_HEIGHT) |
            (highlight_mask & REG_PREP_SIZE_HEIGHT)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
  	Points[0][Z] = z_min;
	Points[0][X]=center_x - width;
	Points[0][Y]=center_y - height;

	Points[1][Z] = z_min;
	Points[1][X]=center_x + width;
	Points[1][Y]=center_y - height;

  	Points[3][Z] = z_max;
	Points[3][X]=center_x - width;
	Points[3][Y]=center_y - height;

	Points[2][Z] = z_max;
	Points[2][X]=center_x + width;
	Points[2][Y]=center_y - height;

	glBegin(GL_POLYGON);
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++)
	  glVertex3fv(Points[i]);
        VERBOSE(20, "          glEnd()");
	glEnd();

        // Central target
        if ((highlight_mask & REG_TRANSLATE) | 
            (highlight_mask & REG_PREP_TRANSLATE)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
	Points[0][X]+=x_offset;
	Points[0][Y]+=y_offset;

	Points[1][X]-=x_offset;
	Points[1][Y]+=y_offset;

	Points[3][X]+=x_offset;
	Points[3][Y]+=y_offset;

	Points[2][X]-=x_offset;
	Points[2][Y]+=y_offset;

	glBegin(GL_POLYGON);
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++)
	  glVertex3fv(Points[i]);
        VERBOSE(20, "          glEnd()");
	glEnd();


        if ((highlight_mask & REG_SIZE)|(highlight_mask & REG_PREP_SIZE)|
            (highlight_mask & REG_SIZE_WIDTH)|
            (highlight_mask & REG_PREP_SIZE_WIDTH)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
  	Points[0][Z] = z_min;
	Points[0][X]=center_x + width;
	Points[0][Y]=center_y - height;

	Points[1][Z] = z_min;
	Points[1][X]=center_x + width;
	Points[1][Y]=center_y + height;

  	Points[3][Z] = z_max;
	Points[3][X]=center_x + width;
	Points[3][Y]=center_y - height;

	Points[2][Z] = z_max;
	Points[2][X]=center_x + width;
	Points[2][Y]=center_y + height;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

        // Central target
        if ((highlight_mask & REG_TRANSLATE) | 
            (highlight_mask & REG_PREP_TRANSLATE)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }

	Points[0][X]-=x_offset;
	Points[0][Y]+=y_offset;

	Points[1][X]-=x_offset;
	Points[1][Y]-=y_offset;

	Points[3][X]-=x_offset;
	Points[3][Y]+=y_offset;

	Points[2][X]-=x_offset;
	Points[2][Y]-=y_offset;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

        if ((highlight_mask & REG_SIZE)|(highlight_mask & REG_PREP_SIZE)|
            (highlight_mask & REG_SIZE_HEIGHT) |
            (highlight_mask & REG_PREP_SIZE_HEIGHT)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
  	Points[0][Z] = z_min;
	Points[0][X]=center_x + width;
	Points[0][Y]=center_y + height;

	Points[1][Z] = z_min;
	Points[1][X]=center_x - width;
	Points[1][Y]=center_y + height;

  	Points[3][Z] = z_max;
	Points[3][X]=center_x + width;
	Points[3][Y]=center_y + height;

	Points[2][Z] = z_max;
	Points[2][X]=center_x - width;
	Points[2][Y]=center_y + height;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

        // Central target
        if ((highlight_mask & REG_TRANSLATE) | 
            (highlight_mask & REG_PREP_TRANSLATE)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }

	Points[0][X]-=x_offset;
	Points[0][Y]-=y_offset;

	Points[1][X]+=x_offset;
	Points[1][Y]-=y_offset;

	Points[3][X]-=x_offset;
	Points[3][Y]-=y_offset;

	Points[2][X]+=x_offset;
	Points[2][Y]-=y_offset;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

        if ((highlight_mask & REG_SIZE)|(highlight_mask & REG_PREP_SIZE)|
            (highlight_mask & REG_SIZE_WIDTH)|
            (highlight_mask & REG_PREP_SIZE_WIDTH)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
  	Points[0][Z] = z_min;
	Points[0][X]=center_x - width;
	Points[0][Y]=center_y + height;

	Points[1][Z] = z_min;
	Points[1][X]=center_x - width;
	Points[1][Y]=center_y - height;

  	Points[3][Z] = z_max;
	Points[3][X]=center_x - width;
	Points[3][Y]=center_y + height;

	Points[2][Z] = z_max;
	Points[2][X]=center_x - width;
	Points[2][Y]=center_y - height;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();
  
        // Central target
        if ((highlight_mask & REG_TRANSLATE) | 
            (highlight_mask & REG_PREP_TRANSLATE)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }

	Points[0][X]+=x_offset;
	Points[0][Y]-=y_offset;

	Points[1][X]+=x_offset;
	Points[1][Y]+=y_offset;

	Points[3][X]+=x_offset;
	Points[3][Y]-=y_offset;

	Points[2][X]+=x_offset;
	Points[2][Y]+=y_offset;

	glBegin(GL_POLYGON); 
        VERBOSE(20, "          glBegin(GL_POLYGON)");
	for(i=0;i<4;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

	glEndList();
	return(0);
}

int hide_cross_section(nmg_State * state, int id ) 
{
    state->xs_state[id].enabled = 0;
    return 0;
}

int move_cross_section(nmg_State * state, int id, int enable,  
                       float center_x,float center_y, 
                       float widthL, float widthR, 
                       float angle, int highlight_mask)
{
    if (id < 0 || id > 2) return 0;
    state->xs_state[id].enabled = enable;
    if(enable) {
        state->xs_state[id].center_x = center_x;
        state->xs_state[id].center_y = center_y;
        state->xs_state[id].widthL = widthL;
        state->xs_state[id].widthR = widthR;
        state->xs_state[id].angle = angle;
        state->xs_state[id].highlight_mask = highlight_mask;
    }
    return 0;
}

int draw_cross_section(void * data )
{
    nmg_State * state = (nmg_State *) data;

    VertexType Points[6];
    float z_min,z_max, z_range;
    float normal_color[4] = {0.9,0.0,0.0,1};
    float highlight_color[4] = {0.9,0.0,0.9,0.5};
    BCPlane* plane = state->inputGrid->getPlaneByName
        (state->heightPlaneName);
    if (plane == NULL)
    {
        fprintf(stderr, "Error in make_region_box: could not get plane!\n");
        return -1;
    }
    
    // Init with reasonable value. 
    z_min=z_max= plane->scaledValue(0,0);
    // If min max equal, plane has no real data.
    if (plane->maxNonZeroValue() > plane->minNonZeroValue()) {
        z_min=plane->minNonZeroValue()*plane->scale();
        z_max=plane->maxNonZeroValue()*plane->scale();
    }
    // Add 10 nm to edges to make sure it's visible. 
    z_min=z_min-10*plane->scale();
    z_max=z_max+10*plane->scale();
    z_range = z_max-z_min;

    glPushAttrib(GL_CURRENT_BIT);
    for (int id=0; id < 2; id++) {
      if (state->xs_state[id].enabled != 0) {
        glPushMatrix();
        glTranslatef(state->xs_state[id].center_x, 
                     state->xs_state[id].center_y, 0);
        glRotatef(state->xs_state[id].angle, 0.0,0.0,1.0);
        
  	Points[0][Z]= z_min;
	Points[0][X]= -state->xs_state[id].widthL;
	Points[0][Y]= 0;

	Points[1][Z]= z_max;
	Points[1][X]= -state->xs_state[id].widthL;
	Points[1][Y]= 0;

  	Points[2][Z]= z_min;
	Points[2][X]= 0;
	Points[2][Y]= 0;

	Points[3][Z]= z_max;
	Points[3][X]= 0;
	Points[3][Y]= 0;

  	Points[4][Z]= z_min;
	Points[4][X]= state->xs_state[id].widthR;
	Points[4][Y]= 0;

	Points[5][Z]= z_max;
	Points[5][X]= state->xs_state[id].widthR;
	Points[5][Y]= 0;

        glLineWidth(3.0);
        if ((state->xs_state[id].highlight_mask & REG_SIZE_WIDTH)|
            (state->xs_state[id].highlight_mask & REG_PREP_SIZE_WIDTH) |
            (state->xs_state[id].highlight_mask & REG_SIZE_HEIGHT)|
            (state->xs_state[id].highlight_mask & REG_PREP_SIZE_HEIGHT)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
        make_line(Points[0],Points[1]);
        make_line(Points[4],Points[5]);
        if ((state->xs_state[id].highlight_mask & REG_TRANSLATE)|
            (state->xs_state[id].highlight_mask & REG_PREP_TRANSLATE)) {
            glColor4fv(highlight_color);
        } else {
            glColor4fv(normal_color);
        }
        make_line(Points[2],Points[3]);

        glColor4fv(normal_color);
        glLineWidth(1.0);
        make_line(Points[0],Points[4]);
        Points[0][2] = z_min + 0.1667*z_range;
        Points[4][2] = z_min + 0.1667*z_range;
        make_line(Points[0],Points[4]);
        Points[0][2] = z_min + 0.3333*z_range;
        Points[4][2] = z_min + 0.3333*z_range;
        make_line(Points[0],Points[4]);
        Points[0][2] = z_min + 0.5*z_range;
        Points[4][2] = z_min + 0.5*z_range;
        make_line(Points[0],Points[4]);
        Points[0][2] = z_min + 0.6667*z_range;
        Points[4][2] = z_min + 0.6667*z_range;
        make_line(Points[0],Points[4]);
        Points[0][2] = z_min + 0.8333*z_range;
        Points[4][2] = z_min + 0.8333*z_range;
        make_line(Points[0],Points[4]);
        make_line(Points[1],Points[5]);

        glPopMatrix();
      }
      // Hack - 2nd cross section is a different color
      normal_color[0] = 0.0;
      normal_color[1] = 0.8;
      normal_color[2] = 0.8;
//          highlight_color[0] = 
//          highlight_color[1] =
//          highlight_color[2] =

    }
    glPopAttrib();
    return(0);
}

int make_line (const float a [], const float b [])
{
  glBegin(GL_LINES);
  VERBOSE(20, "          glBegin(GL_LINES)");
  glVertex3fv(a);
  glVertex3fv(b);
  VERBOSE(20, "          glEnd()");
  glEnd();
  return 0;
}

int make_line(const double a[], const double b[])
{
  glBegin(GL_LINES);
  VERBOSE(20, "          glBegin(GL_LINES)");
  glVertex3dv(a);
  glVertex3dv(b);
  VERBOSE(20, "          glEnd()");
  glEnd();
  return 0;
}

int make_aim (const float a [], const float b [])
{
  v_gl_set_context_to_vlib_window();
  glDeleteLists(aim_struct,1);
  aim_struct = glGenLists(1);
  glNewList(aim_struct,GL_COMPILE);
  glColor3f(0.0,1.0,0.0);
  make_line(a,b);
  glEndList();
  return(aim_struct);
}

static int make_cone ()
{
  int i;

  glRotated(-90.0, 0.0, 1.0, 0.0);	//Yes, this is necessary.

  glBegin(GL_TRIANGLE_FAN);
    glVertex3f(750, 0.0, 0.0);
    for (i = 0; i < 17; i++) {
      glVertex3f(750, 100.0*cos(i*M_PI/8.0), 100.0*sin(i*M_PI/8.0));
    } 
  glEnd();

  glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0, 0.0, 0.0);
    for (i = 0; i < 17; i++) {
      glVertex3f(750, 100.0*cos(i*M_PI/8.0), 100.0*sin(i*M_PI/8.0));
    }
  glEnd();
  return 0;
}

static char     *MODE_NAMES[] = {
                        "fly",
                        "grab",
                        "scale up",
                        "scale down",
                        "select",
                        "pulse",
                        "measure line",
                        "line",
                        "sweep",
                        "measure",
                        "position light",
                        "touch",
                        "modify",
                        "blunt tip",
                        "comb",
                        "center texture",
                        "line scan",
			"region select",
			"cross section"
 };

int make_collab_hand_icon (double pos[], double rotate[], int mode) {
  v_xform_type headxform;
  double rot_mat[16];
  float invscale;

  q_to_ogl_matrix(rot_mat, rotate);

  v_gl_set_context_to_vlib_window(); 
  glDeleteLists(collab_hand_struct, 1);
  collab_hand_struct = glGenLists(1);
  glNewList(collab_hand_struct, GL_COMPILE);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glRasterPos3d(pos[0], pos[1], pos[2]);
    drawStringInFont(myfont, MODE_NAMES[mode]);
    glTranslated(pos[0], pos[1], pos[2]); 
    glMultMatrixd(rot_mat);
    glColor3f(1.0, 0.2, 0.2);

    v_get_world_from_head(0, &headxform);
    invscale = (float)headxform.scale / 26000;
    glScalef(invscale, invscale, invscale);

    make_cone();
  glEndList();
  return (collab_hand_struct); 
}

int make_red_line (nmg_State * state, const float a[], const float b[])
{  
  if ( !state->config_chartjunk ) 
    return 0;
   v_gl_set_context_to_vlib_window(); 
   glDeleteLists(red_line_struct,1);
   red_line_struct = glGenLists(1);
   glNewList(red_line_struct,GL_COMPILE);
   glColor3f(1.0,0.2,0.2);
   glLineWidth(2.0);
   make_line(a,b);
   glLineWidth(1.0);
   glEndList();
   return(red_line_struct);
} 

int make_green_line (nmg_State * state, const float a[], const float b[])
{  
  if ( !state->config_chartjunk ) 
    return 0;
   v_gl_set_context_to_vlib_window(); 
   glDeleteLists(green_line_struct,1);
   green_line_struct = glGenLists(1);
   glNewList(green_line_struct,GL_COMPILE);
   // No it's not green - it's yellow!
   // To avoid red-green colorblindness conflicts. 
   glColor3f(0.9,0.9,0.2);
   glLineWidth(2.0);
   make_line(a,b);
   glLineWidth(1.0);
   glEndList();
   return(green_line_struct);
} 

int make_blue_line (nmg_State * state, const float a[], const float b[])
{  
  if ( !state->config_chartjunk ) 
    return 0;
   v_gl_set_context_to_vlib_window(); 
   glDeleteLists(blue_line_struct,1);
   blue_line_struct = glGenLists(1);
   glNewList(blue_line_struct,GL_COMPILE);
   glColor3f(0.2,0.2,1.0);
   glLineWidth(2.0);
   make_line(a,b);
   glLineWidth(1.0);
   glEndList();
   return(blue_line_struct);
} 

//this will be where an axis centered on the position of the microscope tip
//will be created
int make_ds_sphere_axis (nmg_State *state, const q_type rot )
{

	//int scale = state->inputGrid->getPlaneByName(state->heightPlaneName)->maxX
	BCPlane *plane = state->inputGrid->getPlaneByName(state->heightPlaneName);
	int size = plane->maxX() - plane->minY();

	//factor to scale axis by
	int axis_scale = size * .05;

	//size of tick mark
	//the tick size will be double this number, in NM (reletive to everything else on the screen) i think
	int tick_size = axis_scale *.12;

	//these are the axes vectors when they are not rotated
	q_vec_type red_axis = {1,0,0};
    q_vec_type green_axis = {0,1,0};
    q_vec_type blue_axis  = {0,0,1};

	//end points of the axes
	q_vec_type red_axis_end, green_axis_end, blue_axis_end;

	q_vec_type axis_step, temp_vec, top_tick, bottom_tick, top_tick_start, bottom_tick_start;
	q_vec_type axis_end;
	
	int step_size_red = decoration->ds_red_ss;
	int step_size_green = decoration->ds_green_ss;
	int step_size_blue = decoration->ds_blue_ss;
	
	//position of the sphere
	q_vec_type sphere ={sphere_x, sphere_y, sphere_z};
	
	//rotate axis vectors
	q_xform(red_axis,(double*)rot,red_axis);
	q_xform(green_axis,(double*)rot,green_axis);
	q_xform(blue_axis,(double*)rot,blue_axis);

	//scale axis vectors to get the end points
	q_vec_scale(red_axis_end, axis_scale, red_axis);
	q_vec_scale(green_axis_end, axis_scale, green_axis);
	q_vec_scale(blue_axis_end, axis_scale, blue_axis);
	
	v_gl_set_context_to_vlib_window(); 
	glDeleteLists(ds_sphere_axis_struct,1);
	ds_sphere_axis_struct = glGenLists(1);
	glNewList(ds_sphere_axis_struct,GL_COMPILE);

	//positive lines, solid
	//red axis
	glColor3f(1.0,0.0,0.0);
	q_vec_add(axis_end, sphere, red_axis_end);
	make_line(sphere,axis_end);

	//green axis
	glColor3f(0.0,1.0,0.0);
	q_vec_add(axis_end, sphere, green_axis_end);
	make_line(sphere,axis_end);

	//blue axis
	glColor3f(0.0,0.0,1.0);
	q_vec_add(axis_end, sphere, blue_axis_end);
	make_line(sphere,axis_end);
	
	//negative lines, dotted
	glLineStipple(1, 0x00FF);
	glEnable(GL_LINE_STIPPLE);

	//red axis
	glColor3f(1.0,0.0,0.0);
	q_vec_subtract(axis_end, sphere, red_axis_end);
	make_line(sphere,axis_end);

	//green axis
	glColor3f(0.0,1.0,0.0);
	q_vec_subtract(axis_end, sphere, green_axis_end);
	make_line(sphere,axis_end);

	//blue axis
	glColor3f(0.0,0.0,1.0);
	q_vec_subtract(axis_end, sphere, blue_axis_end);
	make_line(sphere,axis_end);
	
	glDisable(GL_LINE_STIPPLE);


	//draw white tick marks, 2 along each axis, spaced a step-size apart
	int i;
	glColor3f(1.0, 1.0, 1.0);
	//red axis
	q_vec_scale(temp_vec, tick_size, blue_axis);
	q_vec_add(top_tick_start, sphere, temp_vec);
	q_vec_subtract(bottom_tick_start, sphere, temp_vec);
	
	for(i = -2; i <= 2; i++) {
		if(i != 0) {
			//when i = 0, we are drawing and the origin of the axis.
			//skip over.
			
			//distance along red axis to place tick
			q_vec_scale(axis_step, i * step_size_red, red_axis);
			
			q_vec_add(top_tick, top_tick_start, axis_step);
			q_vec_add(bottom_tick, bottom_tick_start, axis_step);
			
			make_line(top_tick, bottom_tick);
		}
	}
	
	//green axis
	//q_vec_scale(temp_vec, 50, blue_axis);
	q_vec_add(top_tick_start, sphere, temp_vec);
	q_vec_subtract(bottom_tick_start, sphere, temp_vec);
	
	for(i = -2; i <= 2; i++) {
		if(i != 0) {
			//distance along red axis to place tick
			q_vec_scale(axis_step, i * step_size_green, green_axis);
			
			q_vec_add(top_tick, top_tick_start, axis_step);
			q_vec_add(bottom_tick, bottom_tick_start, axis_step);
			
			make_line(top_tick, bottom_tick);
		}
	}
	//blue axis step tick marks, if we are in 3D mode
	q_vec_scale(temp_vec, tick_size, red_axis);
	q_vec_add(top_tick_start, sphere, temp_vec);
	q_vec_subtract(bottom_tick_start, sphere, temp_vec);
	
	for(i = -2; i <= 2; i++) {
		if(i != 0) {
			//distance along red axis to place tick
			q_vec_scale(axis_step, i * step_size_blue, blue_axis);
			
			q_vec_add(top_tick, top_tick_start, axis_step);
			q_vec_add(bottom_tick, bottom_tick_start, axis_step);
			
			make_line(top_tick, bottom_tick);
		}
	}
	
	
	glEndList();
	return(ds_sphere_axis_struct);
	
	return 0;
}




int make_sweep (nmg_State * /*state*/, const float a [], const float b [],
		const float c [], const float d [] )
{
  v_gl_set_context_to_vlib_window(); 
  glDeleteLists(sweep_struct,1);
  sweep_struct = glGenLists(1);
  glNewList(sweep_struct,GL_COMPILE);
  glColor3f(0.0,1.0,0.0);
  make_line(a,b);
  make_line(c,d);
  glEndList();
  return(sweep_struct);
}

int draw_list (void * data)
{
  GLint *listnum;
  listnum = (GLint *) data;
  glPushMatrix();
  glPushAttrib(GL_CURRENT_BIT);
  glCallList(*listnum);

  glPopAttrib();

  glPopMatrix();
  return(0);
}

static int myroom (int, void * data)
{
  nmg_State * state = (nmg_State *) data;
  nmg_Funclist *head;
  head = v_room;

  // Don't draw anything if chart junk is off
  if (!state->config_chartjunk) {
	return 0;
  }

  // Set material parameters for the space, then draw things in room space.
  // Since all in room space is text or lines, set measure materials.
  TIMERVERBOSE(5, mytimer, "globjects.c:myroom:set_gl_measure_materials");
  set_gl_measure_materials(state);
  TIMERVERBOSE(5, mytimer, "globjects.c:myroom:end set_gl_measure_materials");

  while(head != NULL) {
      if (spm_graphics_verbosity >= 12)
        fprintf(stderr,"            Drawing %s\n", head->name);
      head->function(head->data);
      head=head->next;
  }
  return(0);
}

static int myhead (int, void * data)
{
  nmg_State * state = (nmg_State *) data;
  nmg_Funclist *head;
  head = v_head;

  // Don't draw anything if chart junk is off
  if (!state->config_chartjunk) {
	return 0;
  }

  // Set material parameters for the space, then draw things in head space.
  // Since all in head space is text or lines, set measure materials.
  set_gl_measure_materials(state);
  while(head != NULL) {
      if (spm_graphics_verbosity >= 12)
        fprintf(stderr,"            Drawing %s\n", head->name);
      head->function(head->data);
      head=head->next;
  }
  return(0);
}

static int myhand (int, void * data)
{
  nmg_State * state = (nmg_State *) data;
  nmg_Funclist *head;
  head = v_hand;

  // Don't draw anything if chart junk is off
  if (!state->config_chartjunk) {
	return 0;
  }

  // Set material parameters for the space, which holds the icon 
  set_gl_icon_materials(state); 
  while(head != NULL) {
      if (spm_graphics_verbosity >= 12)
        fprintf(stderr,"            Drawing %s\n", head->name);
      head->function(head->data);
      head=head->next;
  }
  return(0);
}


static int myscreen (int, void * data)
{
  nmg_State * state = (nmg_State *) data;
  nmg_Funclist *head;
  GLint saveMatrixMode;
  head = v_screen;

  // Don't draw anything if chart junk is off
  if (!state->config_chartjunk) {
	return 0;
  }
  
  // Set material parameters for the space, then draw things in screen space.
  // Since all in screen space is text or lines, set measure materials.
  TIMERVERBOSE(5, mytimer, "myscreen;set_gl_measure_materials");
  set_gl_measure_materials(state);
  TIMERVERBOSE(5, mytimer, "myscreen; end set_gl_measure_materials");

  // Ugly kluge to put text on CRTs "in front of" the screen.
  // There isn't enough depth on the projection stack to push/pop
  // matrices, so instead we manually multiply and hope too much isn't
  // lost to floating point accuracy.
  // Correct values for the translation and scale were found by
  // iteration.
  if (g_CRT_correction) {
    glGetIntegerv(GL_MATRIX_MODE, &saveMatrixMode);
    if (saveMatrixMode != GL_PROJECTION) {
      glMatrixMode(GL_PROJECTION);
    }

    glTranslatef(-0.0225f, -0.018f, 0.0f);
    glScalef(0.075f, 0.075f, 0.2f);
  }

  while (head != NULL) {

    TIMERVERBOSE(5, mytimer, "myscreen:draw function");

    if (spm_graphics_verbosity >= 8) {
      fprintf(stderr,"            Drawing %s\n", head->name);
    }

    head->function(head->data);

    TIMERVERBOSE(5, mytimer, "myscreen:done draw function");

    head = head->next;
  }

  if (g_CRT_correction) {
    glScalef(13.333333f, 13.333333f, 5.0f);
    glTranslatef(0.0225f, 0.018f, 0.0f);
    if (saveMatrixMode != GL_PROJECTION) 
      glMatrixMode((GLenum) saveMatrixMode);
  }

  return(0);
}

int myworld (nmg_State * state)
{
  nmg_Funclist *head;
  head=vir_world;
  
  // Don't draw anything if chart junk is off
  if (!state->config_chartjunk) {
	return 0;
  }
    // Set the lighting model for the icons in the world, then draw it
    VERBOSE(4,"    Setting icon materials");
    set_gl_icon_materials(state);

  while(head != NULL)
    {
      if (spm_graphics_verbosity >= 12)
        fprintf(stderr,"            Drawing %s\n", head->name);
      head->function(head->data);
      head=head->next;
    }
  return(0);
}


/*various gl functions to create objects*/
int grabhand(void *data)
{
  nmg_State * state = (nmg_State *) data;
  float size = 0.02;

  glPushMatrix();
  glPushAttrib(GL_CURRENT_BIT);

  glColor3f(0.0,1.0,0.0); 
  glScalef(size * state->icon_scale, size * state->icon_scale, size * state->icon_scale);
  mycube();

  glPopAttrib();

  glPopMatrix();
  return(0);
}


int lighthand(void *data)
{
  nmg_State * state = (nmg_State *) data;
    float x_wide = state->inputGrid->maxX() - state->inputGrid->minX();
    float y_wide = state->inputGrid->maxY() - state->inputGrid->minY();
    float z_value;

    float size = 0.15*x_wide;
    v_xform_type	worldFromHand;
    q_matrix_type hand_mat;
    q_type q_world_from_hand;

    glPushMatrix();

    BCPlane *height = state->inputGrid->getPlaneByName
        (state->heightPlaneName);
  
    if (height) {
        z_value = height->scale() * 0.5 * (height->maxNonZeroValue() + 
                                           height->minNonZeroValue());
    } else {
        z_value = 0.0f;
    }
    
    // Approximate center of the surface. 
    glTranslatef( state->inputGrid->minX() + x_wide/2.0,
                  state->inputGrid->minY() + y_wide/2.0,
                  z_value);  

  // Make it correspond to hand rotation
  v_get_world_from_hand(0, &worldFromHand);
  q_copy(q_world_from_hand, worldFromHand.rotate);

  // Apply hand rotation directly to the light icon. 
  q_to_row_matrix(hand_mat, q_world_from_hand);
  glMultMatrixd((double *)hand_mat);

  // back away from the surface so icon "orbits" the center of the surface.
  glTranslatef(0.0, x_wide/2.0, 0.0);

  // rotate so light ray sticks out in -y direction in hand space
  glRotatef(-90, 1.0, 0.0, 0.0);

  glScalef(size * state->icon_scale, size * state->icon_scale, size * state->icon_scale);

  glPushAttrib(GL_CURRENT_BIT);

  glColor3f(0.5,0.0,0.9); 

  // The base of the light (cube, with -z open)
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(0.0,0.0,1.0);
      glVertex3f(-0.5,0.5,1.0);
      glVertex3f(-0.5,-0.5,1.0);
      glVertex3f(0.5,-0.5,1.0);
      glVertex3f(0.5,0.5,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(0.0,-1.0,0.0);
      glVertex3f(0.5,-0.5,1.0);
      glVertex3f(-0.5,-0.5,1.0);
      glVertex3f(-0.5,-0.5,0.0); 
      glVertex3f(0.5,-0.5,0.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(0.0,1.0,0.0);
      glVertex3f(0.5,0.5,1.0);
      glVertex3f(0.5,0.5,0.0);
      glVertex3f(-0.5,0.5,0.0);
      glVertex3f(-0.5,0.5,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(-1.0,0.0,0.0);
      glVertex3f(-0.5,0.5,1.0);
      glVertex3f(-0.5,0.5,0.0);
      glVertex3f(-0.5,-0.5,0.0);
      glVertex3f(-0.5,-0.5,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(1.0,0.0,0.0);
      glVertex3f(0.5,-0.5,1.0);
      glVertex3f(0.5,-0.5,0.0);
      glVertex3f(0.5,0.5,0.0);
      glVertex3f(0.5,0.5,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();

  // The shade of the light
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(0.0,-1.0,0.0);
      glVertex3f(0.5,-0.5,0.0);
      glVertex3f(-0.5,-0.5,0.0);
      glVertex3f(-1.0,-1.0,-1.0); 
      glVertex3f(1.0,-1.0,-1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(0.0,1.0,0.0);
      glVertex3f(0.5,0.5,0.0);
      glVertex3f(1.0,1.0,-1.0);
      glVertex3f(-1.0,1.0,-1.0);
      glVertex3f(-0.5,0.5,0.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(-1.0,0.0,0.0);
      glVertex3f(-0.5,0.5,0.0);
      glVertex3f(-1.0,1.0,-1.0);
      glVertex3f(-1.0,-1.0,-1.0);
      glVertex3f(-0.5,-0.5,0.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(1.0,0.0,0.0);
      glVertex3f(0.5,-0.5,0.0);
      glVertex3f(1.0,-1.0,-1.0);
      glVertex3f(1.0,1.0,-1.0);
      glVertex3f(0.5,0.5,0.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  // inside 
  // yellow square so inside of light looks lit. 
  glColor3f(1.0,1.0,0.0);
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(0.0,0.0,1.0);
      glVertex3f(-0.45,0.45,0.9);
      glVertex3f(-0.45,-0.45,0.9);
      glVertex3f(0.45,-0.45,0.9);
      glVertex3f(0.45,0.45,0.9);
      VERBOSE(20, "          glEnd()");
  glEnd();

  // Light ray from center.
  glBegin(GL_LINES);
    VERBOSE(20, "          glBegin(GL_LINES)");
    glColor3f(1.0,1.0,0.0);
    glVertex3f(0.0,0.0,-7.0);
    glVertex3f(0.0,0.0,0.0);
    glVertex3f(0.8,0.8,-5.0);
    glVertex3f(0.8,0.8,-0.8);
    glVertex3f(-0.8,0.8,-5.0);
    glVertex3f(-0.8,0.8,-0.8);
    glVertex3f(-0.8,-0.8,-5.0);
    glVertex3f(-0.8,-0.8,-0.8);
    glVertex3f(0.8,-0.8,-5.0);
    glVertex3f(0.8,-0.8,-0.8);
    VERBOSE(20, "          glEnd()");
  glEnd();
  
  glPopAttrib();
  glPopMatrix();
  return(0);
}


int selecthand(void *)
{
  glPushMatrix();

  glPushAttrib(GL_CURRENT_BIT);
  glRotated(90.0, 0.0, 1.0, 0.0);	//Yes, this is necessary.

  glColor3f(0.0,0.0,1.0); 
  glCallList(vx_half_down);

  glPopAttrib();
  glPopMatrix();
  return(0);
}

int mycube(void)
{  
  GLfloat matspec[4] = { 0.5, 0.5, 0.5, 0.0 };
  glPushMatrix();
  glMaterialfv(GL_FRONT, GL_SPECULAR, matspec);
  glMaterialf(GL_FRONT, GL_SHININESS, 64.0);
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glColor3f(0.0,0.0,1.0);
      glNormal3f(0.0,0.0,-1.0);
      glVertex3f(1.0,1.0,-1.0);
      glVertex3f(1.0,-1.0,-1.0);
      glVertex3f(-1.0,-1.0,-1.0);
      glVertex3f(-1.0,1.0,-1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glColor3f(0.0,1.0,0.0);
      glNormal3f(0.0,0.0,1.0);
      glVertex3f(-1.0,1.0,1.0);
      glVertex3f(-1.0,-1.0,1.0);
      glVertex3f(1.0,-1.0,1.0);
      glVertex3f(1.0,1.0,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glColor3f(1.0,0.0,0.0);
      glNormal3f(0.0,-1.0,0.0);
      glVertex3f(1.0,-1.0,1.0);
      glVertex3f(-1.0,-1.0,1.0);
      glVertex3f(-1.0,-1.0,-1.0); 
      glVertex3f(1.0,-1.0,-1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glColor3f(0.0,1.0,1.0);
      glNormal3f(0.0,1.0,0.0);
      glVertex3f(1.0,1.0,1.0);
      glVertex3f(1.0,1.0,-1.0);
      glVertex3f(-1.0,1.0,-1.0);
      glVertex3f(-1.0,1.0,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glColor3f(1.0,1.0,0.0);
      glNormal3f(-1.0,0.0,0.0);
      glVertex3f(-1.0,1.0,1.0);
      glVertex3f(-1.0,1.0,-1.0);
      glVertex3f(-1.0,-1.0,-1.0);
      glVertex3f(-1.0,-1.0,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glColor3f(1.0,0.0,1.0);
      glNormal3f(1.0,0.0,0.0);
      glVertex3f(1.0,-1.0,1.0);
      glVertex3f(1.0,-1.0,-1.0);
      glVertex3f(1.0,1.0,-1.0);
      glVertex3f(1.0,1.0,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glPopMatrix();
  return(0);
}

int big_flat_arrow(nmg_State * state)
{

  // Don't draw anything if chart junk is off
  if (!state->config_chartjunk) {
	return 0;
  }

  /* using a triangle and a square to draw the arrow */
  glBegin(GL_TRIANGLES);
    VERBOSE(20, "          glBegin(GL_TRIANGLES)");
    glNormal3f(0.0,0.0,1.0);
    glVertex3f(0.5, 0.3, 0.0);
    glVertex3f(0.0, 0.8, 0.0);
    glVertex3f(-0.5, 0.3, 0.0);
    VERBOSE(20, "          glEnd()");
  glEnd();

  glBegin(GL_QUADS);
    VERBOSE(20, "          glBegin(GL_QUADS)");
    glNormal3f(0.0,0.0,1.0);
    glVertex3f(-0.3, 0.3, 0.0);
    glVertex3f(-0.3, 0.0, 0.0);
    glVertex3f(0.3, 0.0, 0.0);
    glVertex3f(0.3, 0.3, 0.0);
    VERBOSE(20, "          glEnd()");
  glEnd();

  return(0);
}


void subdivide(float *v1, float *v2, float *v3, int depth)
{
  GLfloat v12[3], v23[3], v31[3];
  int i;
  float d1,d2,d3;

  if (depth==0) 
    {
      glBegin(GL_POLYGON);
        VERBOSE(20, "          glBegin(GL_POLYGON)");
         glNormal3fv(v1);
         glVertex3fv(v1);
         glNormal3fv(v2);
         glVertex3fv(v2);
         glNormal3fv(v3);
         glVertex3fv(v3);
        VERBOSE(20, "          glEnd()");
      glEnd();
      return;
    }

  for (i=0; i<3; i++)
    {
      v12[i] = v1[i] + v2[i];
      v23[i] = v2[i] + v3[i];
      v31[i] = v3[i] + v1[i];
    }

  d1 = sqrt(v12[0]*v12[0]+v12[1]*v12[1]+v12[2]*v12[2]);
  d2 = sqrt(v23[0]*v23[0]+v23[1]*v23[1]+v23[2]*v23[2]);
  d3 = sqrt(v31[0]*v31[0]+v31[1]*v31[1]+v31[2]*v31[2]);

  if (d1 !=0 && d2 != 0 && d3 != 0)
    {
      for (i=0; i<3; i++)
	{
	  v12[i] /= d1;
	  v23[i] /= d2;
	  v31[i] /= d3;
	}
    }
  else
    {
      printf("division by zero while creating sphere icon\n");
    }

  subdivide(v1,v12,v31, depth-1);
  subdivide(v2,v23,v12, depth-1);
  subdivide(v3,v31,v23, depth-1);
  subdivide(v12,v23,v31, depth-1);

}

void init_sphere(void)
{
  int i;

  static VertexType vdata[] = {
    {-xx,0,zz},   {xx,0,zz}, {-xx,0,-zz}, {xx,0,-zz},   
    {0,zz,xx},   {0,zz,-xx}, {0,-zz,xx}, {0,-zz,-xx},   
    {zz,xx,0}, {-zz,xx,0},  {zz,-xx,0}, {-zz,-xx,0} };

  static GLint trindex[20][3] = {
    {0,4,1},   {0,9,4},   {9,5,4},  {4,5,8},  {4,8,1},
    {8,10,1}, {8,3,10},   {5,3,8},  {5,2,3},  {2,7,3},
    {7,10,3}, {7,6,10},  {7,11,6}, {11,0,6},  {0,1,6},
    {6,1,10}, {9,0,11}, {9,11,2},  {9,2,5}, {7,2,11} };

  v_gl_set_context_to_vlib_window(); 
  glNewList(sphere,GL_COMPILE);
  for (i=0;i<20;i++)
    {
      //needed to make the vertices get traversed the other way, otherwise we
      //get a backfacing sphere
      subdivide(vdata[trindex[i][2]],vdata[trindex[i][1]],vdata[trindex[i][0]],SPHERE_DEPTH);
    }
  glEndList();

}


void position_sphere(nmg_State * state, float x,float y, float z)
{
  sphere_x=x;
  sphere_y=y;
  // position is at bottom of sphere, r=state->sphere_scale, maybe
  sphere_z=z+state->sphere_scale;	
}

int mysphere(void * data )
{
  nmg_State * state = (nmg_State *) data;

       	glPushMatrix();
	glPushAttrib(GL_CURRENT_BIT);
	glColor3f(1.0,0.0,0.0);
        glTranslatef(sphere_x,sphere_y,sphere_z);
	//the scaling is obtained from a tcl slider
	glScalef(state->sphere_scale, state->sphere_scale, state->sphere_scale);
        glCallList(sphere);
	glPopAttrib();
	glPopMatrix();

	return(0);
}


// Position of user's hand
int Tip(void * data)
{
  nmg_State * state = (nmg_State *) data;
        int i;
        static float handlescale = 0.1f;

	glPushMatrix();

	/* I don't know who keeps taking this out but unless we change
	how the tip is drawn it seems to be necessary */
	glRotatef(-90.0, 0.0,1.0,0.0); 

	glPushAttrib(GL_CURRENT_BIT);

	glColor3f(0.0,0.0,1.0);
	glScalef(handlescale * state->icon_scale,
                 handlescale * state->icon_scale,
                 handlescale * state->icon_scale);

	/* top */
	glBegin(GL_TRIANGLE_FAN);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_FAN)");
	glVertex3f(TIP_HEIGHT,0.0,0.0);
	for (i=0;i<17;i++)
	  glVertex3f(TIP_HEIGHT,0.1*cos(i*M_PI/8.0),0.1*sin(i*M_PI/8.0));
        VERBOSE(20, "          glEnd()");
	glEnd();
	
        /* side */
	glBegin(GL_TRIANGLE_FAN);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_FAN)");
	glVertex3f(0.0,0.0,0.0);
	for (i=0;i<17;i++)
	  glVertex3f(TIP_HEIGHT,0.1*cos(i*M_PI/8.0),0.1*sin(i*M_PI/8.0));
        VERBOSE(20, "          glEnd()");
	glEnd();

	glPopAttrib();
	glPopMatrix();

	return(0);
}	/* Tip */


// Last known position of microscope's actual tip
// Needs to be differentiated from Tip() for latency compensation

// static
int TrueTip (void *data)
{
  nmg_State * state = (nmg_State *) data;
        int i;

        static const float handlescale = 125.0f;
        //static const float handlescale = 50000.0f;
          // World space has a VERY different scale than hand space,
          // so we need to have 50000x the scale when we draw the
          // tip.
          // Value changed by a factor of 400 sometime before June 1999

	glPushMatrix();

        // Translate to the right point
        glTranslatef(state->trueTipLocation[0],
                     state->trueTipLocation[1],
                     state->trueTipLocation[2]);

        if (spm_graphics_verbosity >= 4)
          fprintf(stderr, "Drawing true tip at (%.2f %.2f %.2f) "
                          "with scale %.2f (net scale %.2f).\n",
		  state->trueTipLocation[0], state->trueTipLocation[1], 
		  state->trueTipLocation[2], state->trueTipScale, 
		  state->trueTipScale * handlescale * state->icon_scale);

	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

	glPushAttrib(GL_CURRENT_BIT);

	glColor3f(0.8f, 0.0f, 0.4f);
	glScalef(state->trueTipScale * handlescale * state->icon_scale,
                 state->trueTipScale * handlescale * state->icon_scale,
                 state->trueTipScale * handlescale * state->icon_scale);

	// top
	glBegin(GL_TRIANGLE_FAN);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_FAN)");
	glVertex3f(TIP_HEIGHT, 0.0f, 0.0f);
	for (i = 0; i < 17; i++)
	  glVertex3f(TIP_HEIGHT, 0.1f * cos(i * M_PI / 8.0f),
                                 0.1f * sin(i * M_PI / 8.0f));
        VERBOSE(20, "          glEnd()");
	glEnd();
	
        // side
	glBegin(GL_TRIANGLE_FAN);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_FAN)");
	glVertex3f(0.0f, 0.0f, 0.0f);
	for (i = 0; i < 17; i++)
	  glVertex3f(TIP_HEIGHT, 0.1f * cos(i * M_PI / 8.0f),
                                 0.1f * sin(i * M_PI / 8.0f));
        VERBOSE(20, "          glEnd()");
	glEnd();

	glPopAttrib();
	glPopMatrix();

	return 0;
}	/* Tip */


//JM from TCH branch

// Draws wireframe triangles to show the grid being returned by
// feel-ahead haptics.

int FeelGrid (void *data) {
	nmg_State * state = (nmg_State *) data;
   int i, j;
   double * vp;  // UGLY HACK!




   glPushMatrix();
   glPushAttrib(GL_CURRENT_BIT);
   glColor3f(1.0f, 1.0f, 1.0f);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   for (i = 0; i < state->fg_xside - 1; i++) {
      glBegin(GL_TRIANGLE_STRIP);

      for (j = 0; j < state->fg_yside; j++) {
      /*
      vp = state->fg_vertices[i * state->fg_xside + j];
      glVertex3d(vp[0], vp[1], vp[2]);
      vp = state->fg_vertices[(i + 1) * state->fg_xside + j];
      glVertex3d(vp[0], vp[1], vp[2]);
          */
          
          vp = &state->fg_vertices[i] [j];
          glVertex3d(vp[0], vp[1], vp[2]);

          vp = &state->fg_vertices[i+1] [j];
          glVertex3d(vp[0], vp[1], vp[2]);

      }
      glEnd();
   }

   //HACK
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   //glPopAttrib();
   //glPopMatrix();


   return 0;
   /* FeelGrid */
}

//JM from TCH branch

int make_feelGrid (void *data) {
   int i, j;
   double * vp;  // UGLY HACK!

   //glPushMatrix();
   //glPushAttrib(GL_CURRENT_BIT);

   v_gl_set_context_to_vlib_window();
   glDeleteLists(feelGrid_struct,1);
   feelGrid_struct = glGenLists(1);
   glNewList(feelGrid_struct,GL_COMPILE);

   glColor3f(1.0f, 1.0f, 1.0f);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   for (i = 0; i < fg_xside - 1; i++) {
      glBegin(GL_TRIANGLE_STRIP);

      for (j = 0; j < fg_yside; j++) {
        vp = fg_vertices[i * fg_xside + j];
        glVertex3d(vp[0], vp[1], (vp[2]+40) );
        vp = fg_vertices[(i + 1) * fg_xside + j];
        glVertex3d(vp[0], vp[1], (vp[2] + 40) );
      }
      glEnd();
   }

   //glPopAttrib();
   //glPopMatrix();
   
   glEndList();

   return feelGrid_struct;
   /* FeelGrid */
}

int make_feelPlane (void *data) {

	nmg_State * state = (nmg_State *) data;

    //JM
    nmg_haptic_graphics * haptic_graphics = (nmg_haptic_graphics *) data;
	q_vec_type fp_origin, fp_normal;

   //int i, j;
   q_vec_type parallel;
   q_vec_type span1, span2;
   q_vec_type mspan1, mspan2;
   double vp [4][3];
   double scalescale = 3.0;

   //glPushMatrix();
   //glPushAttrib(GL_CURRENT_BIT);

   haptic_graphics->update_origin(&fp_origin, &fp_normal);

  // find vectors describing the sides
   
   v_gl_set_context_to_vlib_window();
   glDeleteLists(feelPlane_struct,1);
   feelPlane_struct = glGenLists(1);
   glNewList(feelPlane_struct,GL_COMPILE);

   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   
   glColor3f(1.0f, 1.0f, 1.0f);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   
   glTranslatef(fp_origin_j[0], fp_origin_j[1], fp_origin_j[2]);

  q_vec_set(parallel, 1.0, 0.0, 0.0);
  q_vec_cross_product(span1, fp_normal_j, parallel);
  q_vec_cross_product(span2, fp_normal_j, span1);

  q_vec_normalize(span1, span1);
  q_vec_normalize(span2, span2);
  q_vec_scale(mspan1, -1.0, span1);
  q_vec_scale(mspan2, -1.0, span2);

  // set up vertices

  q_vec_add(vp[0], span1, span2);
  q_vec_add(vp[1], mspan1, span2);
  q_vec_add(vp[2], mspan1, mspan2);
  q_vec_add(vp[3], span1, mspan2);

  q_vec_scale(vp[0],50,vp[0]);
  q_vec_scale(vp[1],50,vp[1]);
  q_vec_scale(vp[2],50,vp[2]);
  q_vec_scale(vp[3],50,vp[3]);

  glColor3f(1.0f, 1.0f, 0.0f);

  glBegin(GL_TRIANGLE_STRIP);
  glVertex3d(vp[0][0], vp[0][1], vp[0][2]);
  glVertex3d(vp[1][0], vp[1][1], vp[1][2]);
  glVertex3d(vp[3][0], vp[3][1], vp[3][2]);
  glVertex3d(vp[2][0], vp[2][1], vp[2][2]);
  glEnd();

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   //glPopAttrib();
   //glPopMatrix();

   glEndList();

   return (feelPlane_struct);
   /* FeelPlane */
}



#define RED   1
#define GREEN 2
#define BLUE  3
int measure_hand(void *data)
{
  nmg_State * state = (nmg_State *) data;
  float handlescale = 0.012f;
  int color;
  GLfloat matspec[4] = { 0.5, 0.5, 0.5, 0.0 };
  color = state->hand_color;

  glPushMatrix();
  glPushAttrib(GL_CURRENT_BIT);
 
  if( color==RED) {
    glColor3f(1.0,0.2,0.2);   //red
  } else if(color==GREEN) {
   // No it's not green - it's yellow!
   // To avoid red-green colorblindness conflicts. 
   glColor3f(0.9,0.9,0.2);
  } else {
     glColor3f(0.2,0.2,1.0);   //blue
  }
  glScalef(handlescale * state->icon_scale,
           handlescale * state->icon_scale,
           handlescale * state->icon_scale);

  glMaterialfv(GL_FRONT, GL_SPECULAR, matspec);
  glMaterialf(GL_FRONT, GL_SHININESS, 64.0);
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(0.0,0.0,-1.0);
      glVertex3f(1.0,1.0,-1.0);
      glVertex3f(1.0,-1.0,-1.0);
      glVertex3f(-1.0,-1.0,-1.0);
      glVertex3f(-1.0,1.0,-1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(0.0,0.0,1.0);
      glVertex3f(-1.0,1.0,1.0);
      glVertex3f(-1.0,-1.0,1.0);
      glVertex3f(1.0,-1.0,1.0);
      glVertex3f(1.0,1.0,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(0.0,-1.0,0.0);
      glVertex3f(1.0,-1.0,1.0);
      glVertex3f(-1.0,-1.0,1.0);
      glVertex3f(-1.0,-1.0,-1.0);
      glVertex3f(1.0,-1.0,-1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(0.0,1.0,0.0);
      glVertex3f(1.0,1.0,1.0);
      glVertex3f(1.0,1.0,-1.0);
      glVertex3f(-1.0,1.0,-1.0);
      glVertex3f(-1.0,1.0,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(-1.0,0.0,0.0);
      glVertex3f(-1.0,1.0,1.0);
      glVertex3f(-1.0,1.0,-1.0);
      glVertex3f(-1.0,-1.0,-1.0);
      glVertex3f(-1.0,-1.0,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glBegin(GL_POLYGON);
      VERBOSE(20, "          glBegin(GL_POLYGON)");
      glNormal3f(1.0,0.0,0.0);
      glVertex3f(1.0,-1.0,1.0);
      glVertex3f(1.0,-1.0,-1.0);
      glVertex3f(1.0,1.0,-1.0);
      glVertex3f(1.0,1.0,1.0);
      VERBOSE(20, "          glEnd()");
  glEnd();
  glPopAttrib();
  glPopMatrix();
  return(0);
}
#undef RED
#undef GREEN
#undef BLUE


int vx_down_icon(void *)
{
  glPushAttrib(GL_CURRENT_BIT);
  glPushMatrix();
  glColor3f(1.0,0.0,0.0);
    glCallList(vx_half_down);
    glRotatef(180.0, 0.0,0.0,1.0);
    glCallList(vx_half_down);
  glPopMatrix();
  glPopAttrib();

  return(0);
}	/* vx_down_icon */



int vx_up_icon(void *)
{
  glPushAttrib(GL_CURRENT_BIT);
  glPushMatrix();
  glColor3f(0.0,1.0,0.0);
    glCallList(vx_half_up);
    glRotatef(180.0, 0.0,0.0,1.0);
    glCallList(vx_half_up);
  glPopMatrix();
  glPopAttrib();

  return(0);
}	/* vx_up_icon */


int draw_north_pointing_arrow (void * data)
{
  nmg_State * state = (nmg_State *) data;
	float x_wide = state->inputGrid->maxX() - state->inputGrid->minX();
	float y_wide = state->inputGrid->maxY() - state->inputGrid->minY();
	float z_value;

	float scale  = 0.25f * x_wide;

	BCPlane *height = state->inputGrid->getPlaneByName
                    (state->heightPlaneName);

	if (height) {
		z_value = height->scaledValue(state->inputGrid->numX() / 2,
                                              state->inputGrid->numY() - 1);
	} else {
		z_value = 0.0f;
	}
		
	glPushAttrib(GL_CURRENT_BIT);
	glPushMatrix();
	glColor3f(1.0,0.0,0.0);
	glTranslatef( state->inputGrid->minX() + x_wide/2.0,
		      state->inputGrid->minY() + 1.02 * y_wide,
		      z_value);  
	glScalef(scale, scale, scale);
	big_flat_arrow(state); 
	glPopMatrix();
	glPopAttrib();
	return(0);
}	/* draw_north_pointing_arrow */

/* markers for endpoints in a polyline. */
int my_line_mark(void *data)
{
    float *a = (float *)(data);  // extract endpoints of line
    float *b = (&(a[3]));

    glPushAttrib(GL_CURRENT_BIT);
    glColor3f(0.0,0.9,0.9);
    glLineWidth(2.0);
    make_line(a,b);
    glLineWidth(1.0);
    glPopAttrib();
    
    return(0);
}

#define NUM_SCANLINE_INDICATOR_POINTS 50
int my_scanline_indicator(void *data) {
    nmg_State * state = (nmg_State *) data;
    // data contains coords of two points to draw
    // the lines between: x1, y1,z1, x2, y2, z2
    float * temp = (float *)state->scanlinePt;
    VertexType a[NUM_SCANLINE_INDICATOR_POINTS];
    VertexType b[NUM_SCANLINE_INDICATOR_POINTS];
    VertexType p0, p1;

    p0[0] = temp[0];
    p0[1] = temp[1];
    p0[2] = temp[2];
    p1[0] = temp[3];
    p1[1] = temp[4];
    p1[2] = temp[5];
    int i;
    BCPlane* plane = state->inputGrid->getPlaneByName(state->heightPlaneName);
    if (plane == NULL) {
        fprintf(stderr, "Error in my_scanline_indicator:can't get plane!\n");
        return -1;
    }
   
    float p_scale = plane->scale();
    float r0, r1;

    for (i = 0; i < NUM_SCANLINE_INDICATOR_POINTS; i++) {
	r0 = (float)(i)/(float)(NUM_SCANLINE_INDICATOR_POINTS-1);
	r1 = 1.0 - r0;
       	a[i][0] = p0[0]*r0 + p1[0]*r1;
	a[i][1] = p0[1]*r0 + p1[1]*r1;
        a[i][2] = plane->interpolatedValueAt(a[i][0], a[i][1])*p_scale;
	b[i][0] = a[i][0];
	b[i][1] = a[i][1];
	b[i][2] = p0[2]*r0 + p1[2]*r1;
    }

    glPushAttrib(GL_CURRENT_BIT);
    glColor3f(0.0,0.9,0.9);
    glLineWidth(2.0);
    // XXX - hack to get line to look nice since it doesn't exactly
    // trace the surface (sometimes goes below the surface - the right way to
    // do it would be to exactly follow the surface by finding intersections
    // of a cutting plane with surface edges and drawing lines between those
    // points instead of the evenly-spaced points chosen here

    // On the other hand, maybe this is what we want - this lets you see
    // a sort of graph of where the scan line will go relative to the
    // surface so you don't want the surface to occlude parts of the graph
    // because then you might miss something - although it would be nice to
    // dim the occluded parts or something
    make_line(b[0], b[NUM_SCANLINE_INDICATOR_POINTS-1]);
    for (i = 0; i < NUM_SCANLINE_INDICATOR_POINTS-1; i++){
    	make_line(a[i],a[i+1]);
	make_line(a[i], b[i]);
    }
    make_line(a[NUM_SCANLINE_INDICATOR_POINTS-1],
		b[NUM_SCANLINE_INDICATOR_POINTS-1]);
    glPopAttrib();
    return(0);
}

/* draws 7 lines, parallel to the surface, from the most
 * recent point in a polyline to the user's hand. 
 */
int my_rubber_line(void * data)
{
    // data contains coords of two points to draw
    // the lines between: x1, y1, x2, y2
    float * temp = (float *)data;
    VertexType a,b;
    a[0] = temp[0];
    a[1] = temp[1];
    a[2] = temp[2];
    b[0] = temp[3];
    b[1] = temp[4];
    b[2] = temp[5];

    double z_min,z_max;
    z_max = max (a[2], b[2]);
    z_min = min (a[2], b[2]);

    // Make limits 10% bigger (+ const) so lines are visible above min and max. 
    double z_range = z_max - z_min;
    z_min = (z_min-(z_range * 0.1 + 50));
    z_max = (z_max+(z_range * 0.1 + 50));
    z_range = z_max - z_min;

    // Now we draw some lines - 7 horizontal lines.
    glPushAttrib(GL_CURRENT_BIT);
    glColor3f(0.0,0.9,0.9);

    a[2]=z_min;
    b[2]=z_min;
    make_line(a,b);
    a[2]=z_min + 0.1667*z_range;
    b[2]=z_min + 0.1667*z_range;
    make_line(a,b);
    a[2]=z_min + 0.3333*z_range;
    b[2]=z_min + 0.3333*z_range;
    make_line(a,b);
    a[2]=z_min + 0.5*z_range;
    b[2]=z_min + 0.5*z_range;
    make_line(a,b);
    a[2]=z_min + 0.6667*z_range;
    b[2]=z_min + 0.6667*z_range;
    make_line(a,b);
    a[2]=z_min + 0.8333*z_range;
    b[2]=z_min + 0.8333*z_range;
    make_line(a,b);
    a[2]=z_max;
    b[2]=z_max;
    make_line(a,b);

    glPopAttrib();
    return(0);

}


int make_rubber_line_point ( nmg_State * state, const PointType point[2], Position_list * p) {
  int list_id;

  list_id = addFunctionToFunclist(&vir_world, my_line_mark, (void *) point,
                                  "my_line_mark");

  //   If this is the first point in the polyline,
  // initialize the rubber-band line.
  if (p->empty()) {

    poly_rubber_line_id =
      addFunctionToFunclist( &vir_world, my_rubber_line,
			     (void *) state->rubberPt,
			     "my_rubber_line" );
  }
  // TODO:
  //   move into server, or onto graphics state!
  //   currently duplicated in interaction and here
  // Save this point as part of the poly-line
  // printf("doline - add func: %d\n", list_id);
  // DO NOT insert points
  p->insert(point[0][0], point[0][1], list_id);

  return list_id;
}

static int draw_rubber_line[2] = {0,0};
static int draw_rubber_corner_line[2] = {0,0};
int make_rubber_line_point ( nmg_State * state, 
    Position_list * p, int index)
{
    //static float rep [4];
    int list_id;

    if ( draw_rubber_line[index] ) { 
        // XXX memory leak
	float *rubber_point = new float[6];
	rubber_point[0] = state->rubberSweepPts[index][0];
	rubber_point[1] = state->rubberSweepPts[index][1];
	rubber_point[2] = state->rubberSweepPts[index][2];
	rubber_point[3] = state->rubberSweepPts[index][3];
	rubber_point[4] = state->rubberSweepPts[index][4];
	rubber_point[5] = state->rubberSweepPts[index][5];
	
	list_id = addFunctionToFunclist(&vir_world, my_rubber_line,
					(void *) rubber_point,
					"my_rubber_line");
	p->insert( rubber_point[0], rubber_point[1], list_id);

	if ( draw_rubber_corner_line[index] ) {
            // XXX memory leak
	    float *rubber_corner_point = new float[6];
	    rubber_corner_point[0] = state->rubberSweepPtsSave[index][0];
	    rubber_corner_point[1] = state->rubberSweepPtsSave[index][1];
	    rubber_corner_point[2] = state->rubberSweepPtsSave[index][2];
	    rubber_corner_point[3] = state->rubberSweepPtsSave[index][3];
	    rubber_corner_point[4] = state->rubberSweepPtsSave[index][4];
	    rubber_corner_point[5] = state->rubberSweepPtsSave[index][5];
	    
	    list_id = addFunctionToFunclist(&vir_world, my_rubber_line,
					    (void *) rubber_corner_point,
					    "my_rubber_line");
	    p->insert( rubber_point[0], rubber_point[1], list_id);

	    state->rubberSweepPtsSave[index][0] = state->rubberSweepPts[index][3];
	    state->rubberSweepPtsSave[index][1] = state->rubberSweepPts[index][4];
	    state->rubberSweepPtsSave[index][2] = state->rubberSweepPts[index][5];
	}
	else {
	    state->rubberSweepPtsSave[index][0] = state->rubberSweepPts[index][3];
	    state->rubberSweepPtsSave[index][1] = state->rubberSweepPts[index][4];
	    state->rubberSweepPtsSave[index][2] = state->rubberSweepPts[index][5];

	    poly_sweep_rubber_line_id[index + 2] =
		addFunctionToFunclist(&vir_world, my_rubber_line,
				      (void *) state->rubberSweepPtsSave[index],
				      "my_rubber_line");
	    draw_rubber_corner_line[index] = 1;
	}
     }
    else if ( p->empty() ) { 
	poly_sweep_rubber_line_id[index] =
	    addFunctionToFunclist(&vir_world, my_rubber_line,
				  (void *) state->rubberSweepPts[index],
				  "my_rubber_line");
	draw_rubber_line[index] = 1;
    }
    return list_id;
}

void empty_rubber_line (Position_list * p) {

   p->start();
   while( p->notDone()) {
      removeFunctionFromFunclist(&vir_world, (p->curr())->iconID());
      // Delete the current position, and advance to the next one in the list. 
      p->del();
  }
  // get rid of the rubber-band line
   removeFunctionFromFunclist(&vir_world, poly_rubber_line_id);
}

void empty_rubber_line (Position_list * p, int index) {
  p->start();
   while( p->notDone()) {
      removeFunctionFromFunclist(&vir_world, (p->curr())->iconID());

      // Delete the current position, and advance to the next one in the list. 
      p->del();
  }
  // get rid of the rubber-band line
   removeFunctionFromFunclist(&vir_world, poly_sweep_rubber_line_id[index]);
   removeFunctionFromFunclist(&vir_world, poly_sweep_rubber_line_id[index + 2]);
   draw_rubber_line[index] = 0;
   draw_rubber_corner_line[index] = 0;
}


// WARNING
// This may be inefficient.
// If it is too slow, write spm_draw_scrapes() and spm_draw_pulses()
// that do the glLineWidth/glDisable/glBegin/glEnd and just send the
// vertices in spm_render_mark
int spm_render_mark (const nmb_LocationInfo & p, void *) {
    GLfloat Bottom [3], Top [3];
    GLfloat LowerThanBottom[3];
    
    Bottom[0] = Top[0] = p.x;
    Bottom[1] = Top[1] = p.y;
    Bottom[2] = p.bottom;
    Top[2] = p.top;
    LowerThanBottom[0] = p.x; LowerThanBottom[1] = p.y;
    LowerThanBottom[2] = Bottom[2] - (Top[2] - Bottom[2]);
    
    
    // Partially transparent to make it easier to see surface. 
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

    glLineWidth(1.0);
    glDisable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    VERBOSE(20, "          glBegin(GL_TRIANGLE_STRIP)");
    glVertex3fv(Bottom);
    glVertex3fv(Top);
    VERBOSE(20, "          glEnd()");
    
    // Partially transparent to make it easier to see surface. 
    glColor4f(1.0f, 0.0f, 0.5f, 0.5f);

    //glBegin(GL_LINES);
    glVertex3fv(Bottom);
    glVertex3fv(LowerThanBottom);
    glEnd();
    
    return 0;
}


int replaceDefaultObjects(nmg_State * state)
{
  /*init various data struct set list pointer to NULL or empty*/
  v_room = NULL;
  v_head = NULL;
  v_hand = NULL;
  v_screen = NULL;
  vir_world = NULL;

  /*allocate Lists id's for display lists and save in GLint variables */ 
  vx_quarter_down = glGenLists(1);
  vx_half_down = glGenLists(1);
  vx_quarter_up = glGenLists(1);
  vx_half_up = glGenLists(1);
  rubber_corner = glGenLists(1);
  region_box = glGenLists(1);
//    region_marker = glGenLists(1);
  aim_struct = glGenLists(1);
  red_line_struct = glGenLists(1);
  green_line_struct = glGenLists(1);
  blue_line_struct = glGenLists(1);
  collab_hand_struct = glGenLists(1);
  
  ds_sphere_axis_struct = glGenLists(1);

  //Haptic graphics JM 
  feelPlane_struct =   glGenLists(1);
  feelGrid_struct = glGenLists(1);

  marker_list = (marker_type *)malloc(sizeof(marker_type));
  marker_list->id = glGenLists(1);
  marker_list->next = NULL;

  sweep_struct = glGenLists(1);
  sphere = glGenLists(1);

  /* init world objects */
  myobjects(state);
  /* create subdivided sphere display list */
  init_sphere();

  addChartjunk(&v_screen, state);
  /* End changes */

  // binding display functions to positions in the vlib tree
  addFunctionToFunclist(&vir_world, draw_north_pointing_arrow, state,
			"draw_north_pointing_arrow");

  marker_type *marker_node = marker_list;
  while (marker_node != NULL) {
    addFunctionToFunclist( &vir_world,draw_list, &(marker_node->id),
			   "draw_list(marker_node->id)" );
    marker_node= marker_node->next;
  }

  v_replace_drawfunc(0, V_HAND, myhand, state);
  v_replace_drawfunc(0, V_ROOM, myroom, state);
  v_replace_drawfunc(0, V_HEAD, myhead, state);
  v_replace_drawfunc(0, V_SCREEN, myscreen, state);

  return 0;
}

int initialize_globjects ( nmg_State * /*state*/, const char * fontName) {
  char * ev;

  initializeChartjunk(fontName);

  ev = getenv("V_DISPLAY");
  if (ev && strstr(ev, "crt"))
    g_CRT_correction = 1;

  return 0;
}

void enable_ds_sphere_axis() {
ds_sphere_axis_struct_id = addFunctionToFunclist(&vir_world, draw_list, &ds_sphere_axis_struct,
					       "draw_list(ds_sphere_axis_struct)"); 
}

void disable_ds_sphere_axis() {
	removeFunctionFromFunclist(&vir_world, ds_sphere_axis_struct_id );

}

//JM - addition from TCH branch 11/02
void enableFeelGrid (int on) {

    if(on) {

        feelGrid_struct_id = addFunctionToFunclist(&vir_world, draw_list, &feelGrid_struct,
	     "draw_list(feelGrid_struct)");
    } else {
        removeFunctionFromFunclist(&vir_world,feelGrid_struct_id);
    }
}

//JM - addition from TCH branch 11/02
void enableFeelPlane (nmg_haptic_graphics * haptic_graphics, int on) {

    if(on) {
        feelPlane_struct_id = addFunctionToFunclist(&vir_world, draw_list, &feelPlane_struct,
	     "draw_list(feelPlane_struct)");
    } else {
        removeFunctionFromFunclist(&vir_world,feelPlane_struct);
    }

}