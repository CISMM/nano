#ifndef GLOBJECTS_H
#define GLOBJECTS_H

#ifndef NMB_DECORATION_H
#include <nmb_Decoration.h>  // for nmb_LocationInfo
#endif

class Position_list;  // from Position.h
class nmg_State;
//class nmm_Microscope_Remote;

class nmg_haptic_graphics;

// Function Prototypes

//   Called in openGL.c
extern int myworld (nmg_State * state);
extern int make_red_line (nmg_State * state, 
                          const float a [], const float b []);
extern int make_green_line (nmg_State * state, 
                            const float a [], const float b []);
extern int make_blue_line (nmg_State * state, 
                           const float a [], const float b []);

extern int make_ds_sphere_axis(nmg_State * state, const q_type rot);
extern int make_feelPlane(void *data);

extern q_vec_type fp_origin_j, fp_normal_j;
extern int config_feelPlane_temp;

//   Called in nmg_GraphicsImpl.c
extern int make_aim (const float a [], const float b []);
extern int clear_world_modechange (nmg_State * state,
                                   int mode, int style, int tool_param);
extern int init_world_modechange (nmg_State * state, 
                                  int mode, int style, int tool_param);
extern int make_sweep (nmg_State * state, const float a [], const float b [],
		const float c [], const float d [] );
extern int make_rubber_corner ( nmg_State * state, 
                                float, float, float, float, int);
extern int make_region_box ( nmg_State * state, 
                             float, float, float, float, float, int);
extern int move_cross_section ( nmg_State * state, int, int, 
                             float, float, float, float, float, int);
extern int hide_cross_section(nmg_State * state, int id );
extern void position_sphere (nmg_State * state, float, float, float);
extern int mysphere(void * data);
extern void enableCollabHand (nmg_State * state, int);
extern int make_collab_hand_icon(double pos[], double quat[], int mode);
extern void enableScanlinePositionDisplay(nmg_State * state, const int);

//extern int make_rubber_line_point (const float [2][3],
extern int make_rubber_line_point (nmg_State * state, const PointType[2],
                                   Position_list *);
extern int make_rubber_line_point (nmg_State * state, 
                                   Position_list * p, int index);
extern int make_slow_line_3d_marker(const float point[2][3], Position_list * p);
// extern int make_slow_line_3d_marker(const float point[3]);
  // First parameter is the top and bottom of the marker line
  // (at the endpoint of the most recent segment?),
  // second parameter are the global endpoints of the rubber line,
  // third is the position list to add this point to.
extern void empty_rubber_line (Position_list *);
extern void empty_rubber_line (Position_list *, int);
extern int spm_render_mark (const nmb_LocationInfo &, void *);


extern int replaceDefaultObjects (nmg_State * state);
extern int initialize_globjects ( nmg_State * state, const char * fontName = NULL);


//JM from TCH branch 11/02
extern void enableFeelGrid (nmg_State * state,int on);
extern void enableFeelPlane (nmg_haptic_graphics * haptic_graphics, int on);

#endif  // GLOBJECTS_H
