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
#include "graphics_globals.h"
#include "spm_gl.h"
#include "font.h"
#include "chartjunk.h"

#include "Timer.h"
#include "nmg_Globals.h"  // guess we need this for VERBOSE.  Yuck.

#define ARROW_SCALE  0.005
#define xx .525731112119133606
#define zz .850650808352039932
#define SPHERE_DEPTH 2
//XXX this is redundant with math.h, which SOMEBODY ELSE up above
// us is including in their header file.  Yuck.
#define PI 3.1415926535

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
//static nmg_Funclist * v_tracker;
static nmg_Funclist * v_screen;

// indicies of display lists created by similarly-named functions

static GLint vx_quarter_down;
static GLint vx_half_down;
static GLint vx_quarter_up;
static GLint vx_half_up;
static GLint rubber_corner;
static GLint region_marker;
static GLint aim_struct;

static GLint red_line_struct;
static GLint green_line_struct;
static GLint blue_line_struct;

static int red_line_struct_id;
static int green_line_struct_id;
static int blue_line_struct_id;

static GLint collab_hand_struct;

static marker_type * marker_list; // linked list of markers for selected area 

static GLint sweep_struct;
static GLint sphere; /* dim */

static int aim_struct_id;

static int rubber_corner_id;
static int sweep_struct_id;

// ID for marker of user's hand position
static int hand_id;

// ID for marker of actual position of microscope tip
static int trueTip_id;

static int sphere_id;

static int scanline_id;

static int collabHand_id;

// nonzero if we're using a CRT and should play with the definition of
// screen space
static int g_CRT_correction = 0;

static float hand_scale[1];
static float room_scale[1];
static float screen_scale[1];

/*functions prototypes*/
/* remember that v_dlist_ptr_type is a int func_name(int) */

static int myroom (int);
static int myhead (int);
static int myhand (int);
static int myscreen (int);

// used in openGL.c
int myworld (void);

static void myobjects (void);
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

int my_line_mark (void *);
int my_rubber_line (void *);
int my_scanline_indicator (void *);


int replaceDefaultObjects (void);
int clear_world_modechange (int);
int make_aim (const float a [], const float b []);
int make_sweep (const float a [], const float b [],
		const float c [], const float d [] );
int make_rubber_corner (float, float, float, float);

static int selecthand (void *);
static int mycube (void);
static int big_flat_arrow (void); /* dim */
//static int mysphere (void *);
static void init_sphere (void); /* dim */

void position_sphere (float, float, float);

static float sphere_x, sphere_y, sphere_z;


/* When the user changes modes, clear the world of any 
 * icons dependent on that mode. */
