#ifndef NMG_GRAPHICS_H
#define NMG_GRAPHICS_H


#ifndef INCLUDED_V_H
#include <v.h>
#define INCLUDED_V_H
#endif

#include <vrpn_Shared.h>
#include <nmb_Types.h>  // for PointType
#include <colormap.h>

#include "ImageMaker.h" // Image formats for screen capture


class Position_list;  // from Position.h
class nmb_Dataset;  // from nmb_Dataset.h

class vrpn_Connection;  // from vrpn_Connection.h

class nmg_Graphics {

  public:

    enum TextureMode { NO_TEXTURES, CONTOUR, RULERGRID, ALPHA, GENETIC, REALIGN,
			REGISTRATION };

    nmg_Graphics (vrpn_Connection *, const char * name);

    virtual ~nmg_Graphics (void);


    virtual void mainloop (void) = 0;


    // MANIPULATORS
    virtual void resizeViewport(int width, int height) = 0;
      // changes the size of the display window (actually this only
      // handles changing the way the world is drawn into the display
      // window and the actual resizing may be done by the window system)
    virtual void getDisplayPosition (q_vec_type &ll, q_vec_type &ul,
                                                        q_vec_type &ur) = 0;
      // gets position of lower left, upper left and upper right corners
      // of the screen in room space

    virtual void loadRulergridImage (const char * name) = 0;
      // Specifies the name of a PPM file to display as the rulergrid.

    virtual void enableChartjunk (int on) = 0;
      // Controls display of chartjunk (text in screenspace).
      // Turn on for real use, off to capture images for publication.
    virtual void enableFilledPolygons (int on) = 0;
      // Controls filling of surface.  Turn off to show wireframe for demos.
    virtual void enableSmoothShading (int on) = 0;
      // Controls smooth shading of surface.  Turn off to show faceting
      // for demos.

    virtual void enableTrueTip (int on) = 0;
      // Controls latency compensation technique:  displaying second
      // tip image.

    virtual void setAdhesionSliderRange (float low, float hi) = 0;
      // Sets the range over which adhesion is interpolated from 0 to 1.
      // Adhesion is not normally a graphics parameter, but is mapped
      // to one on PxFl.  This could be made clearer by renaming
      // "adhesion" within this module.

    virtual void setAlphaColor (float r, float g, float b) = 0;
      // Sets the color of the alpha-blended texture.
    virtual void setAlphaSliderRange (float low, float hi) = 0;
      // Sets the range over which the alpha-blended texture's
      // alpha values vary from 0 to 1.

    virtual void setBumpMapName (const char *) = 0;
      // Specifies the name of the dataset to use to drive bump mapping
      // on PxFl.

    virtual void setColorMapDirectory (const char *) = 0;
      // Specifies the path in which to search for color maps.
    virtual void setColorMapName (const char *) = 0;
      // Specifies the name of the color map to use.
    virtual void setColorSliderRange (float low, float hi) = 0;
      // Specifies the range of values over which to interpolate
      // the color map.

    virtual void setTextureDirectory (const char *) = 0;
      // Specifies the path in which to search for textures.

    virtual void setComplianceSliderRange (float low, float hi) = 0;
      // Sets the range over which compliance is interpolated from
      // 0 to 1.  Compliance is not normally a graphics parameter,
      // but is mapped to one on PxFl.  This could be made clearer
      // by renaming "compliance" within this module.

    virtual void setContourColor (int r, int g, int b) = 0;
      // Sets the color of the intermediate lines drawn in the
      // contour map.  Apparently the major lines (every 10th)
      // are hardwired to be white?
    virtual void setContourWidth (float) = 0;
      // Sets the width of the contour lines.

    virtual void setFrictionSliderRange (float low, float hi) = 0;
      // Sets the range over which friction is interpolated from
      // 0 to 1.  Friction is not normally a graphics parameter,
      // but is mapped to one on PxFl.  This could be made clearer
      // by renaming "friction" within this module.

	virtual void setBumpSliderRange (float low, float hi) = 0;
		// sets the range over which haptic bump size is
		// interpolated from 0 to 1
	virtual void setBuzzSliderRange (float low, float hi) = 0;
		// sets the range over which haptic buzz amplitude is
		// interpolated from 0 to 1

    virtual void setHandColor (int) = 0;
      // UNKNOWN.
    virtual void setHatchMapName (const char *) = 0;  // RENAME?
      // Specifies the name of the data plane to use to drive hatch
      // maps on PxFl.

    virtual void setIconScale (float) = 0;
      // Scale the 3D icons.

