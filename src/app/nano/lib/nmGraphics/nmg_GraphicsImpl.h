/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMG_GRAPHICS_IMPL_H
#define NMG_GRAPHICS_IMPL_H

#include "nmg_Graphics.h"
#include "PPM.h"

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM

class nmg_Graphics_Implementation : public nmg_Graphics {

  public:

    nmg_Graphics_Implementation (nmb_Dataset * data,
                                 const int minColor [3],
                                 const int maxColor [3],
                                 const char * rulergridName = NULL,
                                 vrpn_Connection * = NULL,
                                 unsigned int portNum = 4503);

    virtual ~nmg_Graphics_Implementation (void);

    virtual void mainloop (void);

    virtual void changeDataset( nmb_Dataset * data);
    virtual void resizeViewport(int width, int height);
    virtual void getViewportSize(int *width, int * height);

    virtual void loadRulergridImage (const char *);

    virtual void causeGridReColor (void);
    virtual void causeGridRedraw (void);
    virtual void causeGridRebuild (void);

    virtual void enableChartjunk (int);
    virtual void enableFilledPolygons (int);
    virtual void enableSmoothShading (int);
    virtual void enableTrueTip (int);

    virtual void setAdhesionSliderRange (float low, float hi);

    virtual void setAlphaColor (float r, float g, float b);
    virtual void setAlphaSliderRange (float low, float hi);

    virtual void setBumpMapName (const char *);

    virtual void setColorMapDirectory (const char *);
    virtual void setColorMapName (const char *);
    virtual void setColorMinMax (float low, float hi);
    virtual void setDataColorMinMax (float low, float hi);

    virtual void setTextureDirectory (const char *);

    virtual void setComplianceSliderRange (float low, float hi);

    virtual void setContourColor (int r, int g, int b);
    virtual void setContourWidth (float);

    virtual void setFrictionSliderRange (float low, float hi);
    virtual void setBumpSliderRange (float low, float hi);
    virtual void setBuzzSliderRange (float low, float hi);

    virtual void setHandColor (int);
    virtual void setHatchMapName (const char *);  // RENAME?

    virtual void setAlphaPlaneName (const char *);
    virtual void setColorPlaneName (const char *);
    virtual void setContourPlaneName (const char *);
    virtual void setHeightPlaneName (const char *);

    virtual void setIconScale (float);

    virtual void enableCollabHand (vrpn_bool);
    virtual void setCollabHandPos (double [3], double [4]);
    virtual void setCollabMode (int);

    // arguments in range [0..1]
    virtual void setMinColor (const double [3]);
    virtual void setMaxColor (const double [3]);
    // arguments in range [0..255]
    virtual void setMinColor (const int [3]);
    virtual void setMaxColor (const int [3]);

    virtual void setPatternMapName (const char *);  // RENAME?

    // Realigning Textures:
    virtual void createRealignTextures( const char * );
    virtual void setRealignTextureSliderRange (float, float);
    virtual void setRealignTexturesConversionMap( const char *, const char * );
    virtual void computeRealignPlane( const char *, const char * );

//    virtual void enableRealignTextures (int on);
    virtual void translateTextures ( int on, float dx, float dy );
    virtual void scaleTextures ( int on, float dx, float dy );
    virtual void shearTextures ( int on, float dx, float dy );
    virtual void rotateTextures ( int on, float theta );
    virtual void setTextureCenter( float dx, float dy );

    virtual void updateTexture(int which, const char *image_name,
       int start_x, int start_y,
       int end_x, int end_y);
 
    // helper for updateTexture
    virtual void loadRawDataTexture(const int which, const char *image_name,
	const int start_x, const int start_y);

//    virtual void enableRegistration (int on);
    virtual void setTextureTransform(double *xform);

//    virtual void enableRulergrid (int);
    virtual void setRulergridAngle (float);
    virtual void setRulergridColor (int r, int g, int b);
    virtual void setRulergridOffset (float x, float y);
    virtual void setRulergridOpacity (float alpha);
    virtual void setRulergridScale (float);
    virtual void setRulergridWidths (float x, float y);

    virtual void setSpecularity (int);
    virtual void setSpecularColor (float);
    virtual void setDiffusePercent (float);
    virtual void setSurfaceAlpha (float);
    virtual void setSphereScale (float);

    virtual void setTesselationStride (int);

    virtual void setTextureMode (TextureMode, 
		TextureTransformMode = RULERGRID_COORD);
    virtual void setTextureScale (float);
    virtual void setTrueTipScale (float);

    virtual void setUserMode (int oldMode, int oldStyle, int newMode, int style);

    virtual void setLightDirection (q_vec_type &);
    virtual void resetLightDirection (void);

    virtual int addPolylinePoint (const float [2][3]);
    virtual void emptyPolyline (void);

