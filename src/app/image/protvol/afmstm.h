#ifndef __AFMSTM_H__
#define __AFMSTM_H__


#define ZDATA_USER_DEFINED 15
#define ZUNIT_USER_DEFINED 15
#define SCAN_OFF -1
#define SCAN_NOSCAN 0
#define SCAN_LINESCAN 1
#define SCAN_IMAGE 2
#define SCAN_IMAGE_XONLY 3
#define SCAN_SCOPE 4
#define SCAN_PRESCAN 5
#define PROBE_AFM 0
#define PROBE_STM 1
#define PROBE_TUNING 2
#define VOLTAGE_V 0
#define VOLTAGE_MV 1
#define DISTANCE_ANGS 0
#define DISTANCE_UM   1
#define SIG_NONE -1        
#define SIG_T_B 0
#define SIG_L_R 1
#define SIG_SUM 2
#define SIG_SENSOR_IN 3
#define SIG_LAT_SENSOR_IN 4
#define SIG_X_LIN 5
#define SIG_Y_LIN 6
#define SIG_Z_LIN 7
#define SIG_AUX1_IN 8
#define SIG_AUX2_IN 9
#define SIG_AUX5_IN 10
#define SIG_AUX6_IN 11
#define SIG_STM_CUR 12
#define SIG_ALT_SENS 13
#define SIG_Z_PIEZO 14
#define SIG_Z_BI_EN 15
#define SIG_AL0_EN 16
#define SIG_TMX2000_EXT1 17
#define SIG_TMX2000_FFM 18   
#define SIG_INT_CHECK 19
#define SIG_Z_ANALOG 20
#define SIG_IOU_BNC0 21
#define SIG_IOU_BNC1 22
#define SIG_IOU_BNC2 23
#define SIG_IOU_BNC3 24
#define SIG_AUX3_IN  25
#define SIG_AUX4_IN	 26
#define AL_NONE -1        
#define AL0     0
#define AL1     1
#define AL2     2
#define AL3     3
#define AL4     4
#define AL5     5
#define AL6     6
#define AL7     7
#define AL8     8
#define AL9     9
#define AL10    10
#define AL11    11
#define AL12    12
#define AL13    13
#define AL14    14
#define AL15    15
#define ADC_NONE    -1      
#define ADC_ANY     -2    
#define PMT_DAC_MIN 0x8000
#define PMT_DAC_MAX 0xFFFF
#define PMT_HVGAIN 100.0
#define BULB_DAC_MIN 0x7000
#define BULB_DAC_MAX 0xC000
#define BIG_NEGATIVE -9999999.0f
#define BIG_POSITIVE  9999999.0f
#define N_MAX_XYSCANRANGE 20
#define STEP_SCAN_IMAGE     0
#define STEP_SCAN_WAFER     1
#define CLEARANCE_COEFF 0.011f /* distance -> time conversion for Z motor */
#define MAX_STEP        100
#define N_MAX_DIGITAL_FILTERS 12
#define TRANS_UP 6
#define TRANS_DOWN 4
#define TRANS_LEFT (GetMonitorType() == 0? 2:8)
#define TRANS_RIGHT (GetMonitorType() == 0? 8:2)
#define TRANS_PY 8
#define TRANS_NY 2
#define TRANS_PX 4
#define TRANS_NX 6
#define TRANS_PRIMARY 0
#define TRANS_SECONDARY 1
#define TRANS_FINE_SPEED 0
#define TRANS_MEDIUM_SPEED 1
#define TRANS_COARSE_SPEED 2
#define SP_SCALE_FACTOR 1.0
#define HYSTER_OFF 0 /* linear coeffs */
#define HYSTER_SYSFILE 1 /* from system file for scanner */
#define HYSTER_CALIBRATION 2 /* use last loaded user-calibration file values */
#define HYSTER_NOCHANGE 3   /* for scan linearizer */
#define PRE_SCAN_RES 4
#define N_CAL_FACTOR 4
#define N_SCAN_RANGE 7
#define MODEL_STD           0       // discoverer AFM & STM probe holder
#define MODEL_STM           2       // discoverer STM (scanning tip)
#define MODEL_EXPLORER_AFM  3       // was TTAFM
#define MODEL_EXPLORER_STM  4
#define MODEL_UNIVERSAL_AFM 5       // was LSSFM
#define MODEL_SNOM          6       // Aurora
#define MODEL_OBSERVER_AFM  7
#define MODEL_OBSERVER_STM  8
#define MODEL_TOPOCRON_AFM  9       // these are only used when reading in a scanner system file.
#define MODEL_TOPOCRON_STM  10      // are used to distinguish the types of scanner files (AFM,STM)
#define MODEL_TOPOCRON      12      // only used for old documents prior to v3.05
#define MODEL_ACCUREX_AFM   13
#define MODEL_ACCUREX_STM   14
#define Z_POLARITY_NON_INVERTED 1
#define Z_POLARITY_INVERTED 0    
#define NONCONTACT_NONE 0
#define NONCONTACT_USER_SELECTED 1
#define NONCONTACT_ON_STARTUP 2
#define ZADJUST_ENABLE_UP 1
#define ZADJUST_ENABLE_DOWN 2
#define MENU_COMMANDS_OBSERVER 1
#define MENU_SETUP_OBSERVER 2
#define DISABLE_MODE_MODULATIONZ    1
#define DISABLE_MODE_MODULATIONV    2
#define DISABLE_MODE_LAYERED_IMAGES 4
#define DISABLE_MODE_THERMAL        8
#define DISABLE_MODE_SEPM           0x10
#define DISABLE_MODE_AFM_CURRENT    0x20
#define ZMOTOR_NORMAL 0
#define ZMOTOR_INVERTED 1
#define ZMOTOR_1 1
#define ZMOTOR_3 3
#define MONITOR_STD 0
#define MONITOR_INVERTED 1
#define CALFILE_MTI 0
#define CALFILE_ZYGO 1
#define SCAN_NONE -1
#define SCAN_25UM_TRIPOD 0
#define SCAN_75UM_TRIPOD 1
#define SCAN_10UM_TUBE 2
#define SCAN_1UM_TUBE 3
#define LINEARIZER_NONE 0
#define LINEARIZER_ELECTROSTRICTIVE 1
#define LINEARIZER_NORM 2
#define NONCONTACT_GAIN_LOW  1
#define NONCONTACT_GAIN_HIGH 0
#define NONCONTACT_PHASE_MODE  0
#define NONCONTACT_AMP_MODE    1
#define  IO12_NO_BIAS		0
#define  IO12_TIP_BIAS		1
#define  IO12_SAMPLE_BIAS	2
#define  IO12_AMP_BIAS		3
#define SCAN_OFF -1
#define SCAN_NOSCAN 0
#define SCAN_LINESCAN 1
#define SCAN_IMAGE 2
#define SCAN_IMAGE_XONLY 3
#define SCAN_PRESCAN 5
#define MAX_IOMOD_FREQ_RANGES 8
#define SAMPLEPTS       200      /* number of samples to test per location */
#define HARDWARELINEARIZATION 1
#define SOFTWARELINEARIZATION 2           

#endif
