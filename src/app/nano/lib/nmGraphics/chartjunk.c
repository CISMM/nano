
/* 
   code that implement the functionalities of microscape in openGL 
   created by Qiang Liu 08/10/95

  Change log:
    Tom Hudson 17 Mar 97:  Minimized exposure of internals in header
         and with static declarations.  Commented what I could.
*/


#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_Types.h>
#include <nmb_Globals.h>

#include <nmm_Globals.h>
#ifndef USE_VRPN_MICROSCOPE	// #ifndef #else #endif Added by Tiger
#include <Microscope.h>
#else
#include <nmm_MicroscopeRemote.h>
#endif

#include "graphics_globals.h"  // for VERBOSE
#include "font.h"  // drawStringInFont()
#include "nmg_Funclist.h"



#define ARROW_SCALE  0.005
#define xx .525731112119133606
#define zz .850650808352039932
#define SPHERE_DEPTH 2
//XXX this is redundant with math.h, which SOMEBODY ELSE up above
// us is including in their header file.  Yuck.
#define PI 3.1415926535

#include <v.h>  // to define GLfloat
typedef GLfloat VertexType[3];

struct Measure_Data {
  float rgDxy,gbDxy,brDxy;
  float rgDz, gbDz, brDz;
  float rgbAngle,gbrAngle,brgAngle;
};

int myfont;

static Measure_Data measure_data;

#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif


// caludate dxy,dz,and angles among three measure lines
static Measure_Data calculate (const PointType red_pos,
                               const PointType green_pos,
                               const PointType blue_pos)
{
  Measure_Data  data;
  double        rg[2], gb[2], br[2];
  double        rgb_dot, gbr_dot;

  rg[X] = red_pos[X] - green_pos[X];
  rg[Y] = red_pos[Y] - green_pos[Y];
  gb[X] = green_pos[X] - blue_pos[X];
  gb[Y] = green_pos[Y] - blue_pos[Y];
  br[X] = blue_pos[X] - red_pos[X];
  br[Y] = blue_pos[Y] - red_pos[Y];

  data.rgDxy = sqrt( rg[X]*rg[X] + rg[Y]*rg[Y]);
  data.gbDxy = sqrt( gb[X]*gb[X] + gb[Y]*gb[Y]);
  data.brDxy = sqrt( br[X]*br[X] + br[Y]*br[Y]);

//If any of the two points overlap, angles are set to 0
  if(data.rgDxy==0 || data.gbDxy==0 || data.brDxy==0 ) {
      data.rgbAngle = 0;
      data.gbrAngle = 0;
      data.brgAngle = 0;
  }

  else {
      rgb_dot = -(rg[X]*gb[X] + rg[Y]*gb[Y]);
      data.rgbAngle = acos(rgb_dot/data.rgDxy/data.gbDxy) * 180 / M_PI;

      gbr_dot = -(br[X]*gb[X] + br[Y]*gb[Y]);
      data.gbrAngle = acos(gbr_dot/data.brDxy/data.gbDxy) * 180 / M_PI;

      data.brgAngle = 180 - data.gbrAngle -data.rgbAngle;
  }
  return data;

}






int scale_display (void * data) {

  float * size;
  size = (float *) data;

  char message[1000];
#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
  glPushMatrix();

  glColor3f(1.0f, 1.0f, 1.0f);
  glScalef(*size, *size, *size);
  glTranslatef(0.053f, 0.107f, -0.00001f);

  double scale = 1.0;

//fprintf(stderr, "scale_display:  height plane %s.\n", g_heightPlaneName);

  BCPlane * plane = g_inputGrid->getPlaneByName
              (g_heightPlaneName);
  if (plane == NULL)
      fprintf(stderr, "Error in scale_display: could not get plane!\n");
  else
      scale = plane->scale();
  sprintf(message,"Z is from %s (%s), scale is %g",
        g_heightPlaneName,
        plane->units()->Characters(), scale);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
#ifndef FLOW
  drawStringInFont(myfont, message);
#endif
  glPopMatrix(); 
#ifndef FLOW
  glPopAttrib();
#endif
  return(0);
}


int x_y_width_display (void *) {

  char message[1000];

  sprintf(message,"Grid dX is %gnm, dY is %gnm",
        g_inputGrid->maxX() - g_inputGrid->minX(),
        g_inputGrid->maxY() - g_inputGrid->minY());

  // Print the message at the appropriate location on the screen
#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
  glPushMatrix();
  glColor3f(1.0f, 1.0f, 1.0f);
  glTranslatef(0.053f, 0.08f,-0.00001f);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
#ifndef FLOW
  drawStringInFont(myfont, message);
#endif
  glPopMatrix(); 
#ifndef FLOW
  glPopAttrib();
#endif

  return(0);
}


