#ifndef NMG_GRAPHICS_TIMER_H
#define NMG_GRAPHICS_TIMER_H

#include <nmb_TimerList.h>

#include "nmg_Graphics.h"
class nmg_Graphics_Implementation;  // from "nmg_GraphicsImpl.h"

// Wish there was a cleaner way to do this -
// We want to mark as active the current record in an nmb_TimerList
// before executing ANY graphics call.  We could either search through
// the rest of the code & explicitly activate it before making the call
// into nmg_Graphics classes, or we can use subclassing and composition
// to build classes that do that automatically and choose or not choose
// them at runtime.

class nmg_Graphics_Timer : public nmg_Graphics {

  public:

    nmg_Graphics_Timer (nmg_Graphics *, nmb_TimerList *);

    virtual ~nmg_Graphics_Timer (void);

    // NEW MANIPULATORS

    void timeViewpointChanges (vrpn_bool);

    // INHERITED MANIPULATORS

    virtual void mainloop (void);

    virtual void resizeViewport(int width, int height);
    virtual void getDisplayPosition (q_vec_type &ll, q_vec_type &ul,
                                                     q_vec_type &ur);
    virtual void loadRulergridImage (const char * name);
    virtual void causeGridRedraw (void);
    virtual void causeGridRebuild (void);
    virtual void enableChartjunk (int on);
    virtual void enableFilledPolygons (int on);
    virtual void enableSmoothShading (int on);
    virtual void enableTrueTip (int on);
    virtual void setAdhesionSliderRange (float low, float hi);
    virtual void setAlphaColor (float r, float g, float b);
    virtual void setAlphaSliderRange (float low, float hi);
    virtual void setBumpMapName (const char *);
    virtual void setColorMapDirectory (const char *);
    virtual void setColorMapName (const char *);
    virtual void setColorSliderRange (float low, float hi);
    virtual void setTextureDirectory (const char *);
    virtual void setComplianceSliderRange (float low, float hi);
    virtual void setContourColor (int r, int g, int b);
    virtual void setContourWidth (float);
    virtual void setFrictionSliderRange (float low, float hi);
    virtual void setBumpSliderRange (float low, float hi);
    virtual void setBuzzSliderRange (float low, float hi);
    virtual void setHandColor (int);
    virtual void setHatchMapName (const char *);
    virtual void setAlphaPlaneName (const char *);
    virtual void setColorPlaneName (const char *);
    virtual void setContourPlaneName (const char *);
    virtual void setHeightPlaneName (const char *);
    virtual void setIconScale (float);
    virtual void enableCollabHand (vrpn_bool);
    virtual void setCollabHandPos(double [3], double [4]);
    virtual void setCollabMode(int);
    virtual void setMinColor (const double [3]);
    virtual void setMaxColor (const double [3]);
    virtual void setMinColor (const int [3]);
    virtual void setMaxColor (const int [3]);
    virtual void setPatternMapName (const char *);
    virtual void sendGeneticTexturesData (int, char **);
    virtual void createRealignTextures( const char * );
    virtual void setRealignTextureSliderRange (float, float);
    virtual void setRealignTexturesConversionMap
                    (const char *, const char *);
    virtual void computeRealignPlane( const char *, const char * );
    virtual void translateTextures ( int on, float dx, float dy );
    virtual void scaleTextures ( int on, float dx, float dy );
    virtual void shearTextures ( int on, float dx, float dy );
    virtual void rotateTextures ( int on, float theta );
    virtual void setTextureCenter( float dx, float dy );
    virtual void loadRawDataTexture(const int which, const char *image_name,
        const int start_x, const int start_y);
    virtual void updateTexture(int which, const char *image_name,
       int start_x, int start_y, 
       int end_x, int end_y);
    virtual void setTextureTransform(double *xform);
    virtual void setRulergridAngle (float);
    virtual void setRulergridColor (int r, int g, int b);
    virtual void setRulergridOffset (float x, float y);
    virtual void setNullDataAlphaToggle (int val);
    virtual void setRulergridOpacity (float alpha);
    virtual void setRulergridScale (float);
    virtual void setRulergridWidths (float x, float y);
    virtual void setSpecularity (int);
    virtual void setSpecularColor (float);
    virtual void setDiffusePercent (float);
    virtual void setSurfaceAlpha (float);
    virtual void setSphereScale (float);
    virtual void setTesselationStride (int);
    virtual void setTextureMode (TextureMode,TextureTransformMode);
    virtual void setTextureScale (float);
    virtual void setTrueTipScale (float);
    virtual void setUserMode (int oldMode, int newMode, int style);
    virtual void setLightDirection (q_vec_type &);
    virtual void resetLightDirection (void);
    //    virtual int addPolylinePoint (const float [2][3]);
    virtual int addPolylinePoint(const PointType[2]);
    virtual void emptyPolyline (void);
    virtual void setRubberLineStart (float, float);
    virtual void setRubberLineEnd (float, float);
    virtual void setRubberLineStart (const float [2]);
    virtual void setRubberLineEnd (const float [2]);
    virtual void setScanlineEndpoints(const float[3], const float [3]);
    virtual void displayScanlinePosition(const int enable);
    virtual void positionAimLine (const PointType top,
                                  const PointType bottom);
    virtual void positionRubberCorner (float x0, float y0,
                                       float x1, float y1);
    virtual void positionSweepLine (const PointType topL,
                                    const PointType bottomL,
				    const PointType topR,
                                    const PointType bottomR);
    virtual int addPolySweepPoints (const PointType, const PointType,
				    const PointType, const PointType);
    virtual void setRubberSweepLineStart (const PointType, const PointType);
    virtual void setRubberSweepLineEnd (const PointType, const PointType);

    virtual void positionSphere (float x, float y, float z);
    virtual void createScreenImage (const char * filename,
                                    const ImageType type);
    virtual void setViewTransform (v_xform_type);


    virtual void getLightDirection (q_vec_type *) const;
    virtual int getHandColor (void) const;
    virtual int getSpecularity (void) const;
    virtual const double * getMinColor (void) const;
    virtual const double * getMaxColor (void) const;

  protected:

    vrpn_bool d_timingViewpointChanges;

    nmg_Graphics * d_imp;

    void activateTimer (void);

    nmb_TimerList * d_timer;

};



#endif  // NMG_GRAPHICS_TIMER_H






