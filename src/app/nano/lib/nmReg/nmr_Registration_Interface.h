#ifndef NMR_REGISTRATION_INTERFACE_H
#define NMR_REGISTRATION_INTERFACE_H

#include <vrpn_Connection.h>
#include <vrpn_Types.h>

#define NMR_MAX_RESOLUTION_LEVELS (5000)
#define NMR_MAX_FIDUCIAL (5000)

/**
   design:
     2 images (distinguished as source and target)
     image parameters: width, height, whether or not to treat as height field
     images updated one scanline at a time
     transformation options:
         2D->2D (constraining c=g=m=n=o=p=i=j=k=l=0),
         2D->2D (constraining i=j=k=l=c=g=o=0),
         3D->2D (i=j=k=l=0), if source not specified as a height field
                 then z is treated as being 0

     transformation is in general represented by a 4x4 matrix:

           | u |   | a b c d | | x |
           | v | = | e f g h |*| y |
           | z'|   | i j k l | | z |
           | w |   | m n o p | | 1 |

     where (x,y) are in pixels of source image,
     z = source(x,y) if source is height field; otherwise z = 0
     (u/w,v/w) are in pixels of target image
     z'/w = target(u/w,v/w) if both source and target are height fields;
            otherwise z' = 0
*/

/// Client/server network interface class
class nmr_Registration_Interface {
  public:
    nmr_Registration_Interface (const char *name, vrpn_Connection * c = NULL);
    virtual ~nmr_Registration_Interface();

  protected:
    // client-->server messages
    vrpn_int32 d_SetImageParameters_type;
    vrpn_int32 d_SetImageScanlineData_type;
    vrpn_int32 d_SetTransformationOptions_type;
    vrpn_int32 d_SetTransformationParameters_type;
    vrpn_int32 d_SetResolutions_type;
    vrpn_int32 d_SetIterationLimit_type;
    vrpn_int32 d_SetStepSize_type;
    vrpn_int32 d_SetCurrentResolution_type;
    vrpn_int32 d_EnableAutoUpdate_type;
    vrpn_int32 d_AutoAlign_type;
    vrpn_int32 d_EnableGUI_type;
	vrpn_int32 d_EnableEdit_type;
    vrpn_int32 d_SetFiducial_type;
	vrpn_int32 d_ReportFiducial_type;

    // server-->client messages
    vrpn_int32 d_ImageParameters_type;
    vrpn_int32 d_TransformationOptions_type;
    vrpn_int32 d_RegistrationResult_type;

    // message encode/decode functions
    // client-->server
    static char * encode_SetImageParameters (vrpn_int32 *len,
           vrpn_int32 which_image,
           vrpn_int32 res_x, vrpn_int32 res_y,
           vrpn_float32 xSizeWorld, vrpn_float32 ySizeWorld,
           vrpn_bool flip_x, vrpn_bool flip_y);
    static vrpn_int32 decode_SetImageParameters (const char **buf,
           vrpn_int32 *which_image,
           vrpn_int32 *res_x, vrpn_int32 *res_y,
           vrpn_float32 *xSizeWorld, vrpn_float32 *ySizeWorld,
           vrpn_bool *flip_x, vrpn_bool *flip_y);
    static char * encode_SetImageScanlineData (vrpn_int32 *len,
           vrpn_int32 which_image,
           vrpn_int32 row, vrpn_int32 line_length,
           vrpn_float32 *data);
    static vrpn_int32 decode_SetImageScanlineData (const char **buf,
           vrpn_int32 *which_image,
           vrpn_int32 *row, vrpn_int32 *line_length,
           vrpn_float32 *data, vrpn_int32 data_length);
    static char * encode_SetTransformationOptions (vrpn_int32 *len,
           vrpn_int32 transformType);
    static vrpn_int32 decode_SetTransformationOptions (const char **buf,
           vrpn_int32 *transformType);
    static char * encode_SetTransformationParameters (vrpn_int32 *len,
           vrpn_float32 *transformParameters);
    static vrpn_int32 decode_SetTransformationParameters (const char **buf,
           vrpn_float32 *transformParameters);
    static char * encode_SetResolutions (vrpn_int32 *len,
           vrpn_int32 numLevels, vrpn_float32 *std_dev);
    static vrpn_int32 decode_SetResolutions (const char **buf,
           vrpn_int32 *numLevels, vrpn_float32 *std_dev);
    static char * encode_SetIterationLimit (vrpn_int32 *len,
           vrpn_int32 maxIterations);
    static vrpn_int32 decode_SetIterationLimit (const char **buf,
           vrpn_int32 *maxIterations);
    static char * encode_SetStepSize (vrpn_int32 *len,
           vrpn_float32 stepSize);
    static vrpn_int32 decode_SetStepSize (const char **buf,
           vrpn_float32 *stepSize);
    static char * encode_SetCurrentResolution (vrpn_int32 *len,
           vrpn_int32 resolutionLevel);
    static vrpn_int32 decode_SetCurrentResolution (const char **buf,
           vrpn_int32 *resolutionLevel); 
    static char * encode_EnableAutoUpdate (vrpn_int32 *len,
           vrpn_int32 enable);
    static vrpn_int32 decode_EnableAutoUpdate (const char **buf,
           vrpn_int32 *enable);
    static char * encode_AutoAlign (vrpn_int32 *len,
           vrpn_int32 mode);
    static vrpn_int32 decode_AutoAlign (const char **buf,
           vrpn_int32 *mode);
    static char * encode_EnableGUI (vrpn_int32 *len,
           vrpn_int32 enable, vrpn_int32 window);
    static vrpn_int32 decode_EnableGUI (const char **buf,
           vrpn_int32 *enable, vrpn_int32 *window);
	static char * encode_EnableEdit (vrpn_int32 *len,
		vrpn_int32 addAndDelete, vrpn_int32 move);
	static vrpn_int32 decode_EnableEdit (const char **buf,
		vrpn_int32 *addAndDelete, vrpn_int32 *move);

