#include "nmb_Decoration.h"

#include <string.h>  // memcpy()
#include <stdio.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)<(b)?(b):(a))


nmb_Decoration::nmb_Decoration (void) :
  selectedRegion_changed (1),
  //red_changed (1),
  //green_changed (1),
  //blue_changed (1),
  mode (IMAGE),
  elapsedTime (0),
  rateOfTime (1),
  user_mode (0),
  //std_dev_color_scale (1.0f),
  num_markers_shown (2000),
  trueTipLocation_changed (0),
  modSetpoint(0), modSetpointMin(0), modSetpointMax(0),
  imageSetpoint(0), imageSetpointMin(0), imageSetpointMax(0),
  scanlineSetpoint(0), scanlineSetpointMin(0), scanlineSetpointMax(0),
  num_pulses (0),
  max_num_pulses (10),
  pulses (new nmb_LocationInfo [max_num_pulses]),
  num_scrapes (0),
  max_num_scrapes (10),
  scrapes (new nmb_LocationInfo [max_num_scrapes]),
  scrapeCallbacks (NULL),
  pulseCallbacks (NULL),
  scan_line (NULL),
  scanLineCount (0),
  drawScanLine (1),
  num_slow_line_3d_markers (0)
  //  max_num_slow_line_3d_markers (2)
  //  slowLine3dMarkers ( new PointType [max_num_slow_line_3d_markers] )
{
  if (!pulses)
    max_num_pulses = 0;
  if (!scrapes)
    max_num_scrapes = 0;
}


nmb_Decoration::nmb_Decoration (int markerHeight, int numMarkers) :
  selectedRegion_changed (1),
  //red_changed (1),
  //green_changed (1),
  //blue_changed (1),
  mode (IMAGE),
  elapsedTime (0),
  rateOfTime (1),
  user_mode (0),
  //std_dev_color_scale (1.0f),
  trueTipLocation_changed (0),
  modSetpoint(0), modSetpointMin(0), modSetpointMax(0),
  imageSetpoint(0), imageSetpointMin(0), imageSetpointMax(0),
  scanlineSetpoint(0), scanlineSetpointMin(0), scanlineSetpointMax(0),
  num_pulses (0),
  max_num_pulses (10),
  pulses (new nmb_LocationInfo [max_num_pulses]),
  num_scrapes (0),
  max_num_scrapes (10),
  scrapes (new nmb_LocationInfo [max_num_scrapes]),
  scrapeCallbacks (NULL),
  pulseCallbacks (NULL),
  scan_line (NULL),
  scanLineCount (0),
  drawScanLine (1)
  //  max_num_slow_line_3d_markers (2)
  //  slowLine3dMarkers ( new PointType [max_num_slow_line_3d_markers] )
{
  marker_height = markerHeight;
  num_markers_shown = numMarkers;

  if (!pulses)
    max_num_pulses = 0;
  if (!scrapes)
    max_num_scrapes = 0;
}


nmb_Decoration::~nmb_Decoration (void) {
  callbackEntry * t0, * t1;

  if (pulses)
    delete [] pulses;
  if (scrapes)
    delete [] scrapes;
  if (scan_line) {
    delete [] scan_line;
    scan_line = NULL;
  }

  t0 = scrapeCallbacks;
  while (t0) {
    t1 = t0->next;
    delete t0;
    t0 = t1;
  }

  t0 = pulseCallbacks;
  while (t0) {
    t1 = t0->next;
    delete t0;
    t0 = t1;
  }
  // Okay, for some reason, if I delete this object when the program is shutting down
  // it causes a seg fault.  However, if I just leave it alone, everything's just fine
  // and dandy.  In other words, something is killing this besides me and I have
  // no idea what it is.  Maybe the PointType class?
  
  /*
  if (slowLine3dMarkers) {
    // printf("Deleting memory.\n"); // This fixes the seg. fault problem :)
    delete [] slowLine3dMarkers;
    printf("Cleared memory.\n");
    slowLine3dMarkers = NULL;
  }
  */
  
}

void nmb_Decoration::addSlowLine3dMarker(float x, float y, float z) {
  if (num_slow_line_3d_markers < max_num_slow_line_3d_markers) {
    slowLine3dMarkers[num_slow_line_3d_markers][0] = x;
    slowLine3dMarkers[num_slow_line_3d_markers][1] = y;
    slowLine3dMarkers[num_slow_line_3d_markers][2] = z;
    num_slow_line_3d_markers++;
  }
}

void nmb_Decoration::initScanline(long _lineCount) {
  scanLineCount = _lineCount;
  scan_line = new PointType[scanLineCount];
}

