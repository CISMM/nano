#include <microscape.h> // for disableOtherTextures
#include "nmui_SEM.h"
#include "tcl.h"
#include "nmb_Dataset.h"
#include "nmg_Graphics.h"

extern nmb_Dataset * dataset;
extern nmg_Graphics * graphics;

/*
vrpn_int32 nms_SEM_ui::d_matrixSizeX[EDAX_NUM_SCAN_MATRICES] =
                                          EDAX_SCAN_MATRIX_X;
vrpn_int32 nms_SEM_ui::d_matrixSizeY[EDAX_NUM_SCAN_MATRICES] =
                                          EDAX_SCAN_MATRIX_Y;
*/

SEMInitializationState::SEMInitializationState():
    readingLogFile (VRPN_FALSE),
    writingLogFile (VRPN_FALSE)
{
    strcpy(deviceName, "null");
}

nms_SEM_ui::nms_SEM_ui(Tcl_Interp *interp, const char * /*tcl_script_dir*/,
               const char *name,
               vrpn_Connection *c):
    tcl_interp(interp),
    sem_acquire_image("sem_acquire_image", 0),
    sem_acquire_continuous("sem_acquire_continuous", 1),
    display_texture("sem_display_texture", 0),
    no_graphics_update("sem_no_graphics_update", 0),
    sem_resolution("sem_resolution", 0),
    pixel_integration_time_nsec("sem_pixel_integration_time_nsec", 0),
    inter_pixel_delay_time_nsec("sem_inter_pixel_delay_time_nsec", 0),
    sem_window_open("sem_window_open", 0)
{
    //    char command[256];
    printf("creating the sem\n");
    sem = new nmm_Microscope_SEM_Remote(name, c);
    sem->registerChangeHandler(this, handle_device_change);
    fprintf(stderr, "done creating the sem\n");
    char *display_name;

    /* Load the Tcl script that handles main interface window */
/*
    sprintf(command, "source %s/%s",tcl_script_dir, SEM_TCL_FILE);
    printf("evaluating %s\n", command);
    TCLEVALCHECK2(tcl_interp, command);
    fprintf(stderr, "done evaluating\n");
*/
    set_tcl_callbacks();
/*
    initializeTcl
                ("resolution", ".sem_win.left.sem_frame.resolution");
*/

    sem_acquire_continuous = 1;

    image_viewer = ImageViewer::getImageViewer();
    display_name = (char *)getenv("V_X_DISPLAY");
    if (display_name == NULL)
        display_name = (char *)getenv("DISPLAY");
    if (display_name == NULL)
        display_name = strdup("unix:0");
    else
        display_name = strdup(display_name);

    image_viewer->init(display_name);

    image_window_id = image_viewer->createWindow(
          display_name, 300, 200, 256, 200, "SEM");
    image_viewer->setValueRange(image_window_id, 0.0, 255.0);
    image_viewer->setWindowDisplayHandler(image_window_id, drawGreyscaleWindow,
           this);
    int i;
    for (i = 0; i < EDAX_NUM_SCAN_MATRICES; i++){
	image_uint8[i] = NULL;
        image_uint16[i] = NULL;
    }
}

// virtual
nms_SEM_ui::~nms_SEM_ui (void)
{
    if (sem) {
        delete sem;
    }
}

// static
int nms_SEM_ui::drawGreyscaleWindow(const ImageViewerDisplayData & /*data*/, 
      void *ud)
{
    nms_SEM_ui *me = (nms_SEM_ui *)ud;
    me->image_viewer->drawImage(me->image_window_id);
    return 0;
}

int nms_SEM_ui::updateDisplays()
{

    return 0;
}

int nms_SEM_ui::mainloop(const struct timeval * /*timeout*/)
{
    sem->mainloop();
    updateDisplays(); // if displayed data changed in the previous line
			// then it gets sent to display state here
    image_viewer->mainloop();
    return 0;
}