    virtual void setCollabHandPos(double [3], double [4]) = 0;
      //for setting position and rotation of icon for collaborator's hand
      //icon

    virtual void setCollabMode(int) = 0;
      //for setting position and rotation of icon for collaborator's hand
      //icon

    // arguments in range [0..1]
    virtual void setMinColor (const double [3]) = 0;
    virtual void setMaxColor (const double [3]) = 0;
    // arguments in range [0..255]
    virtual void setMinColor (const int [3]) = 0;
    virtual void setMaxColor (const int [3]) = 0;

    virtual void setPatternMapName (const char *) = 0;  // RENAME?
      // Specifies the name of the data plane to use to drive pattern
      // maps on PxFl.

    // Genetic Textures
    virtual void enableGeneticTextures (int) = 0;
    virtual void sendGeneticTexturesData (int, char **) = 0;

    // Realigning Textures:
    virtual void createRealignTextures( const char * ) = 0;
    virtual void setRealignTextureSliderRange (float, float) = 0;
    virtual void setRealignTexturesConversionMap
                    (const char *, const char *) = 0;
    virtual void computeRealignPlane( const char *, const char * ) = 0;
    virtual void enableRealignTextures (int on) = 0;
    virtual void translateTextures ( int on, float dx, float dy ) = 0;
    virtual void scaleTextures ( int on, float dx, float dy ) = 0;
    virtual void shearTextures ( int on, float dx, float dy ) = 0;
    virtual void rotateTextures ( int on, float theta ) = 0;
    virtual void setTextureCenter( float dx, float dy ) = 0;

    virtual void enableRegistration(int on) = 0;
    virtual void setTextureTransform(double *xform) = 0;

    virtual void enableRulergrid (int on) = 0;
      // Controls display of the rulergrid.

    virtual void setRulergridAngle (float) = 0;
      // Sets the angle at which the rulergrid is rotated.
    virtual void setRulergridColor (int r, int g, int b) = 0;
      // Sets the color used to draw the rulergrid.
    virtual void setRulergridOffset (float x, float y) = 0;
      // Sets the translation of the rulergrid from its default
      // position.  (In nm?)
    virtual void setRulergridOpacity (float alpha) = 0;
      // Sets the alpha value to use with the rulergrid.
    virtual void setRulergridScale (float) = 0;
      // Sets the number of nanometers between rulings in the
      // rulergrid.
    virtual void setRulergridWidths (float x, float y) = 0;
      // Sets the width of the lines in the rulergrid.

    virtual void setSpecularity (int) = 0;
      // Sets the specularity of the surface.
    virtual void setSpecularColor (float) = 0;
      // ???
    virtual void setDiffusePercent (float) = 0;
    virtual void setSurfaceAlpha (float) = 0;
    virtual void setSphereScale (float) = 0;
      // Set the size of the sphere drawn at the stylus tip in modify mode.

    virtual void setTesselationStride (int) = 0;
      // Sets the stride with which the data grid is tesselated.

    virtual void setTextureMode (TextureMode) = 0;
      // Determines which texture to display currently.
      // q.v. enum TextureMode { NO_TEXTURES, CONTOUR, RULERGRID, ALPHA };
    virtual void setTextureScale (float) = 0;
      // Interval between contour lines.

    virtual void setTrueTipScale (float) = 0;
      // Size at which true tip indicator is drawn
      // FOR DEBUGGING ONLY

    virtual void setUserMode (int oldMode, int newMode, int style) = 0;
      // Specifies the mode of interaction currently being used.
      // Controls which widgets are displayed.

    virtual void setLightDirection (q_vec_type &) = 0;
      // Sets the direction from which the scene is illuminated.
    virtual void resetLightDirection (void) = 0;
      // Resets the direction from which the scene is illuminated
      // to the default.

    // WARNING:  This code assumes there is only one polyline being
    // maintained (by the server).  If you need more, rewrite it.

    virtual int addPolylinePoint (const float [2][3]) = 0;
      // Adds a point to the active polyline.
    virtual void emptyPolyline (void) = 0;
      // Throws away all the points in the active polyline.

    virtual void setRubberLineStart (float, float) = 0;
    virtual void setRubberLineEnd (float, float) = 0;
    virtual void setRubberLineStart (const float [2]) = 0;
    virtual void setRubberLineEnd (const float [2]) = 0;

    virtual void setScanlineEndpoints(const float[3], const float [3]) = 0;
    virtual void displayScanlinePosition(const int enable) = 0;

    virtual void positionAimLine (const PointType top,
                                  const PointType bottom) = 0;
      // positionAimLine() is OBSOLETE

