
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
#include <nmm_MicroscopeRemote.h>

//#include "nmg_GraphicsImpl.h"
//#include "nmg_Globals.h" // for graphics pointer.
#include "graphics_globals.h"  // for VERBOSE
#include "font.h"  // drawStringInFont()
#include "nmg_Funclist.h"
#include "graphics.h"  // for getViewportSize



#define ARROW_SCALE  0.005
#define xx .525731112119133606
#define zz .850650808352039932
#define SPHERE_DEPTH 2

// M_PI not defined for VC++, for some reason. 
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

// Margin that moves text out from the edge of the screen. 
const float MARGIN = 0.02f;
// Spacing of one line to the next.
const float LINE_SPACE = 0.027f; 
// Back-off from the screen in Z to avoid clipping
const float Z_SLIVER = -0.00001f;

// Top edge of the screen is scaled to be .75 in vlib somewhere
const float TOP_EDGE = 0.75f;
// right edge is scaled to 1.0
const float RIGHT_EDGE = 1.0f;

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

  data.rgDxy = sqrt(rg[X]*rg[X] + rg[Y]*rg[Y]);
  data.gbDxy = sqrt(gb[X]*gb[X] + gb[Y]*gb[Y]);
  data.brDxy = sqrt(br[X]*br[X] + br[Y]*br[Y]);

  if ((data.rgDxy == 0) || (data.gbDxy == 0) || (data.brDxy == 0)) {

    //If any of the two points overlap, angles are set to 0
    data.rgbAngle = 0;
    data.gbrAngle = 0;
    data.brgAngle = 0;

  } else {

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

  char *message = NULL;
  glPushAttrib(GL_CURRENT_BIT);
  glPushMatrix();

  glColor3f(1.0f, 1.0f, 1.0f);
  glScalef(*size, *size, *size);
  glTranslatef(MARGIN, 2*LINE_SPACE + MARGIN, Z_SLIVER);

  double scale = 1.0;

//fprintf(stderr, "scale_display:  height plane %s.\n", g_heightPlaneName);

  BCPlane * plane = g_inputGrid->getPlaneByName
              (g_heightPlaneName);
  if (plane == NULL) {
      fprintf(stderr, "Error in scale_display: could not get plane!\n");
  } else {
      scale = plane->scale();
      // get a reasonable estimate of the maximum size string we'll need
      // and allocate the buffer
      int mess_len = strlen(plane->units()->Characters()) + 
		strlen(g_heightPlaneName) + 100;
      message = new char[mess_len];
      if (message != NULL) {
          sprintf(message,"Displaying %s (%s), scale x%g",
                  g_heightPlaneName,
                  plane->units()->Characters(), scale);
      } else {
          fprintf(stderr, "Error: scale_display: out of memory\n");
      }
  }
  glRasterPos3f(0.0f, 0.0f, 0.0f);
  if (message != NULL) {
    drawStringInFont(myfont, message);
    delete [] message;
  }
  glPopMatrix(); 
  glPopAttrib();

  return(0);
}


int x_y_width_display (void *) {

  char message[100];

  sprintf(message,"%dx%d grid, %g by %g nm",
          g_inputGrid->numX(), g_inputGrid->numY(), 
          decoration->selectedRegionMaxX - decoration->selectedRegionMinX,
          decoration->selectedRegionMaxY - decoration->selectedRegionMinY);

  // Print the message at the appropriate location on the screen
  glPushAttrib(GL_CURRENT_BIT);
  glPushMatrix();
  glColor3f(1.0f, 1.0f, 1.0f);
  glTranslatef(MARGIN, LINE_SPACE + MARGIN,Z_SLIVER);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
  drawStringInFont(myfont, message);
  glPopMatrix(); 
  glPopAttrib();

  return(0);
}


int height_at_hand_display (void *) {

  static	int	 whichUser = 0;
  double	x_loc, y_loc;
  v_xform_type	worldFromHand;
  char message[1000], msgpart[500];

  if (g_user_mode == USER_PLANEL_MODE) {	// sharp tip mode
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
		"Height under hand is unknown");
	} else {
            double result;
            plane->valueAt(&result, x_loc,y_loc);
		sprintf(message,
			"Height under hand is %g",
                        result);
        }
  }

  // Print the message at the appropriate location on the screen
  glPushAttrib(GL_CURRENT_BIT);
  glPushMatrix();
  glColor3f(1.0f, 1.0f, 1.0f);
  glTranslatef(MARGIN, MARGIN, Z_SLIVER);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
  drawStringInFont(myfont, message);
  glPopMatrix(); 
  glPopAttrib();

  return(0);
}


