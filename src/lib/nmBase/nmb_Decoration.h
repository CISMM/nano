#ifndef NMB_DECORATION_H
#define NMB_DECORATION_H

#include "nmb_Types.h"  // for PointType
#include "nmb_Line.h"  // for nmb_Line

// class nmb_Decoration
//
// Tom Hudson, April 1998

///   Tracks status of any indicators (that a microscope sets) that might
/// be displayed directly in the graphics window.
struct nmb_LocationInfo {
  float x, y;
  float bottom, top;
};

typedef int (* nmb_SURFACE_MARKER_CALLBACK)
        (const PointType, const PointType, void *);


class nmb_Decoration {

  public:

    //enum { MAX_LOCATION_INFOS = 2000 };
    enum tipMode { IMAGE, FEEL, MODIFY, SCANLINE };

    nmb_Decoration (void);
    nmb_Decoration ( int markerHeight, int numMarkers );
      // Constructor.

    ~nmb_Decoration (void);
      // Destructor.

    // selected region
    double selectedRegionMinX, selectedRegionMinY,
           selectedRegionMaxX, selectedRegionMaxY;
    int selectedRegion_changed;

    /// measure lines
    //PointType red_top, red_bot,
              //green_top, green_bot,
              //blue_top, blue_bot;
    //int red_changed,
        //green_changed,
        //blue_changed;
    nmb_Line red;
    nmb_Line green;
    nmb_Line blue;

    // microscope current scanline (visual)

    void initScanline(long);
    void clearScanline();
    PointType *scan_line;
    long scanLineCount;
    int drawScanLine;

    // slowline 3d markers
    // PointType *slowline3dmarkers;
    // until I can figure out why a delete [] on this causes a segfault when the
    // program shuts down...
    PointType slowLine3dMarkers[2];
    int num_slow_line_3d_markers;
    void addSlowLine3dMarker(float, float, float);

    nmb_Line aimLine;

    /// mode of interaction
    tipMode mode;

    long elapsedTime;  ///< seconds
    int rateOfTime;  ///< seconds/second, instream_rate or 1 if live
    long totalStreamTime; ///< total length of stream, seconds
    int user_mode;  ///< user_mode[0] from interaction.c


    /// surface modification markers: marker_height determines the minimum
    /// scaled height that a marker will appear above the surface 
    int num_markers_shown;
    int marker_height;

    /// latency compensation features
    float trueTipLocation [3];
    int trueTipLocation_changed;

    //
    float modSetpoint, modSetpointMin, modSetpointMax;
    float imageSetpoint, imageSetpointMin, imageSetpointMax;
    float scanlineSetpoint, scanlineSetpointMin, scanlineSetpointMax;

    /// Color map drift compensation, keep track of changes in
    /// first scan line data.
    float first_line_avg, first_line_avg_prev;

	int ds_sphere_axis;
// To add:
// state.data.inputPoint
// state.modify.setpoint/min/max
// state.image.amplitude/min/max

    // MANIPULATORS

    void addScrapeMark (PointType Top, PointType Bottom, double surface_z);
    void addPulseMark (PointType Top, PointType Bottom);
      ///< Appends a marker to the end of the appropriate list.
      ///< Invokes all registered callbacks in undefined order.
      ///< If one callback returns nonzero, no further callbacks
      ///< will be invoked.

    void clearScrapes (void);
    void clearPulses (void);
      ///< Throws away all scrapes/pulses.

    void registerNewScrapeCallback (nmb_SURFACE_MARKER_CALLBACK f,
                                    void * userdata);
    void registerNewPulseCallback (nmb_SURFACE_MARKER_CALLBACK f,
                                   void * userdata);
      ///< Registers callbacks to be invoked by addScrape/PulseMark.

    void traverseVisibleScrapes (int (* f) (const nmb_LocationInfo &, void *),
                                 void * userdata);
    void traverseVisiblePulses (int (* f) (const nmb_LocationInfo &, void *),
                                void * userdata);
      ///< For each of the <num_markers_shown> most recent scrapes/pulses
      ///< <f> will be called with <userdata> as its second argument.
      ///< The order in which markers are traversed is undefined.
      ///< If <f> returns nonzero, the traversal will terminate.

    void setScrapeHeightScale(float scale);
      ///< Sets the height scale factor for scrape markers
      ///< Should be set to match scaling of the currently displayed data

    float getScrapeHeightScale();
      ///< Gets the height scale factor for scrape markers

    void setScrapeHeightOffset(float offset);
      ///< Sets the offset in height for display of scrape markers
      ///< Offset is done before scale factor is applied
      ///< A reasonable value is the maximum height value in the currently
      ///< displayed data

    float getScrapeHeightOffset();
      ///< Gets the offset in height for display of scrape markers

  private:

    int num_pulses;
    int max_num_pulses;
    nmb_LocationInfo * pulses;
    int num_scrapes;
    int max_num_scrapes;
    nmb_LocationInfo * scrapes;
    int max_num_slow_line_3d_markers;
    float scrapeHeightScale;
    float scrapeHeightOffset;
    float scrapeMaxDepth;

    struct callbackEntry {
      nmb_SURFACE_MARKER_CALLBACK f;
      void * userdata;
      callbackEntry * next;
    };

    callbackEntry * scrapeCallbacks;
    callbackEntry * pulseCallbacks;
};


#endif  // NMB_DECORATION_H