    // server-->client
    static char * encode_ImageParameters (vrpn_int32 *len,
           vrpn_int32 which_image,
           vrpn_int32 res_x, vrpn_int32 res_y,
           vrpn_float32 xSizeWorld, vrpn_float32 ySizeWorld,
           vrpn_bool flip_x, vrpn_bool flip_y);
    static vrpn_int32 decode_ImageParameters (const char **buf,
           vrpn_int32 *which_image,
           vrpn_int32 *res_x, vrpn_int32 *res_y,
           vrpn_float32 *xSizeWorld, vrpn_float32 *ySizeWorld,
           vrpn_bool *flip_x, vrpn_bool *flip_y);
    static char * encode_TransformationOptions (vrpn_int32 *len,
           vrpn_int32 transformType);
    static vrpn_int32 decode_TransformationOptions (const char **buf,
           vrpn_int32 *transformType);
    static char * encode_RegistrationResult (vrpn_int32 *len,
                           vrpn_int32 whichTransform, vrpn_float64 *matrix44);
    static vrpn_int32 decode_RegistrationResult (const char **buf,
                           vrpn_int32 *whichTransform, vrpn_float64 *matrix44);

    // both
    static char * encode_Fiducial (vrpn_int32 *len,
           vrpn_int32 replace, vrpn_int32 num,
           vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
           vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt);
    static vrpn_int32 decode_Fiducial (const char **buf,
           vrpn_int32 *replace, vrpn_int32 *num,
           vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
           vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt);
};

/// represents what role an image plays in the transformation
/// SOURCE is the image from which points are transformed
/// TARGET is the image into which the points from SOURCE are transformed
/// SOURCE_HEIGHTFIELD provides a third dimension for points in the SOURCE
/// image so that we can solve for a transformation from 3D to 2D - although
/// the SOURCE image itself may be a height field, we can also imagine an
/// application in which it is some kind of color textured on that heightfield
/// so this is why the SOURCE and SOURCE_HEIGHTFIELD images are stored separate
/// One of these values is sent as part of the SetImageParameters and 
/// SetImageScanlineData messages
enum nmr_ImageType {NMR_SOURCE, NMR_TARGET, NMR_SOURCE_HEIGHTFIELD};

/// represents tracker for fiducial spot optimization
enum nmr_FiducialSpotTracker {NMR_NO_TRACKER, NMR_LOCAL_MAX_TRACKER, NMR_CONE_TRACKER, NMR_DISK_TRACKER, 
    NMR_FIONA_TRACKER, NMR_SYMMETRIC_TRACKER};

/// values sent in the ENABLE_GUI message
enum nmr_WindowType {NMR_SOURCEWINDOW, NMR_TARGETWINDOW, NMR_ALLWINDOWS};

/// represents method by which the transformation was computed
/// MANUAL is generated as a result of a manual alignment
/// AUTOMATIC is generated as a result of automatic alignment
/// DEFAULT is generated in response to setting the default transformation
/// parameters from the client side
/// One of these values is sent as part of the RegistrationResult message
enum nmr_RegistrationType {NMR_MANUAL, NMR_AUTOMATIC, NMR_DEFAULT};

/// options for using automatic alignment
/// AUTOALIGN_FROM_MANUAL - starts searching for optimal transform from 
/// whatever transformation was set manually
/// AUTOALIGN_FROM_DEFAULT - starts searching for optimal transform from
/// whatever transformation was set as the default
/// AUTOALIGN_FROM_AUTO - starts searching for optimal transform from the
/// last transformation that was set automatically
/// One of these values is sent as part of the AutoAlign message
enum nmr_AutoAlignMode {NMR_AUTOALIGN_FROM_MANUAL, NMR_AUTOALIGN_FROM_DEFAULT,
                        NMR_AUTOALIGN_FROM_AUTO};

/// these options will likely change in the future but NMR_2D2D_AFFINE
/// is a safe choice
// TRANSLATION = translation (in 2D) only
// TRANSLATION_ROTATION_SCALE = translation, rotation, uniform scale (all in 2D)
// AFFINE = translation, rotation/shear, non-uniform scale (all in 2D)

enum nmr_TransformationType {NMR_TRANSLATION, NMR_TRANSLATION_ROTATION_SCALE,
                NMR_ROTATION, NMR_SCALE, 
                NMR_2D2D_AFFINE, NMR_2D2D_PERSPECTIVE, NMR_3D2D,
                NMR_3D3D_AFFINE, NMR_2D2D_AFFINE_Z_TRANSLATE,
                NMR_2D2D_AFFINE_Z_UNIFORMSCALING_Z_TRANSLATE};

/// these represent types of updates you can expect from the registration
/// server
enum nmr_MessageType {
     // for client and server:
     NMR_IMAGE_PARAM, NMR_TRANSFORM_OPTION, 
     // for client only:
     NMR_REG_RESULT,
     // for server only:
     NMR_SCANLINE,
     NMR_FIDUCIAL,
     NMR_SET_RESOLUTIONS,
     NMR_SET_ITERATION_LIMIT,
     NMR_SET_STEPSIZE,
     NMR_SET_CURRENT_RESOLUTION,
     NMR_ENABLE_AUTOUPDATE,
     NMR_AUTOALIGN,
     NMR_ENABLE_GUI,
	 NMR_ENABLE_EDIT,
     NMR_TRANSFORM_PARAM
};

#endif
