#ifndef NMR_REGISTRATION_SERVER_H
#define NMR_REGISTRATION_SERVER_H

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "nmb_Device.h"
#include "nmb_Image.h"
#include "nmr_Registration_Interface.h"
#include "nmb_Transform_TScShR.h"

class nmr_Registration_Server;

/// data passed to the user's message handler
class nmr_ServerChangeHandlerData {
  public:
    nmr_ServerChangeHandlerData() {};
    struct timeval msg_time;
    nmr_MessageType msg_type;
    nmr_Registration_Server *aligner;
};

class nmr_Registration_Server : public nmb_Device_Server,
                                public nmr_Registration_Interface {
  public:
    nmr_Registration_Server(const char *name, vrpn_Connection *c);
    ~nmr_Registration_Server();

    //virtual vrpn_int32 mainloop(void);

    int setImageParameters(nmr_ImageType whichImage,
         vrpn_int32 res_x, vrpn_int32 res_y,
         vrpn_float32 size_x, vrpn_float32 size_y,
         vrpn_bool flip_x, vrpn_bool flip_y);
    int setScanline(nmr_ImageType whichImage, vrpn_int32 row,
                           vrpn_int32 line_length, vrpn_float32 *data);

    int setTransformationOptions(nmr_TransformationType type);
    int setTransformationParameters(vrpn_float32 *parameters);
    int setGUIEnable(vrpn_bool enable);
    int setFiducial(vrpn_int32 replace, vrpn_int32 num,
               vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
               vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt);
    int enableAutoUpdate(vrpn_bool enable);

    // for auto-alignment
    int setResolutions(vrpn_int32 numLevels, vrpn_float32 *stddev);
    int setIterationLimit(vrpn_int32 maxIterations);
    int setStepSize(vrpn_float32 stepSize);
    int setCurrentResolution(vrpn_int32 resolutionIndex);
    int autoAlign(vrpn_int32 mode);

    // message callback registration
    int registerChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ServerChangeHandlerData &info));
    int unregisterChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ServerChangeHandlerData &info));

    // data access functions intended to be called in message callback
    // since they only return the latest values
    /// this returns the parameters and which image they are for from the
    /// the last message received
    void getImageParameters(nmr_ImageType &whichImage,
          vrpn_int32 &res_x, vrpn_int32 &res_y,
          vrpn_float32 &size_x, vrpn_float32 &size_y,
          vrpn_bool &flip_x, vrpn_bool &flip_y);
    void getAutoAlign(vrpn_int32 &mode);
    void getGUIEnable(vrpn_bool &enabled);
    void getTransformationOptions(nmr_TransformationType &type);
    void getTransformationParameters(vrpn_float32 *parameters);
    void getScanline(nmr_ImageType &whichImage,
                           vrpn_int32 &row, vrpn_int32 &length,
                           vrpn_float32 **data);
    void getFiducial(vrpn_int32 &replace, vrpn_int32 &num,
                     vrpn_float32 *x_src, vrpn_float32 *y_src, 
                     vrpn_float32 *z_src, vrpn_float32 *x_tgt,
                     vrpn_float32 *y_tgt, vrpn_float32 *z_tgt);
    void getAutoUpdateEnable(vrpn_bool &enabled);

    // for auto-alignment
    void getResolutions(vrpn_int32 &numLevels, vrpn_float32 *stddev);
    void getIterationLimit(vrpn_int32 &maxIterations);
    void getStepSize(vrpn_float32 &stepSize);
    void getCurrentResolution(vrpn_int32 &resolutionIndex);

    int sendRegistrationResult(int whichTransform, double xform[16]);

  protected:
    static int RcvSetImageParameters (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetScanline (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetTransformationOptions
                                     (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetTransformationParameters
                                     (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvEnableRegistration(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvEnableGUI(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvFiducial (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvEnableAutoUpdate (void *_userdata, vrpn_HANDLERPARAM _p);

    static int RcvSetResolutions(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetIterationLimit(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetStepSize(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetCurrentResolution(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvAutoAlign(void *_userdata, vrpn_HANDLERPARAM _p);

    int notifyMessageHandlers(nmr_MessageType type,
        const struct timeval &msg_time);

    nmr_ImageType d_imageParamsLastReceived;
    vrpn_int32 d_srcResX, d_srcResY;
    vrpn_float32 d_srcSizeX, d_srcSizeY;
    vrpn_bool d_srcFlipX, d_srcFlipY;
    vrpn_int32 d_targetResX, d_targetResY;
    vrpn_float32 d_targetSizeX, d_targetSizeY;
    vrpn_bool d_targetFlipX, d_targetFlipY;

    nmr_ImageType d_imageScanlineLastReceived;
    vrpn_int32 d_row, d_length;
    vrpn_int32 d_lengthAllocated;
    vrpn_float32 *d_scanlineData;
    
    nmr_TransformationType d_transformType;

    vrpn_bool d_GUIEnabled;
    vrpn_bool d_autoUpdateAlignment;

    vrpn_int32 d_numLevels;
    vrpn_float32 d_stddev[NMR_MAX_RESOLUTION_LEVELS];
    vrpn_int32 d_resolutionIndex;
    vrpn_int32 d_maxIterations;
    vrpn_float32 d_stepSize;
    vrpn_int32 d_autoAlignEnableMode;

    // for fiducial message
    int d_numFiducialPoints;
    int d_replaceFiducialList;
    vrpn_float32 d_x_src[NMR_MAX_FIDUCIAL], d_y_src[NMR_MAX_FIDUCIAL],
                 d_z_src[NMR_MAX_FIDUCIAL];
    vrpn_float32 d_x_tgt[NMR_MAX_FIDUCIAL], d_y_tgt[NMR_MAX_FIDUCIAL], 
                 d_z_tgt[NMR_MAX_FIDUCIAL];

    vrpn_float32 *d_transformParameters;

    // message callback management
    typedef struct _msg_handler {
        void *userdata;
        void (*handler)(void *ud, const nmr_ServerChangeHandlerData &info);
        struct _msg_handler *next;
    } msg_handler_list_t;

    msg_handler_list_t *d_messageHandlerList;
};

#endif
