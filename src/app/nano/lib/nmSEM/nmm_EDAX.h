#define EDAX_ERROR (-1)
#define EDAX_OK (0)

#define EDAX_TRUE (1)
#define EDAX_FALSE (0)

#define EDAX_READ (1)
#define EDAX_WRITE (2)

// for SgSetRetrace:
#define EDAX_DEFAULT_HORZ_RETRACE (10)
#define EDAX_DEFAULT_VERT_RETRACE (10)

// for SgSetScanDir:
#define EDAX_DIR_NORMAL (0)
#define EDAX_DIR_MIRRORED (1)

// for SetupDACParams:
#define EDAX_MIN_GAIN (0)
#define EDAX_MAX_GAIN (32767)
#define EDAX_MIN_OFFSET (-32766)
#define EDAX_MAX_OFFSET (32767)

#define EDAX_DEFAULT_X_DAC_GAIN ((EDAX_MAX_GAIN - EDAX_MIN_GAIN)/2)
#define EDAX_DEFAULT_Y_DAC_GAIN EDAX_DEFAULT_X_DAC_GAIN
#define EDAX_DEFAULT_Z_ADC_GAIN EDAX_DEFAULT_X_DAC_GAIN
#define EDAX_DEFAULT_X_DAC_OFFSET ((EDAX_MAX_OFFSET-EDAX_MIN_OFFSET)/2)
#define EDAX_DEFAULT_Y_DAC_OFFSET EDAX_DEFAULT_X_DAC_OFFSET
#define EDAX_DEFAULT_Z_ADC_OFFSET EDAX_DEFAULT_X_DAC_OFFSET

// for SgSetVideoPolarity
#define EDAX_POLARITY_NORMAL (0)
#define EDAX_POLARITY_INVERTED (1)

#define EDAX_NUM_INPUT_CHANNELS (16)
#define EDAX_ADC1 (0)
#define EDAX_ADC2 (1)
#define EDAX_ADC3 (2)
#define EDAX_ADC4 (3)
#define EDAX_DIGITAL1 (4)
#define EDAX_DIGITAL2 (5)
#define EDAX_DIGITAL3 (6)
#define EDAX_BITINPUT1 (7)
#define EDAX_EDS_CROSS_PORT (8)
#define EDAX_BITINPUT2 (9)

// for SgSetMaxSpan:
#define EDAX_DEFAULT_MAX_X_SPAN (65535)
#define EDAX_DEFAULT_MAX_Y_SPAN (51200)

// for SetupSgColl, but affects scan generator settings too
#define EDAX_NUM_SCAN_MATRICES (7)
#define EDAX_SCAN_MATRIX_X {64, 128, 256, 512, 1024, 2048, 4096}
#define EDAX_SCAN_MATRIX_Y {50, 100, 200, 400, 800,  1600, 3200}
#define EDAX_DEFAULT_SCAN_MATRIX (2)	// (256 x 200)

#define EDAX_ADC1_ID (1)
#define EDAX_ADC2_ID (2)
#define EDAX_ADC3_ID (3)
#define EDAX_ADC4_ID (4)
#define EDAX_DIGITAL1_ID (5)
#define EDAX_DIGITAL2_ID (6)
#define EDAX_DIGITAL3_ID (7)
#define EDAX_BITINPUT1_ID (8)
#define EDAX_EDS_CROSS_PORT_ID (9)
#define EDAX_BITINPUT2_ID (10)

// for SetSgScan:
#define EDAX_FULL_SCAN (1)
#define EDAX_REDUCED_RASTER (3)
#define EDAX_SPOT_MODE (5)
