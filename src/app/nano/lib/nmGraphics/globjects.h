#ifndef GLOBJECTS_H
#define GLOBJECTS_H

class Position_list;  // from Position.h

// Function Prototypes

//   Called in openGL.c
extern int myworld (void);
extern int make_red_line (const float a [], const float b []);
extern int make_green_line (const float a [], const float b []);
extern int make_blue_line (const float a [], const float b []);
extern int make_selected_region_marker (float x_min, float y_min, float x_max,
				        float y_max);
extern int make_green_line(const float a[], const float b[]);

//   Called in nmg_GraphicsImpl.c
extern int replaceDefaultObjects (void);
extern int make_aim (const float a [], const float b []);
extern int clear_world_modechange (int mode);
extern int init_world_modechange (int mode, int style);
extern int make_sweep (const float a [], const float b [],
		const float c [], const float d [] );
extern int make_rubber_corner ( float, float, float, float);
extern void position_sphere (float, float, float);
extern int mysphere(void * data);
extern void enableCollabHand (int);
extern void enableScanlinePositionDisplay(const int);

//extern int make_rubber_line_point (const float [2][3],
extern int make_rubber_line_point (const PointType[2],
                                   Position_list *);
extern int make_rubber_line_point (const PointType top, const PointType bot,
				   Position_list * p, int index);
extern int make_slow_line_3d_marker(const float point[2][3], Position_list * p);
// extern int make_slow_line_3d_marker(const float point[3]);
  // First parameter is the top and bottom of the marker line
  // (at the endpoint of the most recent segment?),
  // second parameter are the global endpoints of the rubber line,
  // third is the position list to add this point to.
extern void empty_rubber_line (Position_list *);
extern void empty_rubber_line (Position_list *, int);

extern int initialize_globjects (const char * fontName = NULL);

#endif  // GLOBJECTS_H