// static
void nms_SEM_ui::handle_acquire_image(vrpn_int32 _newval, void *_userdata)
{
    printf("acquire image changed to %d\n", _newval);
    if (!_newval) return;
    nms_SEM_ui *me = (nms_SEM_ui *)_userdata;
    printf("requesting scan\n");
    me->sem->requestScan(0); // clear the pipeline
    me->sem->setExternalScanControlEnable(1);
    me->sem->requestScan(5); // request a bunch to start with in
			// order to fill the pipeline between the server
			// and this program
    me->image_viewer->showWindow(me->image_window_id);
}

// static
void nms_SEM_ui::handle_resolution_change(vrpn_int32 _newval, void *_ud)
{
    printf("res changed to %d\n", _newval);
    nms_SEM_ui *me = (nms_SEM_ui *)_ud;
    if (_newval < 0 || _newval >= EDAX_NUM_SCAN_MATRICES){
       fprintf(stderr, "nms_SEM_ui: Error, resolution index out of range\n");
       return;
    }
    vrpn_int32 res_x, res_y;
    vrpn_int32 res_x2, res_y2;
    nmm_EDAX::indexToResolution(_newval, res_x2, res_y2);
    // only send a message if the last message we received indicated
    // a different value
    me->sem->getResolution(res_x, res_y);
    printf("resolution was (%d, %d), changed to (%d, %d)\n",
	res_x, res_y, 
	res_x2, res_y2);
    if (res_x != res_x2 ||
        res_y != res_y2){
         me->sem->setResolution(res_x2,
                                res_y2);
    }
    return;
}

// static
void nms_SEM_ui::handle_texture_display_change(vrpn_int32 _newval, 
					       void * /*_ud*/)
{   
    //nms_SEM_ui *me = (nms_SEM_ui *)_ud;
    if (_newval) {
        //disableOtherTextures(SEM);
        graphics->setTextureMode(nmg_Graphics::SEM_DATA,
                                 nmg_Graphics::REGISTRATION_COORD);
    }

    else {
      // this check is important to make sure we don't disable some other
      // mode which the graphics have switched to before this function
      // got called
      if (graphics->getTextureMode() == nmg_Graphics::SEM_DATA) {
        graphics->setTextureMode(nmg_Graphics::NO_TEXTURES,
				 nmg_Graphics::RULERGRID_COORD);
      }
    }

}

// static 
void nms_SEM_ui::handle_integration_change(vrpn_int32 _newval, void *_ud)
{
    printf("integr. changed to %d\n", _newval);
    nms_SEM_ui *me = (nms_SEM_ui *)_ud;
    vrpn_int32 time_nsec;
    me->sem->getPixelIntegrationTime(time_nsec);
    if (time_nsec != _newval)
        me->sem->setPixelIntegrationTime(_newval);
}

// static 
void nms_SEM_ui::handle_delay_change(vrpn_int32 _newval, void *_ud)
{
    printf("delay changed to %d\n", _newval);
    nms_SEM_ui *me = (nms_SEM_ui *)_ud;
    vrpn_int32 time_nsec;
    me->sem->getInterPixelDelayTime(time_nsec);
    if (time_nsec != _newval)
        me->sem->setInterPixelDelayTime(_newval);
}

// static
void nms_SEM_ui::handle_window_visibility_change(vrpn_int32 _newval, void *_ud)
{
    //printf("closing window, %d\n", _newval);
    nms_SEM_ui *me = (nms_SEM_ui *)_ud;
    if (_newval == 0) {
        me->image_viewer->hideWindow(me->image_window_id);
    } else {
	me->image_viewer->showWindow(me->image_window_id);
    }
    return;
}