int clear_world_modechange(int mode, int style)
{
  switch(mode) {
  case USER_LIGHT_MODE:
    removeFunctionFromFunclist(&vir_world,hand_id);
    break;
  case USER_FLY_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    //removeFunctionFromFunclist(&vir_world,sphere_id);
    break;
  case USER_MEASURE_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    if ( !g_config_chartjunk ) {
      removeFunctionFromFunclist(&vir_world, red_line_struct_id);
      removeFunctionFromFunclist(&vir_world, green_line_struct_id);
      removeFunctionFromFunclist(&vir_world, blue_line_struct_id);
    }
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
    if (g_config_trueTip) {
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
    break;
  case USER_GRAB_MODE:
    removeFunctionFromFunclist(&v_hand,hand_id);
    removeFunctionFromFunclist(&vir_world,aim_struct_id);
    break;
  case USER_MEAS_MOVE_MODE:
    break;
  case USER_CENTER_TEXTURE_MODE:
    removeFunctionFromFunclist(&vir_world,sphere_id);
    break;
  default:
    break;
  }

  /* The other user's hand doesn't depend on what your hand is doing,
     but we seem to need this anyway... */
  if (g_draw_collab_hand) {
    removeFunctionFromFunclist(&vir_world, collabHand_id);
  }
  /* */

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
int init_world_modechange(int mode, int style)
{
  if (g_config_planeonly) {
    // display NOTHING but the plane - used by nmg_RenderServer
    return 0;
  }
  switch(mode) {
  case USER_LIGHT_MODE:
    hand_id = addFunctionToFunclist(&vir_world,lighthand,hand_scale, "hand_scale");
    break;
  case USER_FLY_MODE:
    hand_id = addFunctionToFunclist(&v_hand, Tip, NULL, "Tip");
    //sphere_id = addFunctionToFunclist(&vir_world,mysphere,NULL, "mysphere"); 
    break;
  case USER_MEASURE_MODE:
    hand_id = addFunctionToFunclist(&v_hand,measure_hand, &g_hand_color,
                                                "measure_hand"); 
    break;
  case USER_PLANE_MODE:
    sphere_id = addFunctionToFunclist(&vir_world,mysphere,NULL, "mysphere");
    hand_id = addFunctionToFunclist(&v_hand, Tip, NULL, "Tip"); 
    aim_struct_id = addFunctionToFunclist(&vir_world,draw_list,&aim_struct,
                                                "draw_list(aim_struct)");
    break;
  case USER_LINE_MODE:
  case USER_PLANEL_MODE:
    hand_id = addFunctionToFunclist(&v_hand, Tip, NULL, "Tip"); 
    sphere_id = addFunctionToFunclist(&vir_world, mysphere, NULL, "mysphere");
    aim_struct_id = addFunctionToFunclist(&vir_world, draw_list, &aim_struct,
					  "draw_list(aim_struct)");
    if (g_config_trueTip) {
      trueTip_id = addFunctionToFunclist(&vir_world, TrueTip, NULL, "true tip");
    }
    if (style == SWEEP) {
      sweep_struct_id = addFunctionToFunclist(&vir_world, draw_list, &sweep_struct,
					      "draw_list(sweep_struct)");
    }
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
    break;
  case USER_GRAB_MODE:
    aim_struct_id = addFunctionToFunclist(&vir_world, draw_list, &aim_struct,
					  "draw_list(aim_struct)");
    hand_id = addFunctionToFunclist(&v_hand,grabhand,hand_scale, "grabhand");   
    break;
  case USER_SCANLINE_MODE:
    hand_id = addFunctionToFunclist(&v_hand, selecthand, NULL, "selecthand");
    break;
  case USER_MEAS_MOVE_MODE:
    break;
  case USER_CENTER_TEXTURE_MODE:
    sphere_id = addFunctionToFunclist(&vir_world,mysphere,NULL,"mysphere"); 
    fprintf( stderr, "Sphere added to functionlist\n" );
    break;
  default:
    break;
  }

  /* a few icons get added every time, no matter what mode we are
   * entering */
  addFunctionToFunclist(&vir_world,draw_north_pointing_arrow, NULL,
                        "draw_north_pointing_arrow");  /* dim */
  if (g_config_measurelines) {
    red_line_struct_id = addFunctionToFunclist(&vir_world, draw_list, &red_line_struct,
					       "draw_list(red_line_struct)"); 
    green_line_struct_id = addFunctionToFunclist(&vir_world, draw_list, &green_line_struct,
					       "draw_list(green_line_struct)"); 
    blue_line_struct_id = addFunctionToFunclist(&vir_world, draw_list, &blue_line_struct,
					       "draw_list(blue_line_struct)"); 
  }
  if (g_scanline_display_enabled) {
	scanline_id = addFunctionToFunclist(&vir_world, my_scanline_indicator,
		(void *)g_scanlinePt, "scanline_indicator");
  }

  /* The other user's hand doesn't depend on what your hand is doing,
     but we seem to need this anyway... */
  if (g_draw_collab_hand) {
    collabHand_id =
        addFunctionToFunclist(&vir_world, draw_list, &collab_hand_struct,
			      "draw_list(collab_hand_struct)");
  }
  /* */
  return 0;
}

void enableCollabHand (int enable) {
  g_draw_collab_hand = enable;
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
void enableScanlinePositionDisplay(const int enable) {

    g_scanline_display_enabled = enable;
    if (enable){
	scanline_id = addFunctionToFunclist(&vir_world,my_scanline_indicator,
                                (void *)g_scanlinePt, "scanline_indicator");
    } else {
	removeFunctionFromFunclist(&vir_world, scanline_id);
    }
}

/* objects in the world space*/
void myobjects(void)
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

  /* region marker */
  glNewList(region_marker,GL_COMPILE);
  glEndList();

  //show lines where old selected areas were
  glNewList(marker_list->id, GL_COMPILE); /* empty list?*/
  glEndList();

  glNewList(rubber_corner,GL_COMPILE);
		/*empty list?*/
  glEndList();

  glNewList(sweep_struct,GL_COMPILE);
		/*empty list?*/
  glEndList();
  
  /* vx_quarter_up structure */
  glNewList(vx_quarter_up,GL_COMPILE);
    glPushMatrix();
    glScalef(ARROW_SCALE * g_icon_scale,
	     ARROW_SCALE * g_icon_scale,
	     ARROW_SCALE * g_icon_scale);
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
    glScalef(ARROW_SCALE * g_icon_scale,
	     ARROW_SCALE * g_icon_scale,
	     ARROW_SCALE * g_icon_scale);
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

int make_rubber_corner(float x_min,float y_min, float x_max,float y_max)
{
        VertexType Points[4];
	float z_min,z_max;
	int i,j;

	int num_x = g_inputGrid->numX() - 1;
	int num_y = g_inputGrid->numY() - 1;

	BCPlane* plane = g_inputGrid->getPlaneByName
                    (g_heightPlaneName);
	if (plane == NULL)
	{
	    fprintf(stderr, "Error in Replace_Rubber_Corner: could not get plane!\n");
	    return -1;
	}

	i=0;
	j=0;
	z_min=z_max= plane->scaledValue(i,j);
	i=0;
	j=num_y;
	if(z_min> plane->scaledValue(i,j)) z_min=plane->scaledValue(i,j);
	if(z_max<plane->scaledValue(i,j)) z_max=plane->scaledValue(i,j);
	i=num_x;
	j=num_y;
	if(z_min>plane->scaledValue(i, j)) z_min=plane->scaledValue(i, j);
	if(z_max<plane->scaledValue(i, j)) z_max=plane->scaledValue(i, j);
	i=num_x;
	j=0;
	if(z_min>plane->scaledValue(i, j)) z_min=plane->scaledValue(i, j);
	if(z_max<plane->scaledValue(i, j)) z_max=plane->scaledValue(i, j);

	z_min=z_min-10/plane->scale();
	z_max=z_max+10/plane->scale();

	v_gl_set_context_to_vlib_window(); 
	glNewList(rubber_corner,GL_COMPILE);
	glColor3f(1.0,0.0,0.0);
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
  
	glEndList();
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
      glVertex3f(750, 100.0*cos(i*PI/8.0), 100.0*sin(i*PI/8.0));
    } 
  glEnd();

  glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0, 0.0, 0.0);
    for (i = 0; i < 17; i++) {
      glVertex3f(750, 100.0*cos(i*PI/8.0), 100.0*sin(i*PI/8.0));
    }
  glEnd();
  return 0;
}

static char     *MODE_NAMES[] = {
                        "fly",
                        "grab",
                        "scale up",
                        "scale down",
                        "servo",
                        "pulse",
                        "measure line",
                        "line",
                        "sweep",
                        "measure",
                        "position light",
                        "touch",
                        "touch live",
                        "blunt tip",
                        "comb",
                        "center texture",
                        "line scan" };

int make_collab_hand_icon (double pos[], double rotate[], vrpn_int32 mode) {
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

// insert scale here?
    v_get_world_from_head(0, &headxform);
    invscale = 1.0f / headxform.scale;
    glScalef(invscale, invscale, invscale);

    make_cone();
    //printf("XXX Making the collaborative hand icon for %s\n",MODE_NAMES[mode]);
  glEndList();
  return (collab_hand_struct); 
}

int make_selected_region_marker (float x_min, float y_min, float x_max, float y_max)
{       VertexType Points[5];
        int num_x,num_y;
        int i;

        num_x = g_inputGrid->numX() - 1;
        num_y = g_inputGrid->numY() - 1;

        BCPlane* plane = g_inputGrid->getPlaneByName
                    (g_heightPlaneName);
        if (plane == NULL) {
          fprintf(stderr, "Error in Add_Region_Marker: could not get plane!\n");
          return -1;
        }

        Points[0][Z] = plane->value(0, 0) * plane->scale();
        Points[0][X] = x_min;
        Points[0][Y] = y_min;

        Points[1][Z] = plane->value(num_x-1, 0) * plane->scale();
        Points[1][X] = x_max;
        Points[1][Y] = y_min;

        Points[2][Z] = plane->value(num_x-1, num_y-1) * plane->scale();
        Points[2][X] = x_max;
        Points[2][Y] = y_max;

        Points[3][Z] = plane->value(0, num_y-1) * plane->scale();
        Points[3][X] = x_min;
        Points[3][Y] = y_max;

        Points[4][Z] = plane->value(0, 0) * plane->scale();
        Points[4][X] = x_min;
        Points[4][Y] = y_min;
	
	marker_type * marker_node = (marker_type *) malloc(sizeof(marker_type));
	if (marker_node == NULL) {
		fprintf(stderr,"Out of memory looking for marker node\n");
		exit(-1);
	}
        
        v_gl_set_context_to_vlib_window(); 
        marker_node->id = glGenLists(1);
        glNewList(marker_node->id,GL_COMPILE);
       
        glColor3f(0.3,1.0,1.0);

        glLineWidth(2.0);	        
	glBegin(GL_LINE_STRIP); 
        VERBOSE(20, "          glBegin(GL_LINE_STRIP)");
	for(i=0;i<5;i++) 
	  glVertex3fv(Points[i]); 
        VERBOSE(20, "          glEnd()");
	glEnd();

	glEndList();

	marker_node->next = marker_list; 
	marker_list = marker_node; 
	
        addFunctionToFunclist(&vir_world,draw_list,&(marker_node->id),
                                                "draw_list(marker_node)");

        return(marker_list->id);
}


int make_red_line (const float a[], const float b[])
{  
  if ( !g_config_chartjunk ) 
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

int make_green_line (const float a[], const float b[])
{  
  if ( !g_config_chartjunk ) 
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

int make_blue_line (const float a[], const float b[])
{  
  if ( !g_config_chartjunk ) 
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


int make_sweep (const float a [], const float b [],
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
#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
  glCallList(*listnum);

#ifndef FLOW
  glPopAttrib();
#endif 

  glPopMatrix();
  return(0);
}

int myroom (int)
{
  nmg_Funclist *head;
  head = v_room;

  // Don't draw anything if chart junk is off
  if (!g_config_chartjunk) {
	return 0;
  }

  // Set material parameters for the space, then draw things in room space.
  // Since all in room space is text or lines, set measure materials.

  TIMERVERBOSE(5, mytimer, "globjects.c:myroom:spm_set_measure_materials");

  spm_set_measure_materials();

  TIMERVERBOSE(5, mytimer, "globjects.c:myroom:end spm_set_measure_materials");

  while(head != NULL) {
      if (spm_graphics_verbosity >= 12)
        fprintf(stderr,"            Drawing %s\n", head->name);
      head->function(head->data);
      head=head->next;
  }
  return(0);
}

int myhead (int)
{
  nmg_Funclist *head;
  head = v_head;

  // Don't draw anything if chart junk is off
  if (!g_config_chartjunk) {
	return 0;
  }

  // Set material parameters for the space, then draw things in head space.
  // Since all in head space is text or lines, set measure materials.
  spm_set_measure_materials();
  while(head != NULL) {
      if (spm_graphics_verbosity >= 12)
        fprintf(stderr,"            Drawing %s\n", head->name);
      head->function(head->data);
      head=head->next;
  }
  return(0);
}

int myhand (int)
{
  nmg_Funclist *head;
  head = v_hand;

  // Don't draw anything if chart junk is off
  if (!g_config_chartjunk) {
	return 0;
  }

  // Set material parameters for the space, which holds the icon 
  spm_set_icon_materials(); 
  while(head != NULL) {
      if (spm_graphics_verbosity >= 12)
        fprintf(stderr,"            Drawing %s\n", head->name);
      head->function(head->data);
      head=head->next;
  }
  return(0);
}

int myscreen (int)
{
  nmg_Funclist *head;
#ifndef FLOW
  GLint saveMatrixMode;
#endif
  head = v_screen;

  // Don't draw anything if chart junk is off
  if (!g_config_chartjunk) {
	return 0;
  }
  
  // Set material parameters for the space, then draw things in head space.
  // Since all in screen space is text or lines, set measure materials.

  TIMERVERBOSE(5, mytimer, "myscreen;spm_set_measure_materials");

  spm_set_measure_materials();

  TIMERVERBOSE(5, mytimer, "myscreen; end spm_set_measure_materials");

  // Ugly kluge to put text on CRTs "in front of" the screen.
  // There isn't enough depth on the projection stack to push/pop
  // matrices, so instead we manually multiply and hope too much isn't
  // lost to floating point accuracy.
  // Correct values for the translation and scale were found by
  // iteration.

#ifndef FLOW
  if (g_CRT_correction) {
    glGetIntegerv(GL_MATRIX_MODE, &saveMatrixMode);
    if (saveMatrixMode != GL_PROJECTION)
      glMatrixMode(GL_PROJECTION);

    glTranslatef(-0.0225f, -0.018f, 0.0f);
    glScalef(0.075f, 0.075f, 0.2f);
  }
#endif

  while (head != NULL) {

    TIMERVERBOSE(5, mytimer, "myscreen:draw function");

    if (spm_graphics_verbosity >= 12)
      fprintf(stderr,"            Drawing %s\n", head->name);

    head->function(head->data);

    TIMERVERBOSE(5, mytimer, "myscreen:done draw function");

    head = head->next;
  }

#ifndef FLOW
  if (g_CRT_correction) {
    glScalef(13.333333f, 13.333333f, 5.0f);
    glTranslatef(0.0225f, 0.018f, 0.0f);
    if (saveMatrixMode != GL_PROJECTION)
      glMatrixMode((GLenum) saveMatrixMode);
  }
#endif

  return(0);
}

int myworld (void)
{
  nmg_Funclist *head;
  head=vir_world;
  
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
  float *size;
  size = (float *)data;

  glPushMatrix();
#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif

  glColor3f(0.0,1.0,0.0); 
  glScalef(*size * g_icon_scale, *size * g_icon_scale, *size * g_icon_scale);
  mycube();

#ifndef FLOW
  glPopAttrib();
#endif

  glPopMatrix();
  return(0);
}

int lighthand(void *data)
{
    float x_wide = g_inputGrid->maxX() - g_inputGrid->minX();
    float y_wide = g_inputGrid->maxY() - g_inputGrid->minY();
    float z_value;

    float size = 0.15*x_wide;
    v_xform_type	worldFromHand;
    q_matrix_type hand_mat;
    q_type q_world_from_hand;

    //size = *((float *)data);

    glPushMatrix();

    BCPlane *height = g_inputGrid->getPlaneByName
        (g_heightPlaneName);
  
    if (height) {
        z_value = height->scale() * 0.5 * (height->maxNonZeroValue() + 
                                           height->minNonZeroValue());
//            z_value = height->scaledValue(g_inputGrid->numX() / 2,
//                                          g_inputGrid->numY() -1);
    } else {
        z_value = 0.0f;
    }
    
    // Approximate center of the surface. 
    glTranslatef( g_inputGrid->minX() + x_wide/2.0,
                  g_inputGrid->minY() + y_wide/2.0,
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

  glScalef(size * g_icon_scale, size * g_icon_scale, size * g_icon_scale);

#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif

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
  
#ifndef FLOW
  glPopAttrib();
#endif

  glPopMatrix();
  return(0);
}


int selecthand(void *)
{
  static float size=0.02;

  glPushMatrix();

#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif

  glColor3f(1.0,0.0,0.0); 
  glScalef(size * g_icon_scale, size * g_icon_scale, size * g_icon_scale);
  mycube();

#ifndef FLOW
  glPopAttrib();
#endif

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

int big_flat_arrow(void)
{

  // Don't draw anything if chart junk is off
  if (!g_config_chartjunk) {
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
      //subdivide(vdata[trindex[i][0]],vdata[trindex[i][1]],vdata[trindex[i][2]],SPHERE_DEPTH);
      //needed to make the vertices get traversed the other way, otherwise we
      //get a backfacing sphere
      subdivide(vdata[trindex[i][2]],vdata[trindex[i][1]],vdata[trindex[i][0]],SPHERE_DEPTH);
    }
  glEndList();

}


void position_sphere(float x,float y, float z)
{
  sphere_x=x;
  sphere_y=y;
  // position is at bottom of sphere, r=g_sphere_scale, maybe
  sphere_z=z+g_sphere_scale;	
}

int mysphere(void * /*data*/ )
{
  //float x_wide = g_inputGrid->maxX() - g_inputGrid->minX();

       	glPushMatrix();
#ifndef FLOW
	glPushAttrib(GL_CURRENT_BIT);
#endif
	glColor3f(1.0,0.0,0.0);
        glTranslatef(sphere_x,sphere_y,sphere_z);
	//the scaling is obtained from a tcl slider
	glScalef(g_sphere_scale, g_sphere_scale, g_sphere_scale);
        glCallList(sphere);
#ifndef FLOW
	glPopAttrib();
#endif
	glPopMatrix();

	return(0);
}

// Position of user's hand

int Tip(void *data)
{
	data = data;	// Keep the compiler happy
        int i;
        static float handlescale = 0.1f;

	glPushMatrix();

/* I don't know who keeps taking this out but unless we change
	how the tip is drawn it seems to be necessary */
	glRotatef(-90.0, 0.0,1.0,0.0); // XXX HACK!!! (or was it wrong before!?)


#ifndef FLOW
	glPushAttrib(GL_CURRENT_BIT);
#endif

	glColor3f(0.0,0.0,1.0);
	glScalef(handlescale * g_icon_scale,
                 handlescale * g_icon_scale,
                 handlescale * g_icon_scale);

	/* top */
	glBegin(GL_TRIANGLE_FAN);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_FAN)");
	glVertex3f(TIP_HEIGHT,0.0,0.0);
	for (i=0;i<17;i++)
	  glVertex3f(TIP_HEIGHT,0.1*cos(i*PI/8.0),0.1*sin(i*PI/8.0));
        VERBOSE(20, "          glEnd()");
	glEnd();
	
        /* side */
	glBegin(GL_TRIANGLE_FAN);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_FAN)");
	glVertex3f(0.0,0.0,0.0);
	for (i=0;i<17;i++)
	  glVertex3f(TIP_HEIGHT,0.1*cos(i*PI/8.0),0.1*sin(i*PI/8.0));
        VERBOSE(20, "          glEnd()");
	glEnd();

#ifndef FLOW
	glPopAttrib();
#endif

	glPopMatrix();

	return(0);
}	/* Tip */


// Last known position of microscope's actual tip
// Needs to be differentiated from Tip() for latency compensation

// static
int TrueTip (void *)
{
        int i;

        static const float handlescale = 125.0f;
        //static const float handlescale = 50000.0f;
          // World space has a VERY different scale than hand space,
          // so we need to have 50000x the scale when we draw the
          // tip.
          // Value changed by a factor of 400 sometime before June 1999

	glPushMatrix();

        // Translate to the right point
        glTranslatef(g_trueTipLocation[0],
                     g_trueTipLocation[1],
                     g_trueTipLocation[2]);

        if (spm_graphics_verbosity >= 4)
          fprintf(stderr, "Drawing true tip at (%.2f %.2f %.2f) "
                          "with scale %.2f (net scale %.2f).\n",
             g_trueTipLocation[0], g_trueTipLocation[1], g_trueTipLocation[2],
             g_trueTipScale, g_trueTipScale * handlescale * g_icon_scale);

	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

#ifndef FLOW
	glPushAttrib(GL_CURRENT_BIT);
#endif

	glColor3f(0.8f, 0.0f, 0.4f);
	glScalef(g_trueTipScale * handlescale * g_icon_scale,
                 g_trueTipScale * handlescale * g_icon_scale,
                 g_trueTipScale * handlescale * g_icon_scale);

	// top
	glBegin(GL_TRIANGLE_FAN);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_FAN)");
	glVertex3f(TIP_HEIGHT, 0.0f, 0.0f);
	for (i = 0; i < 17; i++)
	  glVertex3f(TIP_HEIGHT, 0.1f * cos(i * PI / 8.0f),
                                 0.1f * sin(i * PI / 8.0f));
        VERBOSE(20, "          glEnd()");
	glEnd();
	
        // side
	glBegin(GL_TRIANGLE_FAN);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_FAN)");
	glVertex3f(0.0f, 0.0f, 0.0f);
	for (i = 0; i < 17; i++)
	  glVertex3f(TIP_HEIGHT, 0.1f * cos(i * PI / 8.0f),
                                 0.1f * sin(i * PI / 8.0f));
        VERBOSE(20, "          glEnd()");
	glEnd();

#ifndef FLOW
	glPopAttrib();
#endif

	glPopMatrix();

	return 0;
}	/* Tip */




#define RED   1
#define GREEN 2
#define BLUE  3
int measure_hand(void *data)
{
  static float handlescale = 0.012f;
  int *color;
  GLfloat matspec[4] = { 0.5, 0.5, 0.5, 0.0 };
  color = (int *)data;

  glPushMatrix();
#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
 
  if( *color==RED) {
    glColor3f(1.0,0.2,0.2);   //red
  } else if(*color==GREEN) {
   // No it's not green - it's yellow!
   // To avoid red-green colorblindness conflicts. 
   glColor3f(0.9,0.9,0.2);
  } else {
     glColor3f(0.2,0.2,1.0);   //blue
  }
  glScalef(handlescale * g_icon_scale,
           handlescale * g_icon_scale,
           handlescale * g_icon_scale);

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
#ifndef FLOW
  glPopAttrib();
#endif 
  glPopMatrix();
  return(0);
}
#undef RED
#undef GREEN
#undef BLUE






int vx_down_icon(void *data)
{  
	data = data;	// Keep the compiler happy

#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
  glPushMatrix();
  glColor3f(1.0,0.0,0.0);
    glCallList(vx_half_down);
    glRotatef(180.0, 0.0,0.0,1.0);
    glCallList(vx_half_down);
  glPopMatrix();
#ifndef FLOW
  glPopAttrib();
#endif 

  return(0);
}	/* vx_down_icon */



int vx_up_icon(void *data)
{
	data = data;	// Keep the compiler happy

#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
  glPushMatrix();
  glColor3f(0.0,1.0,0.0);
    glCallList(vx_half_up);
    glRotatef(180.0, 0.0,0.0,1.0);
    glCallList(vx_half_up);
  glPopMatrix();
#ifndef FLOW
  glPopAttrib();
#endif 

  return(0);
}	/* vx_up_icon */

/* dim */
int draw_north_pointing_arrow (void *)
{
	float x_wide = g_inputGrid->maxX() - g_inputGrid->minX();
	float y_wide = g_inputGrid->maxY() - g_inputGrid->minY();
	float z_value;

	float scale  = 0.25f * x_wide;

	BCPlane *height = g_inputGrid->getPlaneByName
                    (g_heightPlaneName);

	if (height) {
		z_value = height->scaledValue(g_inputGrid->numX() / 2,
                                              g_inputGrid->numY() - 1);
	} else {
		z_value = 0.0f;
	}
		
#ifndef FLOW
	glPushAttrib(GL_CURRENT_BIT);
#endif
	glPushMatrix();
	glColor3f(1.0,0.0,0.0);
	glTranslatef( g_inputGrid->minX() + x_wide/2.0,
		      g_inputGrid->minY() + 1.02 * y_wide,
		      z_value);  
	glScalef(scale, scale, scale);
	big_flat_arrow(); 
	glPopMatrix();
#ifndef FLOW
	glPopAttrib();
#endif
	return(0);
}	/* draw_north_pointing_arrow */

/* markers for endpoints in a polyline. */
int my_line_mark(void *data)
{
    float *a = (float *)(data);  // extract endpoints of line
    float *b = (&(a[3]));

#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
    glColor3f(0.0,0.9,0.9);
    glLineWidth(2.0);
    make_line(a,b);
    glLineWidth(1.0);
#ifndef FLOW
    glPopAttrib();
#endif 
    
    return(0);
}

int my_3d_point_marker(void *data) {

  data = data;
  glPushMatrix();
#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
  glColor3f(0.0,1.0,0.0);
  glTranslatef(sphere_x,sphere_y,sphere_z);
  //the scaling is obtained from a tcl slider
  glScalef(g_sphere_scale, g_sphere_scale, g_sphere_scale);
  glCallList(sphere);
#ifndef FLOW
  glPopAttrib();
#endif
  glPopMatrix();
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
    b[0] = temp[2];
    b[1] = temp[3];

    // Now we find the range of z values.
    BCPlane* plane = g_inputGrid->getPlaneByName
                    (g_heightPlaneName);
    if (plane == NULL) {
	fprintf(stderr, "Error in my_rubber_line: could not get plane!\n");
	return -1;
    }
    
    double z_min,z_max, b_val;
    double p_scale = plane->scale();
    plane->valueAt(&z_max, a[0], a[1]);
    z_max *=p_scale;
    plane->valueAt(&b_val, b[0], b[1]);
    b_val *=p_scale;
    
    z_min = z_max;

    if(z_min> b_val) {
        z_min=b_val;
    }
    if(z_max< b_val) 
	z_max=b_val;

    // Make limits 10% bigger (+ const) so lines are visible above min and max. 
    double z_range = z_max - z_min;
    z_min = (z_min-(z_range * 0.1 + 500));
    z_max = (z_max+(z_range * 0.1 + 500));
    z_range = z_max - z_min;

    // Now we draw some lines - 7 horizontal lines.
#ifndef FLOW
    glPushAttrib(GL_CURRENT_BIT);
#endif
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

#ifndef FLOW
    glPopAttrib();
#endif 
    return(0);

}

#define NUM_SCANLINE_INDICATOR_POINTS 50
int my_scanline_indicator(void *data) {
    // data contains coords of two points to draw
    // the lines between: x1, y1,z1, x2, y2, z2
    float * temp = (float *)data;
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
    BCPlane* plane = g_inputGrid->getPlaneByName(g_heightPlaneName);
    if (plane == NULL) {
        fprintf(stderr, "Error in my_scanline_indicator:can't get plane!\n");
        return -1;
    }
   
    float p_scale = plane->scale();
    float r0, r1;

//     int j;
//     VertexType pt;
//     float r2, r3;

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

#ifndef FLOW
    glPushAttrib(GL_CURRENT_BIT);
#endif
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

    //glDisable(GL_DEPTH_TEST);
    make_line(b[0], b[NUM_SCANLINE_INDICATOR_POINTS-1]);
    for (i = 0; i < NUM_SCANLINE_INDICATOR_POINTS-1; i++){
    	make_line(a[i],a[i+1]);
	make_line(a[i], b[i]);
    }
    make_line(a[NUM_SCANLINE_INDICATOR_POINTS-1],
		b[NUM_SCANLINE_INDICATOR_POINTS-1]);
    //glEnable(GL_DEPTH_TEST);
#ifndef FLOW
    glPopAttrib();
#endif
    return(0);
}

int replaceDefaultObjects(void)
{
  /*init various data struct set list pointer to NULL or empty*/
  v_room = NULL;
  v_head = NULL;
  v_hand = NULL;
  //v_tracker = NULL;
  v_screen = NULL;
  vir_world = NULL;

  /*allocate Lists id's for display lists and save in GLint variables */ 
  vx_quarter_down = glGenLists(1);
  vx_half_down = glGenLists(1);
  vx_quarter_up = glGenLists(1);
  vx_half_up = glGenLists(1);
  rubber_corner = glGenLists(1);
  region_marker = glGenLists(1);
  aim_struct = glGenLists(1);
  red_line_struct = glGenLists(1);
  green_line_struct = glGenLists(1);
  blue_line_struct = glGenLists(1);
  collab_hand_struct = glGenLists(1); 

  marker_list = (marker_type *)malloc(sizeof(marker_type));
  marker_list->id = glGenLists(1);
  marker_list->next = NULL;

  sweep_struct = glGenLists(1);
  sphere = glGenLists(1);

  /* init world objects */
  myobjects();
  /* create subdivided sphere display list */
  init_sphere();

  *hand_scale = 0.02;
  //hand_id = addFunctionToFunclist(&v_hand,grabhand,hand_scale, "grabhand");

  /* Moving text feedback displays from head space to screen space to make them
   * stationary in head tracked mode.
   */
  *room_scale = 1.0;
  *screen_scale = 1.0;

  // MOVED to chartjunk.c
  addChartjunk(&v_screen, screen_scale);

  /* End changes */

  // binding display functions to positions in the vlib tree

  addFunctionToFunclist(&vir_world, draw_north_pointing_arrow, NULL,
	"draw_north_pointing_arrow");

  // THis is commented out as we now call init_world_modechange on graphics startup, so
  // it should be unnessary
  //  if (g_config_measurelines) {
  //    red_line_struct_id = addFunctionToFunclist(&vir_world, draw_list, &red_line_struct,
  //                          "draw_list(red_line_struct)"); 
  //    green_line_struct_id = addFunctionToFunclist(&vir_world, draw_list, &green_line_struct,
  //                          "draw_list(green_line_struct)");
  //    blue_line_struct_id = addFunctionToFunclist(&vir_world, draw_list, &blue_line_struct,
  //                          "draw_list(blue_line_struct)");
  //  }

  marker_type *marker_node = marker_list;
  while (marker_node != NULL) {
    addFunctionToFunclist(&vir_world,draw_list, &(marker_node->id),
                                                "draw_list(marker_node->id)");
    marker_node= marker_node->next;
  }

  v_replace_drawfunc(0, V_HAND, myhand);
  v_replace_drawfunc(0, V_ROOM, myroom);
  v_replace_drawfunc(0, V_HEAD, myhead);
  v_replace_drawfunc(0, V_SCREEN, myscreen);

  return 0;
}

int make_rubber_line_point (const PointType point[2], Position_list * p) {
//int make_rubber_line_point (const float point [2][3], Position_list * p) {
  //static float rep [4];
  int list_id;

  list_id = addFunctionToFunclist(&vir_world, my_line_mark, (void *) point,
                                  "my_line_mark");

  //   If this is the first point in the polyline,
  // initialize the rubber-band line.
  if (p->empty()) {

    poly_rubber_line_id =
            addFunctionToFunclist(&vir_world,
                                  my_rubber_line,
                                  (void *) g_rubberPt,
                                  "my_rubber_line");
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
int make_rubber_line_point (
    const PointType /*top*/, const PointType /*bot*/,
    Position_list * p, int index)
{
    //static float rep [4];
    int list_id;

    if ( draw_rubber_line[index] ) { 
	float *rubber_point = new float[4];
	rubber_point[0] = g_rubberSweepPts[index][0];
	rubber_point[1] = g_rubberSweepPts[index][1];
	rubber_point[2] = g_rubberSweepPts[index][2];
	rubber_point[3] = g_rubberSweepPts[index][3];
	
	list_id = addFunctionToFunclist(&vir_world, my_rubber_line,
					(void *) rubber_point,
					"my_rubber_line");
	p->insert( rubber_point[0], rubber_point[1], list_id);

	if ( draw_rubber_corner_line[index] ) {
	    float *rubber_corner_point = new float[4];
	    rubber_corner_point[0] = g_rubberSweepPtsSave[index][0];
	    rubber_corner_point[1] = g_rubberSweepPtsSave[index][1];
	    rubber_corner_point[2] = g_rubberSweepPtsSave[index][2];
	    rubber_corner_point[3] = g_rubberSweepPtsSave[index][3];
	    
	    list_id = addFunctionToFunclist(&vir_world, my_rubber_line,
					    (void *) rubber_corner_point,
					    "my_rubber_line");
	    p->insert( rubber_point[0], rubber_point[1], list_id);

	    g_rubberSweepPtsSave[index][0] = g_rubberSweepPts[index][2];
	    g_rubberSweepPtsSave[index][1] = g_rubberSweepPts[index][3];
	}
	else {
	    g_rubberSweepPtsSave[index][0] = g_rubberSweepPts[index][2];
	    g_rubberSweepPtsSave[index][1] = g_rubberSweepPts[index][3];

	    poly_sweep_rubber_line_id[index + 2] =
		addFunctionToFunclist(&vir_world,
				      my_rubber_line,
				      (void *) g_rubberSweepPtsSave[index],
				      "my_rubber_line");
	    draw_rubber_corner_line[index] = 1;
	}
     }
    else if ( p->empty() ) { 
	poly_sweep_rubber_line_id[index] =
	    addFunctionToFunclist(&vir_world,
				  my_rubber_line,
				  (void *) g_rubberSweepPts[index],
				  "my_rubber_line");
	draw_rubber_line[index] = 1;
    }
    return list_id;
}

void empty_rubber_line (Position_list * p) {

   p->start();
   while( p->notDone()) {
      //printf("commit - remove func: %d\n", (p->curr())->iconID());
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

int initialize_globjects (const char * fontName) {
  char * ev;

  initializeChartjunk(fontName);

  ev = getenv("V_DISPLAY");
  if (ev && strstr(ev, "crt"))
    g_CRT_correction = 1;

  return 0;
}


