#include "mf.h"

#include "microscape.h"  // for #defines

/** \file butt_mode.c
Button bit values returned by vrpn
Arranged like this:
     0   1   2   3
 4   5   6   7   8   9
10  11  12  13  14  15
16  17  18  19  20  21
22  23  24  25  26  27
    28  29  30  31
 */

/// 0 is the trigger on the phantom, duplicated on button box
extern const int TRIGGER_BT = 0;
extern const int NULL1_BT    = 1;
extern const int NULL2_BT    = 2;
extern const int NULL3_BT     = 3;
extern const int GRAB_BT    = 4;
extern const int LIGHT_BT    = 5;
extern const int SCALE_UP_BT  = 6;
extern const int TOUCH_BT    = 7;
extern const int COMMIT_BT  = 8;
extern const int NULL9_BT     = 9;
extern const int CENTER_BT    = 10;
extern const int MEASURE_BT    = 11;
extern const int SCALE_DN_BT  = 12;
extern const int SELECT_BT    = 13;
extern const int CANCEL_BT  = 14;
extern const int NULL15_BT    = 15;
extern const int NULL16_BT    = 16;
extern const int TOUCH_STORED_BT    = 17;
extern const int NULL18_BT    = 18;
extern const int XY_LOCK_BT = 19;
extern const int NULL20_BT    = 20;

/** allow the user to select a mode with a button press.
 * mode <-> button defined above.
 */
int butt_mode(int *eventList, int *user_mode)
  {
  int		mode_change = 0;

  if( PRESS_EVENT == eventList[TOUCH_BT] ) {
	  *user_mode = USER_PLANEL_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[TOUCH_STORED_BT] ) {
	  *user_mode = USER_PLANE_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[GRAB_BT] ) {
	  *user_mode = USER_GRAB_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[SCALE_UP_BT] ) {
	  *user_mode = USER_SCALE_UP_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[SCALE_DN_BT] ) {
	  *user_mode = USER_SCALE_DOWN_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[LIGHT_BT] ) {
	  *user_mode = USER_LIGHT_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[SELECT_BT] ) {
	  *user_mode = USER_SERVO_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[MEASURE_BT] ) {
	  *user_mode = USER_MEASURE_MODE;
	  mode_change = 1;
	  }

  return mode_change;
  }