int height_at_hand_display (void *) {

  static	int	 whichUser = 0;
  double	x_loc, y_loc;
  v_xform_type	worldFromHand;
  char message[1000], msgpart[500];

  if (decoration->user_mode == USER_PLANEL_MODE) {	// sharp tip mode
	// Print all of the values in the intputPoint list of values
	Point_value *value = microscope->state.data.inputPoint->head();

	if (value == NULL) {
		sprintf(message,"No point data sets");
	} else {
		sprintf(message,"Point: %s = %5.2f %s",
			value->name()->Characters(),
			value->value(), value->units()->Characters());
		value = value->next();
	}

	while ( value ) {
		sprintf(msgpart, ", %s = %5.2f %s",value->name()->Characters(),
			value->value(), value->units()->Characters());
		strcat(message, msgpart);
		value = value->next();
	}
  } else {
	// Find out where the hand is in x and y right now
	BCPlane* plane = g_inputGrid->getPlaneByName
                      (g_heightPlaneName);
	v_get_world_from_hand(whichUser, &worldFromHand);
	x_loc = worldFromHand.xlate[X];
	y_loc = worldFromHand.xlate[Y];
// 	printf("%f %f\n", x_loc, y_loc);
	if (x_loc < plane->minX()) x_loc = plane->minX();
	if (x_loc > plane->maxX()) x_loc = plane->maxX();
	if (y_loc < plane->minY()) y_loc = plane->minY();
	if (y_loc > plane->maxY()) y_loc = plane->maxY();

	// Find the height of the surface at the current hand location
	if (plane == NULL) {
		sprintf(message,
		"Closest grid height to current hand location is unknown");
	} else {
		sprintf(message,
			"Closest grid height to current hand location is %g",
		plane->valueAt(x_loc,y_loc));
	}
  }

  // Print the message at the appropriate location on the screen
#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
  glPushMatrix();
  glColor3f(1.0f, 1.0f, 1.0f);
  glTranslatef(0.053f, 0.053f, -0.00001f);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
#ifndef FLOW
  drawStringInFont(myfont, message);
#endif
  glPopMatrix(); 
#ifndef FLOW
  glPopAttrib();
#endif

  return(0);
}


int rate_display (void *) {

  char message[1000];

#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
  glPushMatrix();

  switch (g_inputGrid->readMode()) {
    case READ_DEVICE:
      sprintf(message, "Live at (%5d)", decoration->elapsedTime);
      break;
    case READ_STREAM:
      if (decoration->rateOfTime > 1)
        sprintf(message, "Replay:  %d times the original rate at (%5d)",
                decoration->rateOfTime, decoration->elapsedTime);
      else if (decoration->rateOfTime == 1)
        sprintf(message, "Replay: same as the original rate at (%5d)",
			decoration->elapsedTime);
      else
        sprintf(message, "Replay: Freeze-frame at (%5d)",
			decoration->elapsedTime);
      break;
    case READ_FILE: 
      sprintf(message, "Static Image");
      break;
    default:
      sprintf(message, "Unknown read mode");
      break;
  }
  glColor3f(1.0f, 1.0f, 1.0f);
  glTranslatef(0.053f, 0.706f, -0.00001f);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
#ifndef	FLOW
  drawStringInFont(myfont, message);
#endif
  glPopMatrix(); 
#ifndef FLOW
  glPopAttrib();
#endif

  return(0);
}

int screenaxis_display (void *) {

#if 0
  glPushMatrix();

	glScalef(0.1f, 0.1f, 0.001f);	// Medium-sized cube
	mycube();

	glPushMatrix();
		glScalef(0.1f, 0.1f, 1.5f);	// Smaller cube inside */
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);	// See another face of it
		mycube();
	glPopMatrix();

	glTranslatef(10.0f, 0.0f, 0.0f);	// Cube to the right
	mycube();

	glPushMatrix();
		glScalef(0.1f, 0.1f, 1.5f);	// Smaller cube inside */
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);	// See another face of it
		mycube();
	glPopMatrix();

	glTranslatef(-10.0f, 10*V_WORKBENCH_LEFT_SCREEN_HEIGHT/V_WORKBENCH_LEFT_SCREEN_WIDTH, 0.0f);	// Cube above
	mycube();

	glPushMatrix();
		glScalef(0.1f, 0.1f, 1.5f);	// Smaller cube inside */
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);	// See another face of it
		mycube();
	glPopMatrix();

  glPopMatrix(); 
#endif

  return 0;
}