    virtual void positionRubberCorner (float x0, float y0,
                                       float x1, float y1) = 0;
    virtual void positionSweepLine (const PointType top,
                                    const PointType bottom) = 0;
    virtual void positionSphere (float x, float y, float z) = 0;

    // Screen capture
    virtual void createScreenImage
    (
       const char      *filename,
       const ImageType  type
    ) = 0;

    // ACCESSORS


    virtual void getLightDirection (q_vec_type *) const = 0;
      // Returns the direction from which the light emanates.
    virtual int getHandColor (void) const = 0;
      // Returns the last value passed by setHandColor().
    virtual int getSpecularity (void) const = 0;
      // Returns the specularity of the surface.

    virtual const double * getMinColor (void) const = 0;
    virtual const double * getMaxColor (void) const = 0;

    static const unsigned int defaultPort;
      // port on which to connect to a graphics server

  protected:

    vrpn_Connection * d_connection;
    vrpn_int32 d_myId;

    vrpn_int32 d_resizeViewport_type;
    vrpn_int32 d_loadRulergridImage_type;
    vrpn_int32 d_enableChartjunk_type;
    vrpn_int32 d_enableFilledPolygons_type;
    vrpn_int32 d_enableSmoothShading_type;
    vrpn_int32 d_enableTrueTip_type;
    vrpn_int32 d_setAdhesionSliderRange_type;
    vrpn_int32 d_setAlphaColor_type;
    vrpn_int32 d_setAlphaSliderRange_type;
    vrpn_int32 d_setBumpMapName_type;
    vrpn_int32 d_setColorMapDirectory_type;
    vrpn_int32 d_setColorMapName_type;
    vrpn_int32 d_setColorSliderRange_type;
    vrpn_int32 d_setTextureDirectory_type;
    vrpn_int32 d_setComplianceSliderRange_type;
    vrpn_int32 d_setContourColor_type;
    vrpn_int32 d_setContourWidth_type;
    vrpn_int32 d_setFrictionSliderRange_type;
	vrpn_int32 d_setBumpSliderRange_type;
	vrpn_int32 d_setBuzzSliderRange_type;
    vrpn_int32 d_setHandColor_type;
    vrpn_int32 d_setHatchMapName_type;
    vrpn_int32 d_setIconScale_type;
    vrpn_int32 d_setMinColor_type;
    vrpn_int32 d_setMaxColor_type;
    vrpn_int32 d_setPatternMapName_type;
    vrpn_int32 d_enableRulergrid_type;
    vrpn_int32 d_setRulergridAngle_type;
    vrpn_int32 d_setRulergridColor_type;
    vrpn_int32 d_setRulergridOffset_type;
    vrpn_int32 d_setRulergridOpacity_type;
    vrpn_int32 d_setRulergridScale_type;
    vrpn_int32 d_setRulergridWidths_type;
    vrpn_int32 d_setSpecularity_type;
    vrpn_int32 d_setSpecularColor_type;
    vrpn_int32 d_setDiffusePercent_type;
    vrpn_int32 d_setSurfaceAlpha_type;
    vrpn_int32 d_setSphereScale_type;
    vrpn_int32 d_setTesselationStride_type;
    vrpn_int32 d_setTextureMode_type;
    vrpn_int32 d_setTextureScale_type;
    vrpn_int32 d_setTrueTipScale_type;
    vrpn_int32 d_setUserMode_type;
    vrpn_int32 d_setLightDirection_type;
    vrpn_int32 d_resetLightDirection_type;
    vrpn_int32 d_addPolylinePoint_type;
    vrpn_int32 d_emptyPolyline_type;
    vrpn_int32 d_setRubberLineStart_type;
    vrpn_int32 d_setRubberLineEnd_type;
    vrpn_int32 d_positionAimLine_type;
    vrpn_int32 d_positionRubberCorner_type;
    vrpn_int32 d_positionSweepLine_type;
    vrpn_int32 d_positionSphere_type;

    //type for collaborator's hand position/orientation
    vrpn_int32 d_setCollabHandPos_type;
    vrpn_int32 d_setCollabMode_type;

    // Genetic Textures Network Types:
    vrpn_int32 d_enableGeneticTextures_type;
    vrpn_int32 d_sendGeneticTexturesData_type;

    // Realign Textures Network Types:
    long d_createRealignTextures_type;
    long d_setRealignTexturesConversionMap_type;
    long d_setRealignTextureSliderRange_type;
    long d_computeRealignPlane_type;
    long d_enableRealignTextures_type;
    long d_translateTextures_type;
    long d_scaleTextures_type;
    long d_shearTextures_type;
    long d_rotateTextures_type;
    long d_setTextureCenter_type;