    virtual void setRubberLineStart (float, float);
    virtual void setRubberLineEnd (float, float);
    virtual void setRubberLineStart (const float [2]);
    virtual void setRubberLineEnd (const float [2]);

    virtual void setScanlineEndpoints(const float [3], const float [3]);
    virtual void displayScanlinePosition(const int);

    virtual void positionAimLine (const PointType, const PointType);
    virtual void positionRubberCorner (float, float, float, float);
    virtual void positionSweepLine (const PointType, const PointType,
				    const PointType, const PointType);
    virtual int addPolySweepPoints (const PointType, const PointType,
				    const PointType, const PointType);
    virtual void setRubberSweepLineStart (const PointType, const PointType);
    virtual void setRubberSweepLineEnd (const PointType, const PointType);


    virtual void positionSphere (float, float, float);

    virtual void setViewTransform (v_xform_type);
    virtual void createScreenImage(const char *filename, const ImageType type);

    // ACCESSORS

    virtual void getDisplayPosition (q_vec_type &ll, q_vec_type &ul,
                                                        q_vec_type &ur);
    virtual void getLightDirection (q_vec_type *) const;
    virtual int getHandColor (void) const;
    virtual int getSpecularity (void) const;
//  virtual float getDiffusePercent (void) const;
    virtual const double * getMinColor (void) const;
    virtual const double * getMaxColor (void) const;

  protected:

    virtual void initDisplays (void);
      // Making constructor more flexible:  calls v_open_display()
      // for all users (in [0, NUM_USERS)).  Default behavior is to
      // open V_ENV_DISPLAY, but this can be overridden by derived
      // classes.
      // Wouldn't it have been simpler just to make sure other types
      // of graphics implementations redefine the relevant environment
      // variable?

    virtual void initializeTextures (void);
    virtual void makeAndInstallRulerImage(PPM *myPPM);

    // initializes all texture objects

    void screenCapture (int * w, int * h, unsigned char ** pixels,
                        vrpn_bool captureBack = VRPN_TRUE);
    void depthCapture (int * w, int * h, float ** depths,
                        vrpn_bool captureBack = VRPN_TRUE);
      // If (*pxiels) or (*depths) is non-NULL, assumes it is a
      // properly-sized array and writes into it;  otherwise news
      // a w*h*3 or w*h array respectively.
      // By default captures from the back buffer, but can be set to
      // capture from the front buffer instead.

    nmb_Dataset * d_dataset;

    void getLatestGridChange (int * minX, int * maxX, int * minY, int * maxY);

  private:
    int grid_size_x;
    int grid_size_y;

    v_index * d_displayIndexList;

    TextureTransformMode d_textureTransformMode;

