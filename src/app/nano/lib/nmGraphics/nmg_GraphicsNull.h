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

    virtual void enableChartjunk (int);
    virtual void enableFilledPolygons (int);
    virtual void enableSmoothShading (int);

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

    // Realigning Textures:
    virtual void createRealignTextures( const char * );
    virtual void setRealignTextureSliderRange (float, float);
    virtual void setRealignTexturesConversionMap( char *, char * );
    virtual void computeRealignPlane( char *, char * );
    virtual void enableRealignTextures (int on);
    virtual void translateTextures ( int on, float dx, float dy );
    virtual void scaleTextures ( int on, float dx, float dy );
    virtual void shearTextures ( int on, float dx, float dy );
    virtual void rotateTextures ( int on, float theta );
    virtual void setTextureCenter( float dx, float dy );

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

    virtual void setTesselationStride (int);

    virtual void setTextureMode (TextureMode, 
	TextureTransformMode = RULERGRID_COORD);
    virtual void setTextureScale (float);

    virtual void setUserMode (int oldMode, int newMode, int style);  // TODO

    virtual void setLightDirection (q_vec_type &);
    virtual void resetLightDirection (void);

    virtual int addPolylinePoint (const float [2][3]);
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
