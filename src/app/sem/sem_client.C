#include "nmm_Microscope_SEM_Remote.h"
#include "imageViewer.h"
#include <limits.h>


ImageViewer *viewer;
int main_win;
nmm_Microscope_SEM_Remote *sem;

int drawMainWin(const ImageViewerDisplayData &data, void *ud);
int handleMainWinEvent(const ImageViewerWindowEvent &ivwe, void *ud);
void idlefunc();
void handle_sem_message(void *userdata, 
	const nmm_Microscope_SEM_ChangeHandlerData &info);

static int use_viewer = vrpn_TRUE;
static int time_to_quit = vrpn_FALSE;

int main(int argc, char **argv)
{
  if (use_viewer) {
#ifdef V_GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    viewer = ImageViewer::getImageViewer();
    viewer->init(NULL);
    main_win = viewer->createWindow(NULL, 10, 10, 128, 100, "SEM client",
         (int)GL_UNSIGNED_BYTE);
    viewer->showWindow(main_win);
    viewer->setWindowImageSize(main_win, 128, 100);
    viewer->setValueRange(main_win, 0.0, (double)UCHAR_MAX);
    viewer->setWindowDisplayHandler(main_win, drawMainWin, NULL);
    viewer->setWindowEventHandler(main_win, handleMainWinEvent, NULL);
#ifdef V_GLUT
    glutIdleFunc(idlefunc);
#endif
  }
  sem = new nmm_Microscope_SEM_Remote("SEM@cobalt-cs.cs.unc.edu");

  if (!(sem->connected())) {
    fprintf(stderr, "Error, can't connect to microscope, exiting\n");
    return -1;
  }
  
  sem->registerChangeHandler(NULL, handle_sem_message);
  sem->setResolution(256, 200);
  sem->requestScan(0);
  sem->requestScan(2);
#ifdef V_GLUT
  if (use_viewer) {
    glutMainLoop();
  } else {
    while(1){
      sem->mainloop();
	if (!(sem->connected())){
		printf("not connected\n");
	}
    }
  }
#else
  while(1){
	sem->mainloop();
        if (use_viewer);
          viewer->mainloop();
	if (!(sem->connected())){
		printf("not connected\n");
	}
  }
#endif
  return 0;
}

void idlefunc() {
  sem->mainloop();
	if (!(sem->connected())){
		printf("not connected\n");
	}
}

int drawMainWin(const ImageViewerDisplayData &data, void *ud)
{
        viewer->drawImage(main_win);
        return 0;
}

int handleMainWinEvent(const ImageViewerWindowEvent &ivwe, void *ud)
{
        switch(ivwe.type) {
        case KEY_PRESS_EVENT:
                switch(ivwe.keycode){
                case 'z':
                  
                        break;
                case 'q':
                        time_to_quit = vrpn_TRUE;
                        //exit(0);
                        break;
                default:
                        break;
                }
                break;
        default:
                break;
        }

        return 0;
}


void handle_sem_message(void *userdata,
        const nmm_Microscope_SEM_ChangeHandlerData &info)
{
  vrpn_int32 start_x, start_y, dx,dy, line_length, num_fields, num_lines;
  vrpn_float32 *data;
  vrpn_uint8 *data_uint8;
  static vrpn_int32 res_x, res_y;

  int x = 0;
  int y = 0;
  int i;

  static struct timeval first_time;
  struct timeval now;
  gettimeofday(&now, NULL);
  static int frame_count = -1;
  static int frame_frequency = 0;
  static int ready_to_print;
  static int print_count = 0;
  int new_frame_frequency = 0;
  int dt = now.tv_sec - first_time.tv_sec;
  if (frame_count == -1) {
	  gettimeofday(&first_time, NULL);
  } else if (dt != 0){
      new_frame_frequency = frame_count/dt;
  }

  switch (info.msg_type) {
    case nmm_Microscope_SEM::REPORT_RESOLUTION:
	  info.sem->getResolution(res_x, res_y);
	  printf("Resolution: %d, %d\n", res_x, res_y);
          if (use_viewer)
	    viewer->setWindowImageSize(main_win, res_x, res_y);
	  frame_count = -1;
      break;
    case nmm_Microscope_SEM::REPORT_PIXEL_INTEGRATION_TIME:
      printf("REPORT_PIXEL_INTEGRATION_TIME\n");
      break;
    case nmm_Microscope_SEM::REPORT_INTERPIXEL_DELAY_TIME:
      printf("REPORT_INTERPIXEL_DELAY_TIME\n");
      break;
    case nmm_Microscope_SEM::WINDOW_LINE_DATA:
	  info.sem->getWindowLineData(start_x, start_y, dx, dy, line_length,
		  num_fields, &data);
	  if (start_y == 0)
		  frame_count++;
	  if (ready_to_print && (dt % 4 == 3)){
		  frame_frequency = new_frame_frequency;
		  printf("%d\n", frame_frequency);
		  ready_to_print = 0;
		  print_count++;
/*
		  if (print_count == 5){
			info.sem->setResolution(128, 100);
		  } else if (print_count == 10) {
			info.sem->setResolution(256, 200);
		  } else if (print_count == 15) {
			info.sem->setResolution(512, 400);
		  }
*/
	  }
	  else if (dt % 4 != 3){
          ready_to_print = 1;
	  }
	  x = start_x;
	  y = start_y;
/*
	  for (i = 0; i < line_length; i++){
		double val = (double)data[i*num_fields];
		viewer->setValue(main_win, x, y, val);
		x += dx;
		y += dy;
	  }
*/
	  if (start_y == res_y-1){
		//printf("image done\n");
		info.sem->requestScan(1);
                if (use_viewer)
		  viewer->dirtyWindow(main_win);
	  }

      break;
    case nmm_Microscope_SEM::SCANLINE_DATA:
	  //printf("got scanline\n");
          info.sem->getScanlineData(start_x, start_y, dx, dy, line_length,
                num_fields, num_lines, &data_uint8);
          if (start_y == 0)
                  frame_count++;
          if (ready_to_print && (dt % 4 == 3)){
                  frame_frequency = new_frame_frequency;
                  printf("%d\n", frame_frequency);
                  ready_to_print = 0;
                  print_count++;
/*
                  if (print_count == 5){
                        info.sem->setResolution(128, 100);
                  } else if (print_count == 10) {
                        info.sem->setResolution(256, 200);
                  } else if (print_count == 15) {
                        info.sem->setResolution(512, 400);
                  }
*/
          }
          else if (dt % 4 != 3){
          ready_to_print = 1;
          }
          x = start_x;
          y = start_y;

          for (i = 0; i < num_lines; i++){
              if (use_viewer)
                viewer->setScanline(main_win, y, (data_uint8+line_length*i));
              y += dy;
          }

/*
          for (i = 0; i < line_length; i++){
                double val = (double)data_uint8[i*num_fields];
                viewer->setValue(main_win, x, y, val);
                x += dx;
                y += dy;
          }
*/
          if (start_y + num_lines == res_y){
			  if (frame_count % 16 == 0){
                //printf("image done\n");
			  }
              info.sem->requestScan(1);
              if (use_viewer)
                  viewer->dirtyWindow(main_win);
          }
      break;
    default:
      printf("unknown message type: %d\n", info.msg_type);
      break;
  }
}