    static int handle_resizeViewport (void *, vrpn_HANDLERPARAM);
    static int handle_loadRulergridImage (void *, vrpn_HANDLERPARAM);
    static int handle_causeGridRedraw (void *, vrpn_HANDLERPARAM);
    static int handle_causeGridRebuild (void *, vrpn_HANDLERPARAM);
    static int handle_enableChartjunk (void *, vrpn_HANDLERPARAM);
    static int handle_enableFilledPolygons (void *, vrpn_HANDLERPARAM);
    static int handle_enableSmoothShading (void *, vrpn_HANDLERPARAM);
    static int handle_enableTrueTip (void *, vrpn_HANDLERPARAM);
    static int handle_setAdhesionSliderRange (void *, vrpn_HANDLERPARAM);
    static int handle_setAlphaColor (void *, vrpn_HANDLERPARAM);
    static int handle_setAlphaSliderRange (void *, vrpn_HANDLERPARAM);
    static int handle_setBumpMapName (void *, vrpn_HANDLERPARAM);
    static int handle_setColorMapDirectory (void *, vrpn_HANDLERPARAM);
    static int handle_setColorMapName (void *, vrpn_HANDLERPARAM);
    static int handle_setColorMinMax (void *, vrpn_HANDLERPARAM);
    static int handle_setDataColorMinMax (void *, vrpn_HANDLERPARAM);
    static int handle_setTextureDirectory (void *, vrpn_HANDLERPARAM);
    static int handle_setComplianceSliderRange (void *, vrpn_HANDLERPARAM);
    static int handle_setContourColor (void *, vrpn_HANDLERPARAM);
    static int handle_setContourWidth (void *, vrpn_HANDLERPARAM);
    static int handle_setFrictionSliderRange (void *, vrpn_HANDLERPARAM);
    static int handle_setBumpSliderRange (void *, vrpn_HANDLERPARAM);
    static int handle_setBuzzSliderRange (void *, vrpn_HANDLERPARAM);
    static int handle_setHandColor (void *, vrpn_HANDLERPARAM);
    static int handle_setHatchMapName (void *, vrpn_HANDLERPARAM);
    static int handle_setAlphaPlaneName (void *, vrpn_HANDLERPARAM);
    static int handle_setColorPlaneName (void *, vrpn_HANDLERPARAM);
    static int handle_setContourPlaneName (void *, vrpn_HANDLERPARAM);
    static int handle_setHeightPlaneName (void *, vrpn_HANDLERPARAM);
    static int handle_setMinColor (void *, vrpn_HANDLERPARAM);
    static int handle_setMaxColor (void *, vrpn_HANDLERPARAM);
    static int handle_setPatternMapName (void *, vrpn_HANDLERPARAM);
//    static int handle_enableRulergrid (void *, vrpn_HANDLERPARAM);
    static int handle_setRulergridAngle (void *, vrpn_HANDLERPARAM);
    static int handle_setRulergridColor (void *, vrpn_HANDLERPARAM);
    static int handle_setRulergridOffset (void *, vrpn_HANDLERPARAM);
    static int handle_setRulergridOpacity (void *, vrpn_HANDLERPARAM);
    static int handle_setRulergridScale (void *, vrpn_HANDLERPARAM);
    static int handle_setRulergridWidths (void *, vrpn_HANDLERPARAM);
    static int handle_setSpecularity (void *, vrpn_HANDLERPARAM);
    static int handle_setSpecularColor (void *, vrpn_HANDLERPARAM);
    static int handle_setDiffusePercent (void *, vrpn_HANDLERPARAM);
    static int handle_setSurfaceAlpha (void *, vrpn_HANDLERPARAM);
    static int handle_setSphereScale (void *, vrpn_HANDLERPARAM);
    static int handle_setTesselationStride (void *, vrpn_HANDLERPARAM);
    static int handle_setTextureMode (void *, vrpn_HANDLERPARAM);
    static int handle_setTextureScale (void *, vrpn_HANDLERPARAM);
    static int handle_setTrueTipScale (void *, vrpn_HANDLERPARAM);
    static int handle_setUserMode (void *, vrpn_HANDLERPARAM);
    static int handle_setLightDirection (void *, vrpn_HANDLERPARAM);
    static int handle_resetLightDirection (void *, vrpn_HANDLERPARAM);
    static int handle_addPolylinePoint (void *, vrpn_HANDLERPARAM);
    static int handle_addPolySweepPoints (void *, vrpn_HANDLERPARAM);
    static int handle_emptyPolyline (void *, vrpn_HANDLERPARAM);
    static int handle_setRubberLineStart (void *, vrpn_HANDLERPARAM);
    static int handle_setRubberLineEnd (void *, vrpn_HANDLERPARAM);
    static int handle_setRubberSweepLineStart (void *, vrpn_HANDLERPARAM);
    static int handle_setRubberSweepLineEnd (void *, vrpn_HANDLERPARAM);
    static int handle_setScanlineEndpoints(void *, vrpn_HANDLERPARAM);
    static int handle_displayScanlinePosition(void *, vrpn_HANDLERPARAM);
    static int handle_positionAimLine (void *, vrpn_HANDLERPARAM);
    static int handle_positionRubberCorner (void *, vrpn_HANDLERPARAM);
    static int handle_positionSweepLine (void *, vrpn_HANDLERPARAM);
    static int handle_positionSphere (void *, vrpn_HANDLERPARAM);
    static int handle_enableCollabHand (void *, vrpn_HANDLERPARAM);
    static int handle_setCollabHandPos (void *, vrpn_HANDLERPARAM);
    static int handle_setCollabMode (void *, vrpn_HANDLERPARAM);

  // realign texture handlers:
  static int handle_createRealignTextures (void *, vrpn_HANDLERPARAM);
  static int handle_setRealignTextureSliderRange (void *, vrpn_HANDLERPARAM);
  static int handle_computeRealignPlane (void *, vrpn_HANDLERPARAM);
//  static int handle_enableRealignTextures (void *, vrpn_HANDLERPARAM);
  static int handle_translateTextures (void *, vrpn_HANDLERPARAM);
  static int handle_scaleTextures (void *, vrpn_HANDLERPARAM);
  static int handle_shearTextures (void *, vrpn_HANDLERPARAM);
  static int handle_rotateTextures (void *, vrpn_HANDLERPARAM);
  static int handle_setTextureCenter (void *, vrpn_HANDLERPARAM);
  static int handle_setRealignTexturesConversionMap(void *,vrpn_HANDLERPARAM);

  static int handle_updateTexture(void *, vrpn_HANDLERPARAM);
//  static int handle_enableRegistration(void *, vrpn_HANDLERPARAM);
  static int handle_setTextureTransform (void *, vrpn_HANDLERPARAM);

  static int handle_setViewTransform (void *, vrpn_HANDLERPARAM);
  static int handle_createScreenImage(void *, vrpn_HANDLERPARAM);
};

#endif  // NMG_GRAPHICS_IMPL_H