int control_display (void *) {

  char message[1000];

#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
  glPushMatrix();
  glColor3f(1.0f, 1.0f, 1.0f);

  // Translate accidentally deleted some time ago;
  // restored 29 June 98 by TCH

  glTranslatef(0.599f, 0.053f, -0.001f);
  glRasterPos3f(0.0f, 0.0f, 0.0f);

  sprintf(message, 
	  "FmodMax: %g  FmodMin: %g  FmodCur:  %g",
          decoration->modSetpointMax,
          decoration->modSetpointMin,
          decoration->modSetpoint);
  glTranslatef(0.0f, 0.027f, 0.0f);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
#ifndef	FLOW
  drawStringInFont(myfont, message);
#endif
  
  // Used to be amplitude, but we think it should be setpoint
  sprintf(message, 
	  "FimgMax: %g  FimgMin: %g  FimgCur:  %g",
          decoration->imageSetpointMax,
          decoration->imageSetpointMin,
          decoration->imageSetpoint);
  glTranslatef(0.0f, 0.026f, 0.0f);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
#ifndef	FLOW
  drawStringInFont(myfont, message);
#endif

  glPopMatrix(); 
#ifndef FLOW
  glPopAttrib();
#endif

  return(0);
}

int mode_display (void *) {

  char *message;

#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
  glPushMatrix();
  glColor3f(1.0f, 1.0f, 1.0f);

  switch(decoration->user_mode) {
     /* Don't comment any of these out unless the modes are totally 
      * removed from the code. When I commented out "comb mode", 
      * I got a seg-fault, even though the code was supposed to safely 
      * bypass this unused mode. */
  case USER_LIGHT_MODE:       message = (char *)"Light Mode";          break;
  case USER_FLY_MODE:         message = (char *)"Fly Mode";            break;
  case USER_MEASURE_MODE:     message = (char *)"Measure Mode";        break;
     //case USER_PULSE_MODE:       message = (char *)"Pulse Mode";          break;
     //case USER_LINE_MODE:        message = (char *)"Line Mode";           break;
     //case USER_SWEEP_MODE:       message = (char *)"Sweep Mode";          break;
     //case USER_BLUNT_TIP_MODE:   message = (char *)"Blunt Tip Mode";      break;
     //case USER_COMB_MODE:        message = (char *)"Comb Mode";           break;
  case USER_PLANE_MODE:       message = (char *)"Feel From Grid Mode"; break;
  case USER_PLANEL_MODE:      message = (char *)"Feel & Prepare to Modify Mode"; break;
  case USER_SCALE_UP_MODE:    message = (char *)"Scale Up Mode";       break;
  case USER_SCALE_DOWN_MODE:  message = (char *)"Scale Down Mode";     break;
  case USER_SERVO_MODE:       message = (char *)"Select Mode";         break;
  case USER_GRAB_MODE:        message = (char *)"World Grab Mode";     break;
  case USER_MEAS_MOVE_MODE:   message = (char *)"Measure Grid Grab Mode"; break;
  case USER_CENTER_TEXTURE_MODE:
			      message = (char *)"Center Texture Mode"; break;
  case USER_SCANLINE_MODE:    message = (char *)"LineScan Mode"; break;
  default:                    message = (char *)"Unknown Mode"; break;
  }

  glTranslatef(0.530f, 0.706f, -0.00001f);

  glRasterPos3f(0.0f, 0.0f, 0.0f);
#ifndef	FLOW
  drawStringInFont(myfont, message);
#endif

  glPopMatrix(); 
#ifndef FLOW
  glPopAttrib();
#endif

  return(0);
}