    long d_enableRegistration_type;
    long d_setTextureTransform_type;

    long d_setScanlineEndpoints_type;
    long d_displayScanlinePosition_type;

    //Screen capture
    vrpn_int32 d_createScreenImage_type;


    // Each encode_ routine will allocate a new char [] to hold
    // the appropriate encoding of its arguments, and write the
    // length of that buffer into its first argument.
    // It is the caller's responsibility to delete [] this buffer!
    // (Please forgive the ugliness of that last detail...)
    // It will return NULL (and a length of 0) on error.
    // (Perhaps this should be disambiguated from the successful
    //  case of a 0-length encoding?!)

    // (There is no encode_ routine for messages whose only argument
    // is a single const char *, since they might as well send the array.
    // A NULL array should be sent as a 0-length message.
    // There is also no encode_ routine for messages with no arguments.)

    // Each decode_ routine takes a pointer to a character buffer and
    // a pointer to each of the values to extract.  It will return 0
    // on success, -1 on error.

    char * encode_resizeViewport (int * len, int, int);
    int decode_resizeViewport (const char * buf, int *, int *);

    char * encode_enableChartjunk (int * len, int);
    int decode_enableChartjunk (const char * buf, int *);
    char * encode_enableFilledPolygons (int * len, int);
    int decode_enableFilledPolygons (const char * buf, int *);
    char * encode_enableSmoothShading (int * len, int);
    int decode_enableSmoothShading (const char * buf, int *);
    char * encode_enableTrueTip (int * len, int);
    int decode_enableTrueTip (const char * buf, int *);
    char * encode_setAdhesionSliderRange (int * len, float low, float hi);
    int decode_setAdhesionSliderRange (const char * buf,
                                        float * low, float * hi);
    char * encode_setAlphaColor (int * len, float r, float g, float b);
    int decode_setAlphaColor (const char * buf,
                               float * r, float * g, float * b);
    char * encode_setAlphaSliderRange (int * len, float low, float hi);
    int decode_setAlphaSliderRange (const char * buf,
                                     float * low, float * hi);
    char * encode_setColorSliderRange (int * len, float low, float hi);
    int decode_setColorSliderRange (const char * buf,
                                     float * low, float * hi);
    char * encode_setComplianceSliderRange (int * len, float low, float hi);
    int decode_setComplianceSliderRange (const char * buf,
                                     float * low, float * hi);
    char * encode_setContourColor (int * len, int r, int g, int b);
    int decode_setContourColor (const char * buf, int * r, int * g, int * b);
    char * encode_setContourWidth (int * len, float);
    int decode_setContourWidth (const char * buf, float *);
    char * encode_setFrictionSliderRange (int * len, float low, float hi);
    int decode_setFrictionSliderRange (const char * buf,
                                        float * low, float * hi);
	char * encode_setBumpSliderRange (int * len, float low, float hi);
    int decode_setBumpSliderRange (const char * buf,
                                        float * low, float * hi);
    char * encode_setBuzzSliderRange (int * len, float low, float hi);
    int decode_setBuzzSliderRange (const char * buf,
                                        float * low, float * hi);
    char * encode_setHandColor (int * len, int);
    int decode_setHandColor (const char * buf, int *);
    char * encode_setIconScale (int * len, float);
    int decode_setIconScale (const char * buf, float *);
    char * encode_setMinColor (int * len, const double [3]);
    int decode_setMinColor (const char * buf, double [3]);
    char * encode_setMaxColor (int * len, const double [3]);
    int decode_setMaxColor (const char * buf, double [3]);
    char * encode_setMinColor (int * len, const int [3]);
    int decode_setMinColor (const char * buf, int [3]);
    char * encode_setMaxColor (int * len, const int [3]);
    int decode_setMaxColor (const char * buf, int [3]);
    char * encode_enableRulergrid (int * len, int);
    int decode_enableRulergrid (const char * buf, int *);
    char * encode_setRulergridAngle (int * len, float);
    int decode_setRulergridAngle (const char * buf, float *);
    char * encode_setRulergridColor (int * len, int r, int g, int b);
    int decode_setRulergridColor (const char * buf, int * r, int * g, int * b);
    char * encode_setRulergridOffset (int * len, float x, float y);
    int decode_setRulergridOffset (const char * buf, float * x, float * y);
    char * encode_setRulergridOpacity (int * len, float alpha);
    int decode_setRulergridOpacity (const char * buf, float * alpha);
    char * encode_setRulergridScale (int * len, float);
    int decode_setRulergridScale (const char * buf, float *);
    char * encode_setRulergridWidths (int * len, float x, float y);
    int decode_setRulergridWidths (const char * buf, float * x, float * y);
    char * encode_setSpecularity (int * len, int);
    int decode_setSpecularity (const char * buf, int *);
    char * encode_setDiffusePercent (int * len, float);
    int decode_setDiffusePercent (const char * buf, float *);
    char * encode_setSurfaceAlpha (int * len, float);
    int decode_setSurfaceAlpha (const char * buf, float *);
    char * encode_setSpecularColor (int * len, float);
    int decode_setSpecularColor (const char * buf, float *);
    char * encode_setSphereScale (int * len, float);
    int decode_setSphereScale (const char * buf, float *);
    char * encode_setTesselationStride (int * len, int);
    int decode_setTesselationStride (const char * buf, int *);
    char * encode_setTextureMode (int * len, TextureMode);
    int decode_setTextureMode (const char * buf, TextureMode *);
    char * encode_setTextureScale (int * len, float);
    int decode_setTextureScale (const char * buf, float *);
    char * encode_setTrueTipScale (int * len, float);
    int decode_setTrueTipScale (const char * buf, float *);
    char * encode_setUserMode (int * len, int oldMode, int newMode, int style);
    int decode_setUserMode (const char * buf, int * oldMode, int * newMode,
                             int * style);
    char * encode_setLightDirection (int * len, const q_vec_type &);
    int decode_setLightDirection (const char * buf, q_vec_type &);
    char * encode_addPolylinePoint (int * len, const float [2][3]);
    int decode_addPolylinePoint (const char * buf, float [2][3]);
    char * encode_setRubberLineStart (int * len, const float [2]);
    int decode_setRubberLineStart (const char * buf, float [2]);
    char * encode_setRubberLineEnd (int * len, const float [2]);
    int decode_setRubberLineEnd (const char * buf, float [2]);