// static
void nms_SEM_ui::handle_device_change(void *ud,
           const nmm_Microscope_SEM_ChangeHandlerData &info)
{
  vrpn_int32 res_x, res_y;
  vrpn_int32 time_nsec;
  void *scanlineData;
  vrpn_int32 start_x, start_y, dx,dy, line_length, num_fields, num_lines;
  nmb_PixelType pix_type;
  int i,j;

  // timing stuff
  static int frame_count = 0;
  int new_frame_frequency = 0;
  static struct timeval start_time;
  struct timeval now;
  gettimeofday(&now, NULL);
  static int frame_frequency = 0;
  static int ready_to_print;
  static int print_count = 0;
  int dt = now.tv_sec - start_time.tv_sec;
  if (frame_count == 0) {
    gettimeofday(&start_time, NULL);
    dt = 0;
  } else if (dt != 0) {
    new_frame_frequency = frame_count/dt;
  }

  nms_SEM_ui *me = (nms_SEM_ui *)ud;
  switch(info.msg_type) {
    case nmm_Microscope_SEM::REPORT_RESOLUTION:
        info.sem->getResolution(res_x, res_y);
        fprintf(stderr, "SEM Resolution change: %d, %d\n", res_x, res_y);
        me->image_viewer->setWindowImageSize(me->image_window_id, res_x, res_y);
        i = nmm_EDAX::resolutionToIndex(res_x, res_y);
        if (i < 0) {
	   fprintf(stderr, "Error, resolution unexpected\n");
// this will possibly cause an infinite loop so I won't do it but we might
// want to make it try this at least once
//           info.sem->setResolution(me->d_matrixSizeX[me->sem_resolution],
//                                  me->d_matrixSizeY[me->sem_resolution])
	   break;
        }
        me->sem_resolution = i;
        frame_count = 0;
      break;
    case nmm_Microscope_SEM::REPORT_PIXEL_INTEGRATION_TIME:
        info.sem->getPixelIntegrationTime(time_nsec); 
        printf("REPORT_PIXEL_INTEGRATION_TIME: %d\n",time_nsec);      
        me->pixel_integration_time_nsec = time_nsec;
      break;
    case nmm_Microscope_SEM::REPORT_INTERPIXEL_DELAY_TIME:
        info.sem->getInterPixelDelayTime(time_nsec);
        printf("REPORT_INTERPIXEL_DELAY_TIME: %d\n", time_nsec);
        me->inter_pixel_delay_time_nsec = time_nsec;
      break;
    case nmm_Microscope_SEM::SCANLINE_DATA:
        info.sem->getScanlineData(start_x, start_y, dx, dy, line_length,
                                  num_fields, num_lines, 
                                  pix_type, &scanlineData);

        info.sem->getResolution(res_x, res_y);
        if (start_y + num_lines*dy > res_y || line_length*dx != res_x) {
	   fprintf(stderr, "SCANLINE_DATA, dimensions unexpected\n");
	   fprintf(stderr, "  got (%d,[%d-%d]), expected (%d,%d)\n",
                              line_length*dx, start_y, start_y + dy*(num_lines),
				res_x, res_y);
	   break;
        }
        if (!(me->no_graphics_update)) {
          int x, y;
          x = start_x;
          y = start_y;
          vrpn_uint8 *uint8_data = (vrpn_uint8 *)scanlineData;
          vrpn_uint16 *uint16_data = (vrpn_uint16 *)scanlineData;
          vrpn_float32 *float32_data = (vrpn_float32 *)scanlineData;
          switch(pix_type) {
            case NMB_UINT8:
	      me->image_viewer->setValueRange(me->image_window_id, 0.0, 255.0);
              for (i = 0; i < num_lines; i++) {
                x = start_x;
                for (j = 0; j < line_length; j++) {
                   me->image_viewer->setValue(me->image_window_id,
                        x, y, 
                        (double)(uint8_data[(i*line_length+j)*num_fields]));
                   x += dx;
                }
                y += dy;
              }
              break;
            case NMB_UINT16:
              me->image_viewer->setValueRange(me->image_window_id,0.0,65535.0);
              for (i = 0; i < num_lines; i++) {
                x = start_x;
                for (j = 0; j < line_length; j++) {
                   me->image_viewer->setValue(me->image_window_id,
                        x, y, 
                        (double)(uint16_data[(i*line_length+j)*num_fields]));
                   x += dx;
                }
                y += dy;
              }
              break;
            case NMB_FLOAT32:
              for (i = 0; i < num_lines; i++) {
                x = start_x;
                for (j = 0; j < line_length; j++) {
                   me->image_viewer->setValue(me->image_window_id,
                        x, y, 
                        (double)(float32_data[(i*line_length+j)*num_fields]));
                   x += dx;
                }
                y += dy;
              }
              break;

          }
          

          // when we get the end of an image restart the scan
          // and redraw the window
          if (start_y+num_lines == res_y) {
            me->image_viewer->dirtyWindow(me->image_window_id);
          }
          // this function creates plane data so it would probably be more
          // appropriate in a callback belonging to the dataset object
          me->updateSurfaceTexture(start_x, start_y, dx, dy, line_length,
               num_fields, num_lines, pix_type, scanlineData);

        } // end if !no_graphics_update
        if (start_y+num_lines == res_y) {
            if (me->sem_acquire_continuous){
                info.sem->requestScan(1);
            } else {
                info.sem->requestScan(0);
                info.sem->setExternalScanControlEnable(0);
            }
            frame_count++;
	}
        if (ready_to_print && (dt % 4 == 3)){
             frame_frequency = new_frame_frequency;
             printf("%d\n", frame_frequency);
             ready_to_print = 0;
             print_count++;
        } else if (dt % 4 != 3){
             ready_to_print = 1;
        }
      break;
    default:
        printf("nms_SEM_ui: Warning, unknown message type: %d\n", info.msg_type);
      break;
  }
}

