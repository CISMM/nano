/** \file butt_mode.h
Code to control the SGI button boxes. 
Defines which buttons cause selection of which modes.
*/

extern const int TRIGGER_BT;
extern const int NULL1_BT;
extern const int NULL2_BT;
extern const int NULL3_BT;
extern const int GRAB_BT;
extern const int LIGHT_BT;
extern const int SCALE_UP_BT;
extern const int TOUCH_BT;
extern const int COMMIT_BT;
extern const int NULL9_BT;
extern const int CENTER_BT;
extern const int MEASURE_BT;
extern const int SCALE_DN_BT;
extern const int SELECT_BT;
extern const int CANCEL_BT;
extern const int NULL15_BT;
extern const int NULL16_BT;
extern const int TOUCH_STORED_BT;
extern const int NULL18_BT;
extern const int XY_LOCK_BT;
extern const int Z_LOCK_BT;

extern int butt_mode(int *eventList, int *user_mode);
