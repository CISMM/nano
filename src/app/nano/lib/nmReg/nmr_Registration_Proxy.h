#ifndef NMR_REGISTRATION_PROXY_H
#define NMR_REGISTRATION_PROXY_H
/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/

#include "nmr_Registration_Interface.h"
#include "nmr_Registration_Client.h"
#include "nmr_Registration_Impl.h"
#include "nmb_TransformMatrix44.h"

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

    vrpn_int32 setTransformationParameters(vrpn_float32 *parameters);
    vrpn_int32 sendFiducial(
                  vrpn_float32 x_src, vrpn_float32 y_src, vrpn_float32 z_src,
                  vrpn_float32 x_tgt, vrpn_float32 y_tgt, vrpn_float32 z_tgt);
    vrpn_int32 setResolutions(vrpn_int32 numLevels, vrpn_float32 *stddev);
    vrpn_int32 setIterationLimit(vrpn_int32 maxIterations);
    vrpn_int32 setStepSize(vrpn_float32 stepSize);
    vrpn_int32 setCurrentResolution(vrpn_int32 resolutionIndex);
    vrpn_int32 autoAlignImages(vrpn_int32 mode);
    
    vrpn_int32 setGUIEnable(vrpn_bool enable);
    vrpn_int32 setColorMap(nmr_ImageType whichImage, nmb_ColorMap * cmap);
    vrpn_int32 setColorMinMax(nmr_ImageType whichImage, 
                              vrpn_float64 dmin, vrpn_float64 dmax,
                              vrpn_float64 cmin, vrpn_float64 cmax);
    vrpn_int32 setImage(nmr_ImageType whichImage, nmb_Image *im, 
                        vrpn_bool flip_x, vrpn_bool flip_y);
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
       vrpn_float32 &size_x, vrpn_float32 &size_y,
       vrpn_bool &flip_x, vrpn_bool &flip_y);
    void getTransformationOptions(nmr_TransformationType &type);
    void getRegistrationResult(vrpn_int32 &which, vrpn_float64 *matrix44);
    void getRegistrationResult(vrpn_int32 &which, nmb_TransformMatrix44 &xform);

  protected:
    nmr_Registration_Server *d_server; // non-NULL if local implementation
    nmr_Registration_Impl *d_local_impl; // non-NULL if local implementation
    nmr_Registration_Client *d_remote_impl; // never NULL

    vrpn_bool d_local; // are we using local implementation?

    // here is where we keep the latest information from server messages
    nmr_ImageType d_imageParamsLastReceived;
    vrpn_int32 d_res_x, d_res_y;
    vrpn_float32 d_size_x, d_size_y, d_size_z;
    vrpn_bool d_flip_x, d_flip_y;

    nmr_TransformationType d_transformType;

    int d_transformSource;
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
