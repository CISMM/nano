#ifndef PSEUDOSTREAM_H
#define PSEUDOSTREAM_H


#include "nmm_AFM_Report.h"
#include "nmm_SPM_Report.h"

#include <vrpn_Shared.h>  // for timeval/timezone
#ifndef VRPN_CONNECTION_H
#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM
#endif

class BCGrid;

class PseudoStream : 
    public nmm_AFM_Report, public nmm_SPM_Report  
{
public:
    PseudoStream(vrpn_Connection * connection, char * sender_name);

    long dispatchMessage (long len, const char * buf,
                          vrpn_int32 type);

    int FileToStream(char * baseFileName);

    int EmitScanMessagesFromGrid(BCGrid * grid);

protected:
    timeval d_last_message_time;
    timeval d_time_offset;
    int d_default_sender_id;
    vrpn_Connection * d_connection;

    vrpn_int32 d_numX, d_numY;
    vrpn_float32 d_minX, d_maxX, d_minY, d_maxY;
    int d_forceScanDatasetMsg;
    float **d_rawData;
};


#endif
