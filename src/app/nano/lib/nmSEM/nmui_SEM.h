#ifndef SEM_H
#define SEM_H

#include <vrpn_Types.h>
#include <vrpn_Connection.h>

#include <nmb_Image.h>

#include <Tcl_Linkvar.h>

#include <imageViewer.h>

#include "nmm_Microscope_SEM_Remote.h"
#include "nmm_EDAX.h"  // some constants peculiar to the EDAX SEM interface

class nmui_ColorMap;


#define SEM_TCL_FILE "sem.tcl"
#define SEM_FILE_SUFFIX ".sem"

struct SEMInitializationState {
    SEMInitializationState(void);

    char deviceName [128];
    char inputLogName [256];
    char outputLogName [256];

    vrpn_bool readingLogFile;
    vrpn_bool writingLogFile;
};


class nms_SEM_ui {

  public:
    nms_SEM_ui(Tcl_Interp *interp, const char *tcl_script_dir,
               const char *name,
               vrpn_Connection *c = NULL);
    virtual ~nms_SEM_ui (void);

    void displayTexture(int /*enable*/) {display_texture = 0;};
    int updateDisplays (void);


    // Handle getting any reports
    virtual int mainloop(const struct timeval * timeout = NULL);

	nmm_Microscope_SEM_Remote *semClient() {return sem;}

	void clearImageBuffers() {sem->clearImageBuffers();}
  protected:
    nmm_Microscope_SEM_Remote *sem;

    Tcl_Interp *tcl_interp;

    // variables that can be set through the user interface:
    Tclvar_int sem_acquire_image;
    Tclvar_int sem_acquire_continuous;
    Tclvar_int display_texture;
    Tclvar_string sem_image_name;
    Tclvar_string sem_colormap;
    Tclvar_float sem_texture_alpha;
    Tclvar_int no_graphics_update;
    Tclvar_int sem_resolution; // an index into a set of image sizes
    Tclvar_int pixel_integration_time_nsec;
    Tclvar_int inter_pixel_delay_time_nsec;
    Tclvar_int sem_window_open;

    // greyscale image window (might be useful for debugging)
    ImageViewer *image_viewer;
    int image_window_id;
    static int drawGreyscaleWindow(
        const ImageViewerDisplayData &data, 
        void *ud);
    int updateSurfaceTexture(int start_x, int start_y, 
     int dx, int dy,
     int line_length, int num_fields, int num_lines, nmb_PixelType pix_type,
     void *data);

    // colormap
    nmui_ColorMap* CMap;

    // callbacks for user interface variables
    //   for acquire_image:
    static void handle_acquire_image(vrpn_int32 _newval, void *_userdata);
    //   for resolution
    static void handle_resolution_change(vrpn_int32 _newval, void *_ud);
    //   for texture display
    static void handle_texture_display_change(vrpn_int32 _newval, void *_ud);
    //   for colormap
    static void handle_colormap_change(const char *name, void *_ud);
    //   for colormap minmax
    static void handle_colormap_minmax_change(vrpn_float64, void *_ud);
    //   for texture alpha
    static void handle_sem_texture_alpha_change(vrpn_float64 _newval, void *_ud);
    //   for pixel_integration_time_nsec
    static void handle_integration_change(vrpn_int32 _newval, void *_ud);
    //   for inter_pixel_delay_time_nsec
    static void handle_delay_change(vrpn_int32 _newval, void *_ud);
    //   for sem_window_open
    static void handle_window_visibility_change(vrpn_int32 _newval, void *_ud);

    // callback for parameter and data messages from the hardware:
    static void handle_device_change(void *ud, 
                   const nmm_Microscope_SEM_ChangeHandlerData &info);

    int set_tcl_callbacks(void);
};
#endif