int nms_SEM_ui::updateSurfaceTexture(
    int /*start_x*/, int start_y,
    int /*dx*/, int dy,
    int /*line_length*/,
    int /*num_fields*/,
    int num_lines,
    nmb_PixelType pix_type,
    void * data)
{
  // first look to see if we have already created an image for the current
  // resolution
  vrpn_int32 res_x, res_y;
  int res_index;
  sem->getResolution(res_x, res_y);
  res_index = nmm_EDAX::resolutionToIndex(res_x, res_y);
  if (res_index < 0) {
     fprintf(stderr, "Error, resolution not found (%d,%d)\n", res_x, res_y);
  }
  char image_name[128];

  int i,y;
  y = start_y;

  if (pix_type == NMB_UINT8) {
    sprintf(image_name, "SEM_DATA08_%dx%d", res_x, res_y);
    if (!image_uint8[res_index]) {
       image_uint8[res_index] = new nmb_ImageArray(image_name,
                                "ADC", res_x, res_y, NMB_UINT8);
       dataset->dataImages->addImage(image_uint8[res_index]);
    }
    for (i = 0; i < num_lines; i++){
      image_uint8[res_index]->setLine(y, data);
      y += dy;
    }   
  } else if (pix_type == NMB_UINT16) {
    sprintf(image_name, "SEM_DATA16_%dx%d", res_x, res_y);
    if (!image_uint16[res_index]) {
       image_uint16[res_index] = new nmb_ImageArray(image_name,
                                "ADC", res_x, res_y, NMB_UINT16);
       dataset->dataImages->addImage(image_uint16[res_index]);
    }
    for (i = 0; i < num_lines; i++){   
      image_uint16[res_index]->setLine(y, data);
      y += dy;
    }
  } else {
    fprintf(stderr, "nms_SEM_ui::updateSurfaceTexture:"
            " Error: can't handle pixel type\n");
    return -1;
  }

  // if we've just got the last scanline in the image then tell graphics
  // to update the whole image, we probably want to do this update more
  // frequently if possible to display the first few lines before all the
  // scanlines have come in but once per data frame is reasonable for now
  if (y == res_y) {
      graphics->updateTexture(nmg_Graphics::SEM_DATA, image_name,
        0, 0, res_x, res_y);
  }
  return 0;
}

int nms_SEM_ui::set_tcl_callbacks()
{
   sem_acquire_image.addCallback(handle_acquire_image, this);
   sem_resolution.addCallback(handle_resolution_change, this);
   display_texture.addCallback(handle_texture_display_change, this);
   pixel_integration_time_nsec.addCallback(handle_integration_change, this);
   inter_pixel_delay_time_nsec.addCallback(handle_delay_change, this);
   sem_window_open.addCallback(
                              handle_window_visibility_change, this);
   return 0;
}
