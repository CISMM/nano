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
extern const int MENU_BT    = 1;
extern const int FORC_BT    = 2;
extern const int ALL_BT     = 3;
extern const int GRAB_BT    = 4;
extern const int SCALUP_BT  = 5;
extern const int SCALDN_BT  = 6;
extern const int SLCT_BT    = 7;
extern const int LITE_BT    = 8;
extern const int MEAS_BT    = 9;
extern const int NORM_BT    = 10;
extern const int FEEL_BT    = 11;
extern const int ZAG_BT     = 12;
extern const int COMB_BT    = 13;
extern const int LOCK_BT    = 14;  //lock button in sweep mode (qliu 6/29/95) 
extern const int CAND_BT    = 15;
extern const int CENT_BT    = 16;
extern const int CLEAR_BT   = 17;
extern const int CPNL_BT    = 18;
extern const int X_CPNL_BT  = 19;
extern const int XY_LOCK_BT = 20;
extern const int PH_RSET_BT = 21; // reset phantom
extern const int COMMIT_BT  = 22;
extern const int CANCEL_BT  = 23;
extern const int SLINE_BT   = 24;
extern const int NULL1_BT   = 25;
extern const int NULL2_BT   = 26;
extern const int NULL3_BT   = 27;
extern const int NULL4_BT   = 28;
extern const int NULL5_BT   = 29;
extern const int NULL6_BT   = 30;
extern const int NULL7_BT   = 31;

/** allow the user to select a mode with a button press.
 * mode <-> button defined in butt_mode.h.
 * all buttons select a specific mode.
 */
int butt_mode(int *eventList, int *user_mode)
  {
  int		mode_change = 0;
  //static int	last_mode = USER_GRAB_MODE;

  if( PRESS_EVENT == eventList[NORM_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_BLUNT_TIP_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[FEEL_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_PLANEL_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[ZAG_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_SWEEP_MODE;
	  mode_change = 1;
	  }
  if ( PRESS_EVENT == eventList[SLINE_BT] ) {
	  *user_mode = USER_SCANLINE_MODE;
          mode_change = 1;
          }
  if( PRESS_EVENT == eventList[CAND_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_PLANE_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[GRAB_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_GRAB_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[SCALUP_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_SCALE_UP_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[SCALDN_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_SCALE_DOWN_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[LITE_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_LIGHT_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[SLCT_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_SERVO_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[MEAS_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_MEASURE_MODE;
	  mode_change = 1;
	  }
  if( PRESS_EVENT == eventList[COMB_BT] ) {
	  //last_mode = *user_mode;
	  *user_mode = USER_COMB_MODE;
	  mode_change = 1;
	  }

  return mode_change;
  }
