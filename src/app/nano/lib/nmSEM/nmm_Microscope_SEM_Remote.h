#ifndef NMM_MICROSCOPE_SEM_REMOTE_H
#define NMM_MICROSCOPE_SEM_REMOTE_H

#include "vrpn_Connection.h"
#include "nmm_Microscope_SEM.h"

class nmm_Microscope_SEM_Remote;

// This is what gets passed to handlers that are registered with the
// nmm_Microscope_SEM_Remote:
typedef struct {
    struct timeval msg_time;
    nmm_Microscope_SEM::msg_t msg_type;	// tells you what data has changed
    nmm_Microscope_SEM_Remote *sem;
} nmm_Microscope_SEM_ChangeHandlerData;

typedef void (*nmm_Microscope_SEM_Remote_ChangeHandler_t)(void *userdata,
                          const nmm_Microscope_SEM_ChangeHandlerData &info);

class nmm_Microscope_SEM_Remote : public nmm_Microscope_SEM {

  public:
    nmm_Microscope_SEM_Remote(const char *name, vrpn_Connection *c = NULL);
    ~nmm_Microscope_SEM_Remote(void);

    // message-sending functions
    int setResolution (vrpn_int32 res_x, vrpn_int32 res_y);
    int setPixelIntegrationTime (vrpn_int32 time_nsec);
    int setInterPixelDelayTime (vrpn_int32 time_nsec);
    int requestScan (vrpn_int32 nscans);

    // message callback registration
    int registerChangeHandler(void *userdata,
	nmm_Microscope_SEM_Remote_ChangeHandler_t handler);
    int unregisterChangeHandler(void *userdata,
        nmm_Microscope_SEM_Remote_ChangeHandler_t handler);

    // data access functions intended to be called in message callback
    void getResolution(vrpn_int32 &res_x, vrpn_int32 &res_y);
    void getPixelIntegrationTime(vrpn_int32 &time_nsec);
    void getInterPixelDelayTime(vrpn_int32 &time_nsec);
    void getWindowLineData(vrpn_int32 &start_x, vrpn_int32 &start_y,
		vrpn_int32 &dx, vrpn_int32 &dy, vrpn_int32 &line_length,
                vrpn_int32 &num_fields,
                vrpn_float32 **data);
    void getScanlineData(vrpn_int32 &start_x, vrpn_int32 &start_y,
		vrpn_int32 &dx, vrpn_int32 &dy, vrpn_int32 &line_length,
		vrpn_int32 &num_fields, vrpn_int32 &num_lines, 
                vrpn_uint8 **data);
    // other
    int connected() {return d_connection->connected();}

  protected:
    static int RcvReportResolution (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportPixelIntegrationTime
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportInterPixelDelayTime
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvWindowLineData
                (void *_userdata, vrpn_HANDLERPARAM _p);
	static int RcvScanlineData
				(void *_userdata, vrpn_HANDLERPARAM _p);

    int notifyMessageHandlers(nmm_Microscope_SEM::msg_t type,
        const struct timeval &msg_time);

    /* ---------------------------------------------------------------
       data from messages:
       -------------------
    */
    vrpn_int32 d_resolutionX, d_resolutionY;
    vrpn_int32 d_pixelIntegrationTime_nsec;
    vrpn_int32 d_interpixelDelayTime_nsec;
    // window line data
    vrpn_int32 d_startX, d_startY, d_dX, d_dY;
    vrpn_int32 d_lineLength;
    vrpn_int32 d_numFields;
    vrpn_int32 d_numLines;
    vrpn_float32 *d_data;
	vrpn_uint8 *d_uint8_data;

    /* ---------------------------------------------------------------
       message callback management
       ---------------------------
       each time we get a message in one of the connection callbacks (Rcv*),
       we call all the callbacks in this list
    */
    typedef struct _msg_handler {
        void                                        *userdata;
        nmm_Microscope_SEM_Remote_ChangeHandler_t   handler;
        struct _msg_handler                         *next;
    } msg_handler_list_t;

    msg_handler_list_t *d_messageHandlerList;

};

#endif