int measure_display (void *) {

   char message[1000];

   BCPlane* plane = g_inputGrid->getPlaneByName
                      (g_heightPlaneName);
   if (plane == NULL)
   {
       fprintf(stderr, "Error in doMeasure: could not get plane!\n");
       return -1;
   }

   measure_data = calculate(decoration->red.top(),
                            decoration->green.top(),
                            decoration->blue.top());

   // WARNING
   // plane->valueAt() doesn't do bounds checking, so if a decoration
   // line has been moved outside the bounds of a plane it'll crash
   // mysteriously here.

   measure_data.rgDz =
         plane->valueAt(decoration->red.x(), decoration->red.y()) -
         plane->valueAt(decoration->green.x(), decoration->green.y());
   measure_data.gbDz =
         plane->valueAt(decoration->green.x(), decoration->green.y()) -
         plane->valueAt(decoration->blue.x(), decoration->blue.y());
   measure_data.brDz =
         plane->valueAt(decoration->blue.x(), decoration->blue.y()) -
         plane->valueAt(decoration->red.x(), decoration->red.y());

#ifndef FLOW
  glPushAttrib(GL_CURRENT_BIT);
#endif
   glPushMatrix();
 

   glTranslatef(0.575f, 0.695f, -0.00001f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);

   glLineWidth(3.0);

   glBegin(GL_LINES);
   VERBOSE(20, "          glBegin(GL_LINES)");
     glColor3f(1.0f, 0.0f, 0.0f);
       glVertex3f(0.0f, 0.0f, 0.0f);
       glVertex3f(0.0f, -0.037f, 0.0f);
   VERBOSE(20, "          glEnd()");
   glEnd();

   glColor3f(1.0f, 1.0f, 1.0f);
   glTranslatef(0.016f, -0.01f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   sprintf(message,"Dxy=%g",measure_data.rgDxy);
#ifndef FLOW
   drawStringInFont(myfont,message);
#endif

   glTranslatef(0.0f, -0.023f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   sprintf(message,"Dz =%g",measure_data.rgDz);
#ifndef FLOW
   drawStringInFont(myfont,message);
#endif

   glTranslatef(0.1f, 0.033f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   glBegin(GL_LINES);
   VERBOSE(20, "          glBegin(GL_LINES)");
     glColor3f(0.0f, 1.0f, 0.0f);
     glVertex3f(0.0f, 0.0f, 0.0f);
     glVertex3f(0.0f, -0.037f, 0.0f);
   VERBOSE(20, "          glEnd()");
   glEnd();
 
   glColor3f(1.0f, 1.0f, 1.0f);
   glTranslatef(0.016f, -0.01f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   sprintf(message,"Dxy=%g",measure_data.gbDxy);
#ifndef FLOW
   drawStringInFont(myfont,message);
#endif

   glTranslatef(0.0f, -0.023f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   sprintf(message,"Dz =%g",measure_data.gbDz);
#ifndef FLOW
   drawStringInFont(myfont,message);
#endif

   glTranslatef(0.1f, 0.033f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   glBegin(GL_LINES);
   VERBOSE(20, "          glBegin(GL_LINES)");
     glColor3f(0.0f, 0.0f, 1.0f);
     glVertex3f(0.0f, 0.0f, 0.0f);
     glVertex3f(0.0f, -0.037f, 0.0f);
   VERBOSE(20, "          glEnd()");
   glEnd();
 
   
   glColor3f(1.0f, 1.0f, 1.0f);
   glTranslatef(0.016f, -0.01f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   sprintf(message,"Dxy=%g",measure_data.brDxy);
#ifndef FLOW
   drawStringInFont(myfont,message);
#endif

   glTranslatef(0.0f,-0.023f,0.0f);
   glRasterPos3f(0.0f,0.0f,0.0f);
   sprintf(message,"Dz =%g",measure_data.brDz);
#ifndef FLOW
   drawStringInFont(myfont,message);
#endif


   glTranslatef(0.1f, 0.033f,0.0f);
   glRasterPos3f(0.0f,0.0f,0.0f);
   glBegin(GL_LINES);
   VERBOSE(20, "          glBegin(GL_LINES)");
     glColor3f(1.0f,0.0f,0.0f);
     glVertex3f(0.0f,0.0f,0.0f);
     glVertex3f(0.0f,-0.037f,0.0f);
   VERBOSE(20, "          glEnd()");
   glEnd();

   glLineWidth(1.0); 

   glColor3f(1.0f,1.0f,1.0f);
   glTranslatef(-0.321f,-0.059f,0.0f);
   glRasterPos3f(0.0f,0.0f,0.0f);
   sprintf(message,"Angle: %g",measure_data.rgbAngle);
#ifndef FLOW
   drawStringInFont(myfont,message);
#endif

   glTranslatef(0.190f,0.0f,0.0f);
   glRasterPos3f(0.0f,0.0f,0.0f);
   sprintf(message,"%g",measure_data.gbrAngle);
#ifndef FLOW
   drawStringInFont(myfont,message);
#endif

   glPopMatrix();
#ifndef FLOW
  glPopAttrib();
#endif

   return(0);
}


int addChartjunk (nmg_Funclist ** v_screen, float * screen_scale) {

  fprintf(stderr, "Adding chartjunk to screen\n");

  addFunctionToFunclist(v_screen, screenaxis_display, NULL,
                        "screenaxis_display");
  // rate_id =
  addFunctionToFunclist(v_screen, rate_display, NULL, "rate_display");
  // mode_id =
  addFunctionToFunclist(v_screen, mode_display, NULL, "mode_display");
  // measure_id =
  addFunctionToFunclist(v_screen, measure_display, NULL, "measure_display");
  addFunctionToFunclist(v_screen, scale_display, screen_scale, "scale_display");
  addFunctionToFunclist(v_screen, x_y_width_display, NULL, "x_y_width_display");
  // scale_id =
  addFunctionToFunclist(v_screen, height_at_hand_display, NULL,
                        "height_at_hand_display");
  // control_id =
  addFunctionToFunclist(v_screen, control_display, NULL, "control_display");

  return 0;
}

void initializeChartjunk (const char * name) {

  // Build display list for the labels

  myfont = loadFont(name);
  if (!myfont) {
    fprintf(stderr,"ERROR: Could not load font.\n");
    dataset->done = 1;
  }
}

