#ifndef NMM_EDAX_H
#define NMM_EDAX_H

#include <vrpn_Types.h>

const int EDAX_ERROR = -1;
const int EDAX_OK = 0;

const int EDAX_TRUE = 1;
const int EDAX_FALSE = 0;

const int EDAX_READ = 1;
const int EDAX_WRITE = 2;

const int EDAX_BLANK_ON = 1;
const int EDAX_BLANK_OFF = 0;

const int EDAX_NORMAL_SCAN = 0;
const int EDAX_FAST_SCAN = 1;

const int EDAX_DATA_TRANSFER_NONE = 0;
const int EDAX_DATA_TRANSFER_BYTE = 1;
const int EDAX_DATA_TRANSFER_WORD = 2;

// for SgSetScanParams:
const int EDAX_DEFAULT_SCAN_TYPE = EDAX_FAST_SCAN;
const int EDAX_DEFAULT_DATA_TRANSFER = EDAX_DATA_TRANSFER_BYTE;

// for SgSetRetrace:
const int EDAX_DEFAULT_HORZ_RETRACE = 0;
const int EDAX_DEFAULT_VERT_RETRACE = 0;

// for SgSetScanDir:
const int EDAX_DIR_NORMAL = 0;
const int EDAX_DIR_MIRRORED = 1;

// for SetupDACParams:
const int EDAX_MIN_GAIN = 0;
const int EDAX_MAX_GAIN = 32767;
const int EDAX_MIN_OFFSET = -32766;
const int EDAX_MAX_OFFSET = 32767;

// by default we start with all values in the middle of their range
const int EDAX_DEFAULT_X_DAC_GAIN = ((EDAX_MAX_GAIN + EDAX_MIN_GAIN)/2);
const int EDAX_DEFAULT_Y_DAC_GAIN = EDAX_DEFAULT_X_DAC_GAIN;
const int EDAX_DEFAULT_Z_ADC_GAIN = EDAX_DEFAULT_X_DAC_GAIN;
// (6000,7000) seems to match the Hitachi settings better than (0,0)
const int EDAX_DEFAULT_X_DAC_OFFSET = 6000;//((EDAX_MAX_OFFSET + EDAX_MIN_OFFSET)/2);
const int EDAX_DEFAULT_Y_DAC_OFFSET = 7000;//((EDAX_MAX_OFFSET + EDAX_MIN_OFFSET)/2);
const int EDAX_DEFAULT_Z_ADC_OFFSET = ((EDAX_MAX_OFFSET + EDAX_MIN_OFFSET)/2);

// for SgSetVideoPolarity
const int EDAX_POLARITY_NORMAL = (0);
const int EDAX_POLARITY_INVERTED = (1);

const int EDAX_NUM_INPUT_CHANNELS =(16);
const int EDAX_ADC1 =(0);
const int EDAX_ADC2 =(1);
const int EDAX_ADC3 =(2);
const int EDAX_ADC4 =(3);
const int EDAX_DIGITAL1 =(4);
const int EDAX_DIGITAL2 =(5);
const int EDAX_DIGITAL3 =(6);
const int EDAX_BITINPUT1 =(7);
const int EDAX_EDS_CROSS_PORT =(8);
const int EDAX_BITINPUT2 =(9);

// for SgSetMaxSpan:
const int EDAX_DEFAULT_MAX_X_SPAN = (65536);//(4096) or (65536)
const int EDAX_DEFAULT_MAX_Y_SPAN = (51200);//(3200) or (51200)

// for SetupSgColl, but affects scan generator settings too
const int EDAX_NUM_SCAN_MATRICES =(7);
extern int EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES];
extern int EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES];
const int EDAX_DEFAULT_SCAN_MATRIX =(2);	// (256 x 200)

const int EDAX_ADC1_ID =(1);
const int EDAX_ADC2_ID =(2);
const int EDAX_ADC3_ID =(3);
const int EDAX_ADC4_ID =(4);
const int EDAX_DIGITAL1_ID =(5);
const int EDAX_DIGITAL2_ID =(6);
const int EDAX_DIGITAL3_ID =(7);
const int EDAX_BITINPUT1_ID =(8);
const int EDAX_EDS_CROSS_PORT_ID =(9);
const int EDAX_BITINPUT2_ID =(10);

// for SetSgScan:
const int EDAX_FULL_SCAN =(1);
const int EDAX_REDUCED_RASTER =(3);
const int EDAX_SPOT_MODE =(5);

// other
const int EDAX_DEFAULT_BLANK_MODE=(1); // blank between points enabled

// this is based on experiment: approximate time to execute SpMoveEx() as 
// observed on an oscilloscope
const double EDAX_MIN_POINT_DWELL_SEC=(100e-6);

class nmm_EDAX {
 public:
  static int resolutionToIndex(const int res_x, const int res_y);
  static int indexToResolution(const int id, int &res_x, int &res_y);
  /// finds the nearest valid integration time
  static void snapIntegrationTime_nsec(int &time_nsec, vrpn_bool preferLarger);
  /// finds the nearest valid interpixel delay time
  static void snapInterPixelDelayTime_nsec(int &time_nsec,
                                           vrpn_bool preferLarger);
  /// finds the nearest valid resolution
  static void snapResolution(int &res_x, int &res_y, vrpn_bool preferLarger);
};

#endif