void nmb_Decoration::addScrapeMark (PointType Top, PointType Bottom) {
  nmb_LocationInfo * temp;
  callbackEntry * nce;

  // The top does not take into account the height of the scrapeHeight
  Top[2] += marker_height;

  // Grow virtual memory if needed
  if (num_scrapes >= max_num_scrapes) {
    temp = new nmb_LocationInfo [2 * max_num_scrapes];
    if (!temp) {
      fprintf(stderr, "nmb_Decoration::addScrapeMark:  Out of memory.\n");
      return;
    }
    memcpy(temp, scrapes, num_scrapes * sizeof(nmb_LocationInfo));
    delete [] scrapes;
    scrapes = temp;
    max_num_scrapes *= 2;
  }

  // Add a scrapes indicator to the list
  scrapes[num_scrapes].x = Top[0];
  scrapes[num_scrapes].y = Top[1];
  scrapes[num_scrapes].top = Top[2];
  scrapes[num_scrapes].bottom = Bottom[2];
  num_scrapes++; 

//fprintf(stderr, "Scraping at %.2f %.2f %.2f-%.2f\n", Top[0], Top[1], Top[2],
//Bottom[2]);

  // do callbacks
  for (nce = scrapeCallbacks;  nce;  nce = nce->next)
    if (nce->f(Top, Bottom, nce->userdata)) return;
}


void nmb_Decoration::addPulseMark (PointType Top, PointType Bottom) {
  nmb_LocationInfo * temp;
  callbackEntry * nce;

  // Grow virtual memory if needed
  if (num_pulses >= max_num_pulses) {
    temp = new nmb_LocationInfo [2 * max_num_pulses];
    if (!temp) {
      fprintf(stderr, "nmb_Decoration::addScrapeMark:  Out of memory.\n");
      return;
    }
    memcpy(temp, pulses, num_pulses * sizeof(nmb_LocationInfo));
    delete [] pulses;
    pulses = temp;
    max_num_pulses *= 2;
  }

  // Add a pulse indicator to the list
  pulses[num_pulses].x = Top[0];
  pulses[num_pulses].y = Top[1];
  pulses[num_pulses].top = Top[2];
  pulses[num_pulses].bottom = Bottom[2];
  num_pulses++;

  // do callbacks
  for (nce = pulseCallbacks;  nce;  nce = nce->next)
    if (nce->f(Top, Bottom, nce->userdata)) return;
}

void nmb_Decoration::clearScrapes (void) {
  num_scrapes = 0;
}

void nmb_Decoration::clearPulses (void) {
  num_pulses = 0;
}

void nmb_Decoration::registerNewScrapeCallback
      (nmb_SURFACE_MARKER_CALLBACK f, void * userdata) {
  callbackEntry * nce;

  nce = new callbackEntry;
  if (!nce) {
    fprintf(stderr, "nmb_Decoration::registerNewScrapeCallback:  "
                    "Out of memory!\n");
    return;
  }

  nce->f = f;
  nce->userdata = userdata;
  nce->next = scrapeCallbacks;
  scrapeCallbacks = nce;
  return;
}

void nmb_Decoration::registerNewPulseCallback
      (nmb_SURFACE_MARKER_CALLBACK f, void * userdata) {
  callbackEntry * nce;

  nce = new callbackEntry;
  if (!nce) {
    fprintf(stderr, "nmb_Decoration::registerNewPulseCallback:  "
                    "Out of memory!\n");
    return;
  }

  nce->f = f;
  nce->userdata = userdata;
  nce->next = pulseCallbacks;
  pulseCallbacks = nce;
  return;
}

void nmb_Decoration::traverseVisibleScrapes
          (int (* f) (const nmb_LocationInfo &, void *), void * userdata) {
  int i;
  int numToShow;

  numToShow = num_markers_shown;
  numToShow = min(numToShow, num_scrapes);
  numToShow = max(numToShow, 0);

//fprintf(stderr, "Traversing %d scrapes\n", numToShow);

  for (i = num_scrapes - numToShow; i < num_scrapes; i++)
    if (f(scrapes[i], userdata)) return;

}


void nmb_Decoration::traverseVisiblePulses
          (int (* f) (const nmb_LocationInfo &, void *), void * userdata) {
  int i;
  int numToShow;

  numToShow = num_markers_shown;
  numToShow = min(numToShow, num_pulses);
  numToShow = max(numToShow, 0);

  for (i = num_pulses - numToShow; i < num_pulses; i++)
    if (f(pulses[i], userdata)) return;

}


