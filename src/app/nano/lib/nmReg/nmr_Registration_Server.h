#ifndef NMR_REGISTRATION_SERVER_H
#define NMR_REGISTRATION_SERVER_H

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "nmb_Device.h"
#include "nmb_Image.h"
#include "nmr_Registration_Interface.h"

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
                           vrpn_bool treat_as_height_field);
    int setScanline(nmr_ImageType whichImage, vrpn_int32 row,
                           vrpn_int32 line_length, vrpn_float32 *data);

    int setTransformationOptions(nmr_TransformationType type);
    int setRegistrationEnable(vrpn_bool enable);
    int setGUIEnable(vrpn_bool enable);
    int addFiducial(nmr_ImageType whichImage, 
                    vrpn_float32 x, vrpn_float32 y, vrpn_float32 z);

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
                           vrpn_bool &treat_as_height_field);
    void getRegistrationEnable(vrpn_bool &enabled);
    void getGUIEnable(vrpn_bool &enabled);
    void getTransformationOptions(nmr_TransformationType &type);
    void getScanline(nmr_ImageType &whichImage,
                           vrpn_int32 &row, vrpn_int32 &length,
                           vrpn_float32 **data);
    void getFiducial(nmr_ImageType &whichImage,
                     vrpn_float32 &x, vrpn_float32 &y, vrpn_float32 &z);

    int sendRegistrationResult(double xform[16]);

  protected:
    static int RcvSetImageParameters (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetScanline (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetTransformationOptions
                                     (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvEnableRegistration(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvEnableGUI(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvFiducial (void *_userdata, vrpn_HANDLERPARAM _p);

    int notifyMessageHandlers(nmr_MessageType type,
        const struct timeval &msg_time);

    nmr_ImageType d_imageParamsLastReceived;
    vrpn_int32 d_srcResX, d_srcResY;
    vrpn_bool d_srcHeightField;
    vrpn_int32 d_targetResX, d_targetResY;
    vrpn_bool d_targetHeightField;

    nmr_ImageType d_imageScanlineLastReceived;
    vrpn_int32 d_row, d_length;
    vrpn_int32 d_lengthAllocated;
    vrpn_float32 *d_scanlineData;
    
    nmr_TransformationType d_transformType;

    vrpn_bool d_registrationEnabled;
    vrpn_bool d_GUIEnabled;

    nmr_ImageType d_imageFiducialLastRecieved;
    vrpn_float32 d_x, d_y, d_z;

    // message callback management
    typedef struct _msg_handler {
        void *userdata;
        void (*handler)(void *ud, const nmr_ServerChangeHandlerData &info);
        struct _msg_handler *next;
    } msg_handler_list_t;

    msg_handler_list_t *d_messageHandlerList;
};

#endif
