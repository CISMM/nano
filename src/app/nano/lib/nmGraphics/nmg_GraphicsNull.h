/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMG_GRAPHICS_NULL_H
#define NMG_GRAPHICS_NULL_H

#include "nmg_Graphics.h"

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM

class nmg_Graphics_Null : public nmg_Graphics {

  public:

    nmg_Graphics_Null (nmb_Dataset * data,
                       const int minColor [3],
                       const int maxColor [3],
                       const char * rulergridName = NULL,
                       vrpn_Connection * = NULL);

    virtual ~nmg_Graphics_Null (void);

    virtual void mainloop (void);

    virtual void resizeViewport(int width, int height);
    virtual void getDisplayPosition (q_vec_type &ll, q_vec_type &ul,
					q_vec_type &ur);

    virtual void loadRulergridImage (const char *);
	virtual void loadVizImage (const char *);

    virtual void enableChartjunk (int);
    virtual void enableFilledPolygons (int);
    virtual void enableSmoothShading (int);

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

    // arguments in range [0..1]
    virtual void setMinColor (const double [3]);
    virtual void setMaxColor (const double [3]);
    // arguments in range [0..255]
    virtual void setMinColor (const int [3]);
    virtual void setMaxColor (const int [3]);

    virtual void setPatternMapName (const char *);  // RENAME?

    // Genetic Textures
    virtual void enableGeneticTextures (int);
    virtual void sendGeneticTexturesData (int, char **);

    // Colormap Texture:
    virtual void createColormapTexture( const char * );

    virtual void setTextureColormapSliderRange (int, float, float, float, float);
    virtual void setTextureColormapConversionMap( int, const char *, const char * );
    virtual void setTextureAlpha( int, float );

    virtual void setTextureTransform(double *xform);

    virtual void enableRulergrid (int);
    virtual void setRulergridAngle (float);
    virtual void setRulergridColor (int r, int g, int b);
    virtual void setRulergridOffset (float x, float y);
    virtual void setNullDataAlphaToggle (int val);
    virtual void setRulergridOpacity (float alpha);
    virtual void setRulergridScale (float);
    virtual void setRulergridWidths (float x, float y);

    virtual void setSpecularity (int);
    virtual void setSphereScale (float);

    virtual void setTesselationStride (int, int);

    virtual void setTextureMode (TextureMode, 
	TextureTransformMode = RULERGRID_COORD, int region = 0);
    virtual void setTextureScale (float);

    virtual void setUserMode (int oldMode, int newMode, int style);  // TODO

    virtual void setLightDirection (q_vec_type &);
    virtual void resetLightDirection (void);

    //    virtual int addPolylinePoint (const float [2][3]);
    virtual int addPolylinePoint (const PointType[2]);
    virtual void emptyPolyline (void);

    virtual void setRubberLineStart (float, float);
    virtual void setRubberLineEnd (float, float);
    virtual void setRubberLineStart (const float [2]);
    virtual void setRubberLineEnd (const float [2]);

    virtual void positionAimLine (const PointType, const PointType);
    virtual void positionRubberCorner (float, float, float, float);
    virtual void positionSweepLine (const PointType, const PointType);
    virtual void positionSphere (float, float, float);

    virtual void createScreenImage(const char *filename, const ImageType type);

	virtual void setRegionMask(BCPlane *mask);
    virtual void setViztexScale (float);
    virtual void setRegionMaskHeight (float, float, int);
    virtual void createRegion ();
    virtual void destroyRegion (int);

    //These functions are related to controlling what changes affect the
    //entire surface and what don't.
    virtual void associateAlpha(vrpn_bool associate, int region);
    virtual void associateFilledPolygons(vrpn_bool associate, int region);
    virtual void associateTextureDisplayed(vrpn_bool associate, int region);
    virtual void associateTextureMode(vrpn_bool associate, int region);
    virtual void associateTextureTransformMode(vrpn_bool associate, int region);
    virtual void associateStride(vrpn_bool associate, int region);

    // ACCESSORS
    virtual void getLightDirection (q_vec_type *) const;
    virtual int getHandColor (void) const;
    virtual int getSpecularity (void) const;

    virtual const double * getMinColor (void) const;
    virtual const double * getMaxColor (void) const;

  private:

    v_index * d_displayIndexList;

};

#endif  // NMG_GRAPHICS_NULL_H
