#include "RobotControl.h"

#include "microscape.h"

#include "nmb_Image.h"

#include "vrpn_Types.h"

#include "Point.h"

ImageViewer *g_iViewer(NULL);

int g_iViewerWin(-1);

vrpn_bool g_activeControl(vrpn_false),
     g_displayPlan(vrpn_true);

typedef enum { SCAN_MODE, IMAGE_MODE, MODIFY_MODE } ScopeMode;

ScopeMode g_activeMode(SCAN_MODE);

nmb_Dataset *g_dataset(NULL);

nmm_Microscope_Remote *g_microscope(NULL);


#ifndef V_GLUT

#define GLUT_LEFT_BUTTON  0x0
#define GLUT_RIGHT_BUTTON 0x1
#define GLUT_UP           0x0
#define GLUT_DOWN         0x1

#endif

#include "warrencntsim.C"


int RobotControl::eventHandler(
    const ImageViewerWindowEvent &event,
    void * /*ud*/)
{
   switch(event.type)
   {
      case RESIZE_EVENT:
         reshapeWindowFuncMain(event.width, event.height);
         break;

      case BUTTON_PRESS_EVENT:
         if (g_displayPlan)
            if (event.button == IV_LEFT_BUTTON)
               mouseFuncMain(GLUT_LEFT_BUTTON, GLUT_DOWN, event.mouse_x, event.mouse_y);
         break;

      case MOTION_EVENT:
         if (g_displayPlan)
            if (event.state & IV_LEFT_BUTTON_MASK)
               mouseMotionFuncMain(event.mouse_x, event.mouse_y);
         break;

      case KEY_PRESS_EVENT:
         commonKeyboardFunc(event.keycode, event.mouse_x, event.mouse_y);
         break;

      default:
         break;
   }

   return 0;
}

int RobotControl::displayHandler(
    const ImageViewerDisplayData & /*data*/,
    void * /*ud*/)
{
   if (g_displayPlan)
      displayFuncMain();
   else
      displayFuncView();
   return 0;
}

RobotControl::RobotControl(nmm_Microscope_Remote *microscope, nmb_Dataset *dataset)
{
   g_microscope = microscope;
   g_dataset = dataset;
   g_iViewer = ImageViewer::getImageViewer();

   char *display_name;
   display_name = (char *)getenv("V_X_DISPLAY");
   if (!display_name)
      display_name = (char *)getenv("DISPLAY");
   if (!display_name)
      display_name = strdup("unix:0");
   else
      display_name = strdup(display_name);

   g_iViewer->init(display_name);

   g_iViewerWin = g_iViewer->createWindow(display_name, 0, 0, windowWidth, windowHeight, "Robot Control");

   g_iViewer->setWindowEventHandler(g_iViewerWin, RobotControl::eventHandler, (void *)this);
   g_iViewer->setWindowDisplayHandler(g_iViewerWin, RobotControl::displayHandler, (void *)this);

   initObs();

   updateLiveData();
}

void RobotControl::mainloop(void)
{
   if (g_iViewer)
   {
      g_iViewer->mainloop();

      if (robotRunning || robotImgPhase || robotModPhase)
         g_iViewer->dirtyWindow(g_iViewerWin);
      else
         updateLiveData();
   }
}

void RobotControl::show(void)
{
   g_iViewer->showWindow(g_iViewerWin);
}

void RobotControl::hide(void)
{
   g_iViewer->hideWindow(g_iViewerWin);
}
