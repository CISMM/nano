#ifndef NMR_REGISTRATION_PROXY_H
#define NMR_REGISTRATION_PROXY_H

#include "nmr_Registration_Interface.h"
#include "nmr_Registration_Client.h"
#include "nmr_Registration_Impl.h"

/**
  This class provides an interface that can either be to a server program
  running on a separate machine or to code running in the same process as
  the proxy. To choose a server, pass the constructor argument for the 
  name of the server you would like to connect to. If no name is given,
  the local implementation will be used.
*/

class nmr_Registration_Proxy;

typedef struct {
    struct timeval msg_time;
    nmr_MessageType msg_type;
    nmr_Registration_Proxy *aligner;
} nmr_ProxyChangeHandlerData;

class nmr_Registration_Proxy {
  public:
    nmr_Registration_Proxy(const char *name = NULL, vrpn_Connection *c=NULL);
    ~nmr_Registration_Proxy();

    vrpn_int32 mainloop(void);

    vrpn_int32 registerImages();
    vrpn_int32 setGUIEnable(vrpn_bool enable);
    vrpn_int32 setImage(nmr_ImageType whichImage, nmb_Image *im);
    static void handle_registration_change(void *ud,
                        const nmr_ClientChangeHandlerData &info);

    // message callback registration
    int registerChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ProxyChangeHandlerData &info));
    int unregisterChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ProxyChangeHandlerData &info));

    // data access functions intended to be called in message callback
    // since they only return the latest values
    /// this returns the parameters and which image they are for from the
    /// the last message received
    void getImageParameters(nmr_ImageType &whichImage,
       vrpn_int32 &res_x, vrpn_int32 &res_y,
       vrpn_float32 &size_x, vrpn_float32 &size_y);
    void getTransformationOptions(nmr_TransformationType &type);
    void getRegistrationResult(vrpn_float64 *matrix44);

  protected:
    nmr_Registration_Server *d_server; // non-NULL if local implementation
    nmr_Registration_Impl *d_local_impl; // non-NULL if local implementation
    nmr_Registration_Client *d_remote_impl; // never NULL

    vrpn_bool d_local; // are we using local implementation?

    // here is where we keep the latest information from server messages
    nmr_ImageType d_imageParamsLastReceived;
    vrpn_int32 d_res_x, d_res_y;
    vrpn_float32 d_size_x, d_size_y, d_size_z;

    nmr_TransformationType d_transformType;

    vrpn_float64 d_matrix44[16];

    int notifyMessageHandlers(nmr_MessageType type,
        const struct timeval &msg_time);


    // message callback management
    typedef struct _msg_handler {
        void                                        *userdata;
        void (*handler)(void *ud, const nmr_ProxyChangeHandlerData &info);
        struct _msg_handler                         *next;
    } msg_handler_list_t;

    msg_handler_list_t *d_messageHandlerList;
};

#endif
