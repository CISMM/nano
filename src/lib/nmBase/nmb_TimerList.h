#ifndef NMB_TIMER_LIST
#define NMB_TIMER_LIST

#include <vrpn_Types.h>
#include <vrpn_Shared.h>

// List of timestamps and serial numbers.
// Used to track latency in the user interface.

// Not being used as a queue so much as a list - manipulation n+1
// (PHANTOM grab) can be satisfied while manipulation n (colormap
// change) is still outstanding.

// Remote Rendering:
//   Any manipulation that goes to the server has a latency equal
//     to RTT.
//   Any manipulation that doesn't go to the server has a latency
//     equal to frame time.
// In microscape.c, just before calling interaction(),
//   we call addTimestamp().
// In nmg_RenderClient::causeGridRedraw()
//   we call blockList(getListHead()),
//   sending getListHead() to the render server.
// In nmg_RenderServer::handle_TimerList(),
//   we handle any TimerList messages by sending them back to the
//   render client implementation.
// In nmg_RenderClient_Implementation::handle_TimerList(),
//   we call unblockList(serialNumber)


// interaction() gives us a tight bound on PHANTOM response time
// - from the time the PHANTOM message reaches us, not including
// latency in the device, in the server, or in the network/shmem
// between the controlling PC and our host - but a looser bound
// on response time for Tcl/Tk controls.

struct nmb_Timestamp {
  vrpn_int32 serialNumber;
  timeval timestamp;
  vrpn_bool pending;
  nmb_Timestamp * next;
};

class nmb_TimerList {

  public:

    nmb_TimerList (void);
    ~nmb_TimerList (void);

    vrpn_int32 newTimestep (void);
        // Returns SN of timestamp added.
        // Marks all unblocked timestamps as complete,
        // adds their delta time to d_totalTimeComplete,
        // and throws them away.

    vrpn_int32 getListHead (void);
        // Returns SN of timestamp most recently added.
    timeval getListHeadTime (void);
        // Returns timestamp most recently added.

    void blockList (vrpn_int32 sN);
    void unblockList (vrpn_int32 sN);

    void report (void);

    ///////

    void insert (vrpn_int32 sN);
      // Inserts a timestamp with arbitrary serial number in the list.
      // Use with care.
    void remove (void);
      // Pops the head off the list.  Use with care.


  protected:

    nmb_Timestamp * newTS (void);
    void freeTS (nmb_Timestamp ** snitch, nmb_Timestamp * victim);
    vrpn_int32 d_nextSN;

    timeval d_totalTimeComplete;
    vrpn_int32 d_totalTimestampsComplete;

    nmb_Timestamp * d_list;
    nmb_Timestamp * d_freePool;

};


#endif  // NMB_TIMER_LIST

