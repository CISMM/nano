#ifndef NMR_REGISTRATION_CLIENT_H
#define NMR_REGISTRATION_CLIENT_H

#include "nmr_Registration_Interface.h"
#include "nmb_Device.h"

class nmr_Registration_Client;

/// data passed to the user's message handler
typedef struct {
    struct timeval msg_time;
    nmr_MessageType msg_type;
    nmr_Registration_Client *aligner;
} nmr_ClientChangeHandlerData;

/// client class
class nmr_Registration_Client : public nmb_Device_Client,
                                public nmr_Registration_Interface {
  public:
    nmr_Registration_Client(const char *name, vrpn_Connection *c = NULL);
    ~nmr_Registration_Client();

    virtual vrpn_int32 mainloop(void);

    // message-sending functions
    /// this should be called for each image before you start calling
    /// setScanline()
    int setImageParameters(nmr_ImageType whichImage,
                           vrpn_int32 res_x, vrpn_int32 res_y,
                           vrpn_float32 xSizeWorld, 
                           vrpn_float32 ySizeWorld,
                           vrpn_bool flipX, vrpn_bool flipY);
    /// this allows you to update either image
    int setScanline(nmr_ImageType whichImage, vrpn_int32 row,
                           vrpn_int32 line_length, vrpn_float32 *data);
    int setTransformationOptions(nmr_TransformationType type);
    int setTransformationParameters(vrpn_float32 *parameters);
    /// x,y are 0..1, z should be in whatever units the image is in;
    /// this will affect the registration result whenever that gets sent;
    /// intended for sending tip-position so that this can be corresponded
    /// with the image of the tip in the other image
    /// (this is information associated with but not visible in an
    ///  AFM image)
    int setFiducial(vrpn_int32 replace, vrpn_int32 num,
                vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
                vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt);

    // make the windows visible on the remote display
    int setGUIEnable(vrpn_bool enable, vrpn_int32 window);
	int setEditEnable(vrpn_bool enableAddAndDelete, vrpn_bool enableMove);
    int enableAutoUpdate(vrpn_bool enable);

    // for auto-alignment
    int setResolutions(vrpn_int32 numLevels, vrpn_float32 *stddev);
    int setIterationLimit(vrpn_int32 maxIterations);
    int setStepSize(vrpn_float32 stepSize);
    int setCurrentResolution(vrpn_int32 resolutionIndex);
    int autoAlign(vrpn_int32 mode);

    // message callback registration
    int registerChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ClientChangeHandlerData &info));
    int unregisterChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ClientChangeHandlerData &info));

    // data access functions intended to be called in message callback
    // since they only return the latest values
    /// this returns the parameters and which image they are for from the
    /// the last message received
    void getImageParameters(nmr_ImageType &whichImage,
                           vrpn_int32 &res_x, vrpn_int32 &res_y,
                           vrpn_float32 &xSizeWorld,
                           vrpn_float32 &ySizeWorld,
                           vrpn_bool &flipX, vrpn_bool &flipY);
    void getTransformationOptions(nmr_TransformationType &type);
    void getRegistrationResult(vrpn_int32 &whichTransform,
                               vrpn_float64 *matrix44);
    void getFiducial(
              vrpn_int32 &replace, vrpn_int32 &num,
              vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
              vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt);

  protected:
    static int VRPN_CALLBACK RcvImageParameters (void *_userdata, vrpn_HANDLERPARAM _p);
    static int VRPN_CALLBACK RcvTransformationOptions (void *_userdata, vrpn_HANDLERPARAM _p);
    static int VRPN_CALLBACK RcvRegistrationResult (void *_userdata, vrpn_HANDLERPARAM _p);
    static int VRPN_CALLBACK RcvFiducial (void *_userdata, vrpn_HANDLERPARAM _p);

    int notifyMessageHandlers(nmr_MessageType type,
        const struct timeval &msg_time);

    // here is where we keep the latest information from server messages
    nmr_ImageType d_imageParamsLastReceived;
    vrpn_int32 d_srcResX, d_srcResY;
    vrpn_float32 d_srcSizeX, d_srcSizeY;
    vrpn_bool d_srcFlipX, d_srcFlipY;
    vrpn_int32 d_targetResX, d_targetResY;
    vrpn_float32 d_targetSizeX, d_targetSizeY;
    vrpn_bool d_targetFlipX, d_targetFlipY;

    nmr_TransformationType d_transformType;

    vrpn_int32 d_whichTransform;
    vrpn_float64 d_matrix44[16];

    int d_numFiducialPoints;
    int d_replaceFiducialList;
    vrpn_float32 d_x_src[NMR_MAX_FIDUCIAL], d_y_src[NMR_MAX_FIDUCIAL],
                 d_z_src[NMR_MAX_FIDUCIAL];
    vrpn_float32 d_x_tgt[NMR_MAX_FIDUCIAL], d_y_tgt[NMR_MAX_FIDUCIAL],
                 d_z_tgt[NMR_MAX_FIDUCIAL];

    // message callback management
    typedef struct _msg_handler {
        void                                        *userdata;
        void (*handler)(void *ud, const nmr_ClientChangeHandlerData &info);
        struct _msg_handler                         *next;
    } msg_handler_list_t;

    msg_handler_list_t *d_messageHandlerList;
};


#endif
