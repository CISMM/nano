#ifndef NMR_REGISTRATION_INTERFACE_H
#define NMR_REGISTRATION_INTERFACE_H

#include <vrpn_Connection.h>
#include <vrpn_Types.h>

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
    vrpn_int32 d_EnableRegistration_type;
    vrpn_int32 d_EnableGUI_type;
    vrpn_int32 d_Fiducial_type;

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
    static char * encode_EnableRegistration (vrpn_int32 *len,
           vrpn_int32 enable);
    static vrpn_int32 decode_EnableRegistration (const char **buf,
           vrpn_int32 *enable);
    static char * encode_EnableGUI (vrpn_int32 *len,
           vrpn_int32 enable);
    static vrpn_int32 decode_EnableGUI (const char **buf,
           vrpn_int32 *enable);
    static char * encode_Fiducial (vrpn_int32 *len,
           vrpn_float32 x_src, vrpn_float32 y_src, vrpn_float32 z_src,
           vrpn_float32 x_tgt, vrpn_float32 y_tgt, vrpn_float32 z_tgt);
    static vrpn_int32 decode_Fiducial (const char **buf,
           vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
           vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt);

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
                                    vrpn_float64 *matrix44);
    static vrpn_int32 decode_RegistrationResult (const char **buf,
                                    vrpn_float64 *matrix44);
};

/// represents source or target images (registration results in a
/// transformation that transforms source image into target image)
enum nmr_ImageType {NMR_SOURCE, NMR_TARGET};

/// these options will likely change in the future but NMR_2D2D_AFFINE
/// is a safe choice
// TRANSLATION = translation (in 2D) only
// TRANSLATION_ROTATION_SCALE = translation, rotation, uniform scale (all in 2D)
// AFFINE = translation, rotation/shear, non-uniform scale (all in 2D)

enum nmr_TransformationType {NMR_TRANSLATION, NMR_TRANSLATION_ROTATION_SCALE,
                NMR_2D2D_AFFINE, NMR_2D2D_PERSPECTIVE, NMR_3D2D};

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
     NMR_ENABLE_REGISTRATION,
     NMR_ENABLE_GUI
};

#endif