int rate_display (void *) {

  char message[100];

  glPushAttrib(GL_CURRENT_BIT);
  glPushMatrix();

  switch (g_inputGrid->readMode()) {
    case READ_DEVICE:
      sprintf(message, "Live (%5ld sec)", decoration->elapsedTime);
      break;
    case READ_STREAM:
      if (decoration->rateOfTime > 1)
        sprintf(message, "Play %dx (%5ld sec)",
                decoration->rateOfTime, decoration->elapsedTime);
      else if (decoration->rateOfTime == 1)
        sprintf(message, "Play (%5ld sec)",
			decoration->elapsedTime);
      else
        sprintf(message, "Pause (%5ld sec)",
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
  glTranslatef(MARGIN, TOP_EDGE - MARGIN , Z_SLIVER);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
  drawStringInFont(myfont, message);
  glPopMatrix(); 
  glPopAttrib();

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

//fprintf(stderr, " <cd> in;  graphics is %d\n", graphics);

  char message[100];
  int w,h;

/*
  if (graphics) {
    graphics->getViewportSize(&w, &h);
  } else {
    fprintf(stderr, "NULL graphics pointer;  assuming 1024x768.\n");
    w = 1024;
    h = 768;
  }
*/

  // TCH 18 Feb 2001
  getViewportSize(&w, &h);

//fprintf(stderr, " <cd> got size\n");

  // Total screen is 1.0f across, but font width is in pixels. Convert. 
  float char_width = RIGHT_EDGE * getFontWidth()/ w;
  glPushAttrib(GL_CURRENT_BIT);
  glPushMatrix();
  glColor3f(1.0f, 1.0f, 1.0f);


//fprintf(stderr, " <cd> modify setpoint\n");

  sprintf(message, 
	  "Modify setpoint: %.3g",
          decoration->modSetpoint);
  // Find out length of message:
  float msg_width = char_width * strlen(message);
  // Need a bit bigger margin on the right edge. 
  glTranslatef(RIGHT_EDGE - (3*MARGIN + msg_width), MARGIN, Z_SLIVER);
  //printf ("CD %f %f\n", char_width, msg_width);
  // Leave space for the Point result display to spill over 
  // to right side of screen.
  glTranslatef(0.0f, LINE_SPACE, 0.0f);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
  drawStringInFont(myfont, message);
  
//fprintf(stderr, " <cd> image setpoint\n");

  // Used to be amplitude, but we think it should be setpoint
  // Added extra space so it's the same length as modify message above.
  sprintf(message, 
	  "Image setpoint : %.3g",
          decoration->imageSetpoint);
  glTranslatef(0.0f, LINE_SPACE, 0.0f);
  glRasterPos3f(0.0f, 0.0f, 0.0f);
  drawStringInFont(myfont, message);

  glPopMatrix(); 
  glPopAttrib();

//fprintf(stderr, " <cd> out\n");

  return(0);
}

int mode_display (void *) {

  char *message;

  glPushAttrib(GL_CURRENT_BIT);
  glPushMatrix();
  glColor3f(1.0f, 1.0f, 1.0f);

  switch(g_user_mode) {
     /* Don't comment any of these out unless the modes are totally 
      * removed from the code. When I commented out "comb mode", 
      * I got a seg-fault, even though the code was supposed to safely 
      * bypass this unused mode. */
  case USER_LIGHT_MODE:       message = (char *)"Position Light Mode"; break;
  case USER_FLY_MODE:         message = (char *)"Fly Mode";            break;
  case USER_MEASURE_MODE:     message = (char *)"Measure Mode";        break;
     //case USER_PULSE_MODE:       message = (char *)"Pulse Mode";          break;
     //case USER_LINE_MODE:        message = (char *)"Line Mode";           break;
     //case USER_SWEEP_MODE:       message = (char *)"Sweep Mode";          break;
     //case USER_BLUNT_TIP_MODE:   message = (char *)"Blunt Tip Mode";      break;
     //case USER_COMB_MODE:        message = (char *)"Comb Mode";           break;
  case USER_PLANE_MODE:       message = (char *)"Touch Stored Mode"; break;
  case USER_PLANEL_MODE:      message = (char *)"Touch & Prepare to Modify Mode"; break;
  case USER_SCALE_UP_MODE:    message = (char *)"Scale Up Mode";       break;
  case USER_SCALE_DOWN_MODE:  message = (char *)"Scale Down Mode";     break;
  case USER_SERVO_MODE:       message = (char *)"Select Mode";         break;
  case USER_GRAB_MODE:        message = (char *)"Grab Mode";     break;
  case USER_MEAS_MOVE_MODE:   message = (char *)"Measure Line Mode"; break;
  case USER_CENTER_TEXTURE_MODE:
			      message = (char *)"Center Texture Mode"; break;
  case USER_SCANLINE_MODE:    message = (char *)"LineScan Mode"; break;
  case USER_REGION_MODE:      message = (char *)"Region Select Mode"; break;
  default:                    message = (char *)"Unknown Mode"; break;
  }

  // display in center top.
  glTranslatef(0.5f*RIGHT_EDGE, TOP_EDGE - MARGIN, Z_SLIVER);

  glRasterPos3f(0.0f, 0.0f, 0.0f);
  drawStringInFont(myfont, message);

  glPopMatrix(); 
  glPopAttrib();

  return(0);
}




int measure_display (void *) {

   char message[100];

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
         decoration->red.getIntercept(plane) -
         decoration->green.getIntercept(plane);
   measure_data.gbDz =
         decoration->green.getIntercept(plane) -
         decoration->blue.getIntercept(plane);
   measure_data.brDz =
         decoration->blue.getIntercept(plane) -
         decoration->red.getIntercept(plane);

  glPushAttrib(GL_CURRENT_BIT);
   glPushMatrix();
 

   glTranslatef(0.575f, TOP_EDGE - (MARGIN + 0.5*LINE_SPACE), Z_SLIVER);
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
   sprintf(message,"Dxy=%.5g",measure_data.rgDxy);
   drawStringInFont(myfont,message);

   glTranslatef(0.0f, -0.023f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   sprintf(message,"Dz =%.5g",measure_data.rgDz);
   drawStringInFont(myfont,message);

   glTranslatef(0.1f, 0.033f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   glBegin(GL_LINES);
   VERBOSE(20, "          glBegin(GL_LINES)");
   // It's yellow, not green, to avoid red-green colorblind problems.
     glColor3f(0.7f, 0.7f, 0.0f);
     glVertex3f(0.0f, 0.0f, 0.0f);
     glVertex3f(0.0f, -0.037f, 0.0f);
   VERBOSE(20, "          glEnd()");
   glEnd();
 
   glColor3f(1.0f, 1.0f, 1.0f);
   glTranslatef(0.016f, -0.01f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   sprintf(message,"Dxy=%.5g",measure_data.gbDxy);
   drawStringInFont(myfont,message);

   glTranslatef(0.0f, -0.023f, 0.0f);
   glRasterPos3f(0.0f, 0.0f, 0.0f);
   sprintf(message,"Dz =%.5g",measure_data.gbDz);
   drawStringInFont(myfont,message);

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
   sprintf(message,"Dxy=%.5g",measure_data.brDxy);
   drawStringInFont(myfont,message);

   glTranslatef(0.0f,-0.023f,0.0f);
   glRasterPos3f(0.0f,0.0f,0.0f);
   sprintf(message,"Dz =%.5g",measure_data.brDz);
   drawStringInFont(myfont,message);


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
   sprintf(message,"Angle:  %.4g",measure_data.rgbAngle);
   drawStringInFont(myfont,message);

   glTranslatef(0.192f,0.0f,0.0f);
   glRasterPos3f(0.0f,0.0f,0.0f);
   sprintf(message,"%.4g",measure_data.gbrAngle);
   drawStringInFont(myfont,message);

   glPopMatrix();
   glPopAttrib();

   return(0);
}


int addChartjunk (nmg_Funclist ** v_screen, float * screen_scale) {

    //fprintf(stderr, "Adding chartjunk to screen\n");

//    addFunctionToFunclist(v_screen, screenaxis_display, NULL,
//                          "screenaxis_display");
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