    char * encode_setScanlineEndpoints (int *len, const float [6]);
    int decode_setScanlineEndpoints (const char *buf, float [6]);

    char * encode_displayScanlinePosition (int *len, const int);
    int decode_displayScanlinePosition (const char *buf, int *);

    char * encode_positionAimLine (int * len, const PointType,
                                   const PointType);
    int decode_positionAimLine (const char * buf, PointType, PointType);
    char * encode_positionRubberCorner (int * len, float, float, float, float);
    int decode_positionRubberCorner (const char * buf, float *, float *,
                                      float *, float *);
    char * encode_positionSweepLine (int * len, const PointType,
                                   const PointType);
    int decode_positionSweepLine (const char * buf, PointType, PointType);
    char * encode_positionSphere (int * len, float, float, float);
    int decode_positionSphere (const char * buf, float *, float *, float *);

    // for shared hand pointers
    char * encode_setCollabHandPos (int * len, double [3], double [4]);
    int decode_setCollabHandPos (const char *buf, double [3], double [4]);
    char * encode_setCollabMode (int * len, int);
    int decode_setCollabMode (const char *buf, int *);

    // Genetic Textures Network Transmission Functions:
    char * encode_enableGeneticTextures (int * len, int);
    int decode_enableGeneticTextures (const char * buf, int *);
    char * encode_sendGeneticTexturesData (int * len, int, char **);
    int decode_sendGeneticTexturesData (const char * buf, int *, char ***);

    // Realign Textures Network Transmission Functions:
    char *encode_setRealignTextureSliderRange ( int *len, float, float );
    int decode_setRealignTextureSliderRange( const char *buf,float *, float *);
    
    char *encode_two_char_arrays ( int *len, const char *, const char * );
    int decode_two_char_arrays ( const char *buf, char **, char **);
    
    char *encode_enableRealignTextures ( int *len, int );
    int decode_enableRealignTextures ( const char *buf, int *);
    
    char *encode_dx_dy ( int *len, float, float );
    int decode_dx_dy ( const char *buf, float *, float *);
    
    char *encode_rotateTextures ( int *len, float theta );
    int decode_rotateTextures ( const char *buf, float *theta );

    char * encode_enableRegistration (int * len, int);
    int decode_enableRegistration (const char * buf, int *);

    char *encode_textureTransform(int *len, double *);
    int decode_textureTransform(const char *buf, double *);

    char *encode_createScreenImage
    (
       int             *len,
       const char      *filename,
       const ImageType  type
    );
    int decode_createScreenImage
    (
       const char  *buf,
       char       **filename,
       ImageType   *type
    );
};

#endif  // NMG_GRAPHICS_H






