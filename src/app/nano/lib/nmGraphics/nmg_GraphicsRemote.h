/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMG_GRAPHICS_REMOTE
#define NMG_GRAPHICS_REMOTE

#include "nmg_Graphics.h"

class nmg_Graphics_Remote : public nmg_Graphics {

  public:

    nmg_Graphics_Remote (vrpn_Connection *);

    virtual ~nmg_Graphics_Remote (void);

    virtual void mainloop (void);

    virtual void changeDataset( nmb_Dataset * data);

    virtual void resizeViewport(int width, int height);
    virtual void getViewportSize(int *width, int * height);
    virtual void getDisplayPosition (q_vec_type &ll, q_vec_type &ul,
                                                q_vec_type &ur);

    virtual void loadRulergridImage (const char *);

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

    virtual void setOpacitySliderRange (float low, float hi);

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
    virtual void setOpacityPlaneName (const char *);
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

    virtual void loadRawDataTexture(const int which, const char *image_name,
        const int start_x, const int start_y);
    virtual void updateTexture(int which, const char *image_name,
       int start_x, int start_y,
       int end_x, int end_y);
//    virtual void enableRegistration(int on);
    virtual void setTextureTransform(double *xform);

//    virtual void enableRulergrid (int);
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

    virtual void setTextureMode (TextureMode, 
	TextureTransformMode = RULERGRID_COORD);
    virtual void setTextureScale (float);
    virtual void setTrueTipScale (float);

    virtual void setUserMode (int oldMode, int oldStyle, int newMode, int style);

    virtual void setLightDirection (q_vec_type &);
    virtual void resetLightDirection (void);

    virtual int addPolylinePoint (const PointType[2]);
    //    virtual int addPolylinePoint (const float [2][3]);
    virtual void emptyPolyline (void);

    virtual void setRubberLineStart (float, float);
    virtual void setRubberLineEnd (float, float);
    virtual void setRubberLineStart (const float [2]);
    virtual void setRubberLineEnd (const float [2]);

    virtual void setScanlineEndpoints(const float[3], const float[3]);
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


    virtual void getLightDirection (q_vec_type *) const;
    virtual int getHandColor (void) const;
    virtual int getSpecularity (void) const; 
//    virtual float getDiffusePercent (void) const;

    virtual const double * getMinColor (void) const;
    virtual const double * getMaxColor (void) const;

    // Note:
    //   This implementation's accessors is *NOT* guaranteed
    // to be consistent with the server!

  protected:

    q_vec_type d_lightDirection;
    int d_handColor;
    int d_specularity;
    float d_diffuse;
    float d_surface_alpha;

    double d_minColor [3];
    double d_maxColor [3];

    // added Nov 98 to support RenderMan.c;  should be obsolete

    char * d_colorMapDir;
    char * d_textureDir;
    ColorMap d_colorMap;
    ColorMap * d_curColorMap;

    double d_minAlpha;
    double d_maxAlpha;
    double d_color_min;
    double d_color_max;
    double d_data_min;
    double d_data_max;

    double d_opacity_slider_min;
    double d_opacity_slider_max;
};

#endif  // NMG_GRAPHICS_REMOTE

