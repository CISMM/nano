/* nm_Tip.h : deals with visualization of the AFM tip 
 The idea here is that we've potentially got multiple sources of information
 about where the tip is and what it looks like. The AFM gives us a 
 point location based on what scan voltages it has applied to the piezos.
 The SEM in the combined AFM/SEM system gives us an image that can include
 both the tip and the surface.
 We have prior knowledge about the size and shape of AFM tips and tip
 characterization techniques can give us more detailed shape information.

 Currently, the user interface displays a tip-shaped icon at the location
 of the user's hand when the tip position is being controlled manually.
 This picture isn't ideal for visualizing the true tip location due to 
 stage drift and time delay.
 
 We'd like to put all this together in a visualization that displays a
 polyhedral model of the tip at our best estimate of its current position 
 relative to acquired AFM data or some other model of the surface
 as the user controls the tip location.
*/

#ifndef NM_TIP_H
#define NM_TIP_H

#include <vrpn_Types.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <UTree.h>
#include <URender.h>

#include <nmm_MicroscopeRemote.h>

#include <GL/glut_UNC.h>

/* nm_TipModel:
   contains all state that we are currently using to describe the tip
*/
class nm_TipModel {
 friend class nm_TipRenderer;
 public:
  nm_TipModel();
  ~nm_TipModel();

  // get/set the position of the tip
  void setPosition(double x, double y, double z);
  void getPosition(double &x, double &y, double &z);

 private:
  double d_pos[3];

  // surface model (as from AFM tip characterizer)
  nmb_Image *d_tipSurface;
  // rotation of tipSurface relative to current surface being acquired
  double d_rotation_deg;
  double d_offset[3]; // offset to get from corner of image to tip apex

  // image of tip (as from SEM) along with projection matrix used to display
  // the image as a projective texture
  nmb_Image *d_projectionImage;
  double d_projectionMatrix[16];
  // optimization - keeps track of how many times d_projectionImage has
  // been set so that rendering code knows when it doesn't need to update the
  // texture
  int d_projImUpdateCount;

  // cone sphere approximation (available as default)
  double d_radius_nm;
  double d_coneAngle_deg;
  double d_totalHeight_nm;
};

/* nm_TipRenderer:
   encapsulates translation between model and openGL
*/
class nm_TipRenderer : public URender {
 public:
  nm_TipRenderer(nm_TipModel *tipModel);
  virtual ~nm_TipRenderer();
 
  void buildDisplayList();

  // draw the tip to openGL
  virtual int Render(void *userdata);

  void setTextureEnable(bool enable);

 private:
  bool d_drawConeSphere;
  bool d_drawSurface;
  bool d_drawTexture;
  nm_TipModel *d_tipModel;

  int d_lastInstallProjImCount;
  GLuint d_displayListID;
  GLUquadricObj *d_quadric;
};

/* nm_TipDisplayControls:
   connects Tcl code with the nm_TipModel object and manages interactions
   with the nmUGraphics code 
*/
class nm_TipDisplayControls {
 public:
  // installs nm_TipRenderer object
  nm_TipDisplayControls(nmm_Microscope_Remote *scope = NULL);
  // removes nm_TipRenderer object from UGraphics rendering tree
  ~nm_TipDisplayControls();
  void setSPM(nmm_Microscope_Remote *scope);
  void setDisplayEnable(int enable);
  void setTextureEnable(int enable);

 private:
  int pointDataHandler(const Point_results *pr);
  static int pointDataHandler(void *ud, const Point_results *pr);
  static void handleEnableDisplayChange(vrpn_int32 newval, void *userdata);
  static void handleEnableTextureChange(vrpn_int32 newval, void *userdata);

  // control panel variables
  Tclvar_int d_enableDisplay;
  Tclvar_int d_enableTexture;

  // data for tip model and graphics display
  nm_TipModel d_tipModel;
  nm_TipRenderer d_tipRenderer;
  static char *s_renderName;

  // AFM
  nmm_Microscope_Remote *d_AFM;
};

#endif
