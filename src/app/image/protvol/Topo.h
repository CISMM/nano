#ifndef TOPO_H
#define TOPO_H

#include "stdlib.h"
#include "BCPlane.h"
#include "afmstm.h"

/*
 * Definitions for data types.
 * XXX This is copied from /nano/nano1/topo/spm40/document.h
 * other misc constants from /nano/nano1/topo/spm40/image.h
 */
#define HEIGHT                  (0)
#define CURRENT                 (1)
#define DEFLECTION              (2)
#define AUX                     (3)
#define FORCE_MODULATION_DATA   (8)
#define SWAP_HEIGHT             (10)
#define SWAP_CURRENT            (11)
#define SWAP_DEFLECTION         (12)
#define SWAP_AUX                (13)
#define DO_SWAP                 (10)

/* for iDataType member of _tScanParams (also used by DSP) */
#define ZDATA_NONE          -1
#define ZDATA_HEIGHT        0
#define ZDATA_ADC0          1       /* was ZDATA_CURRENT */
#define ZDATA_FFM           2
#define ZDATA_SPECT         3
#define ZDATA_SPECTV        4
#define ZDATA_ADC1          5
#define ZDATA_ADC2          6
#define ZDATA_TIPV          7       /* oScope, monitor of tip V */
#define ZDATA_DAC1          8       /* oScope, Spare DAC 1 */
#define ZDATA_DAC2          9       /* oScope, Spare DAC 2 */
#define ZDATA_ZPIEZOV       10      /* Z piezo in volts */
#define ZDATA_HEIGHT_ERROR  11      /* height + weighted error signal - weighted set point */
#define ZDATA_LINEARIZED_Z  12      /* linearized Z input */
#define ZDATA_FEEDBACK      13      /* either ADC0,ADC1, or ADC2, as defined in setup acquire */

/* for iDataMode member of _tScanParams (also used by DSP) */
#define D2_IMAGE                    0
#define D3_CITS                     1
#define D3_DITS                     2
#define D3_FIS                      3
#define D3_MFM                      4
#define D3_EFM                      5
#define D1_IV                       10
#define D1_IS                       11
#define D1_FS                       12
#define D1_MS                       13
#define D1_ES                       14
#define D1_ECHEM                    15
#define D1_ECHEM_LINE_AVG           16
#define D1_ECHEM_RUNNING_OUTPUT     17
/* for iDataDir member of _tScanParams (also used by DSP) */
#define DIR_FWD             0
#define DIR_REV             1

// for members of _tFileHeader and related string variables throughout the entire code
#define N_RELEASE               16
#define N_DATETIME              20
#define N_DESCRIPTION           40
#define N_FULLNAME              256

// for iGainZ and iGain members of _tScanParams
#define GAIN_LOW                0
#define GAIN_HI                 1

// for iCalibType member of _tScanParams
#define CALIB_OFF               0
#define CALIB_SYS               1
#define CALIB_USE               2
#define CALIB_LIN               3

// for iXUnitType member of _tDocumentInfo/_tDocument subclasses
#define XUNIT_NM                0
#define XUNIT_V                 1

// for iXYUnitType member of _tDocumentInfo/_tDocument subclasses
#define XYUNIT_NA               0
#define XYUNIT_ANG              1
#define XYUNIT_NM               2
#define XYUNIT_UM               3

// for iWorldUnitType member of _tDocumentInfo/_tDocument subclasses
#define ZUNIT_NA                0
#define ZUNIT_NM                1
#define ZUNIT_ANG               2
#define ZUNIT_UM                3
#define ZUNIT_NN_ANG            4           // DEAD for Now, AFM spectroscopy uses nA/Ang
#define ZUNIT_NA_V              5           // STM Spectroscopy V uses this
#define ZUNIT_V                 6           // +-5V absolute measurement
#define ZUNIT_NA_ABS            7           // +-32nA absolute measurement
#define ZUNIT_NA_ANG            8           // AFM and STM spectroscopy use this
#define ZUNIT_UA_ABS            9
#define ZUNIT_MA_ABS            10

// unit types for iSetpointUnits (units for fI, setpoint value)
#define SETPOINT_NA                 0
#define SETPOINT_FORCE              1
#define SETPOINT_NONCONTACT_PERCENT 2

#define max(x,y)        (((x)>(y))?(x):(y))
#define min(x,y)        (((x)<=(y))?(x):(y))
/* Fix to unix from PC stuff */

typedef int     INT;

/* Header file stuff */

#define N_MAX_GRAPHS            8
#define N_MAX_ECHEM_LEGS        100
#define N_MAX_EC_DATA           4000

typedef unsigned short  *PDATA;
typedef short           TOPOBOOL;

#ifndef _WIN32

typedef struct tagRECT{
	short left;
	short top;
	short right;
	short bottom;
} RECT;

#endif // ndef _WIN32

typedef struct  _tScanParamsLayered {
        float   fVzStart;       /* [1..4] voltage/distance start value*/
        float   fVzStop;        /* [5..8] voltage/distance stop value */
        float   fVzLimit;       /* [9..12] force limit */
        float   fVzArray;      /* [15..16] array of voltage/distance values
                                   (size determined by nLayers)  should be an Array pointer but can't read a pointer from file anyway*/
  /*        short   fVzArray;        should be a pointer value but can't read pointers from a file*/
        float   fVzSpeed1;      /* [17..20] speed of voltage/distance ramp
                                   (sample point) in V/sec or mircons/sec */
        float   fVzSpeed2;      /* [21..24] speed of voltage/distance ramp
                                   (pullback) in V/sec or mircons/sec */
        float   fVzSpeed3;      /* [25..28] speed of voltage/distance ramp
                                   (first sample point) in V/sec or micron/sec*/
        float   fVzSpeed4;      /* [29..32] speed of voltage/distance ramp
                                   (back into feedback) in V/sec or micron/sec*/
        float   fVzPullback;    /* [33..36] pullback distance (to get out of
                                   contact) */
        short   iLayers;        /* [37..38] number of layers */
        short   iHalfCycles;    /* [39..40] number of half cycles (forward = 1,
                                   forward+backward = 2) */
        short   iAvgPoint;      /* [41..42] averaging number per layer point */
        float   fDelayStart;    /* [43..46] time delay before first sample
                                   point in micro-sec */
        float   fDelaySample;   /* [47..50] time delay before each sample
                                   in micro-sec */
        float   fDelayPullback; /* [51..54] time delay after pullback in
                                   micro-sec */
        float   fDelayEstFeedbk;/* [55..58] time delay to re-establish feedback
                                   in micro-sec */
        TOPOBOOL    bFeedbkPoints;  /* [59..60] TRUE: enabled feedback between
                                   points */
        TOPOBOOL    bFeedbkCurves;  /* [61..62] TRUE: enabled feedback between
                                   curves */
        TOPOBOOL    bVzRelative;    /* [63..64] TRUE: voltage/distance values are
                                   relative */
        float   fModFreq;       /* [65..68] modulation frequency */
        float   fVzMod;         /* [69..72] voltage/distance modulation */
	short	iExtLayer;	/* [73..74] extracted layer */
	TOPOBOOL	bSpecialNCScan;  /* [75..76] special NC scanning */
        /*char    cFiller[180];    [77..256] fill bytes 
		officially it should be size 180 but I am putting it to 184 so I can
		handle 3.01 files more easily */
        char    cFiller[184];   /* [77..256] fill bytes */
} SCANPARAMSLAYER, *PSCANPARAMSLAYER;

typedef struct  _tScanParams {
        short   iDataType;              /* [1..2] raw data source (Z, sensor, external #1,...) */
        short   iDataDir;               /* [3..4] direction data collected */
        short   iDataMode;              /* [5..6] collection mode: (2D, CITS, DITS, FIS, MFM, EFM, FS, IV,...) */
        float   fScanZmax;              /* [7..10] Z DAC max: 0 ... 0xFFFF */
        float   fScanZmin;              /* [11..14] Z DAC min: 0 ... 0xFFFF */
        float   fScanXmax;              /* [15..18] X piezo resolution in points */
        float   fScanYmax;              /* [19..22] Y piezo resolution */
        float   fVtip;                  /* [23..26] STM tip voltage in the range -10.0 ... +10.0 volts */
        float   fI;                     /* [27..30] desired sensor feedback/tunneling current in nA */
        float   fVz;                    /* [31..34] Z piezo setpoint in the range 0 ... 440 volts */
        float   fRange;                 /* [35..38] scanned image range */
        float   fRate;                  /* [39..42] XY scan rate in unitType units/sec */
        short   iGain;                  /* [43..44] ADC gain flag */
        float   fPro, fInteg, fDer;     /* [45..56] PID parameters */
        short   iGainZ;                 /* [57..58] Z piezo gain flag */
        float   fRotation;              /* [59..62] XY scan rotation, radians */
        float   fModLevel;              /* [63..66] modulation mode: modulation in  angstroms */
        float   fAveraging;             /* [67..70] modulation mode: number of points to avg per image point */
        float   fSpCalFactor;           /* [71..74] modulation mode: */
        short   iCalibType;             /* [75..76] XY calibration type */
        short   iLaserIntensity;        /* [77..78] laser setting for AFM */
        unsigned short  iScaleFactorZ;  /* [79..80] z scale factor */
        unsigned short  iDACminX, iDACmaxX; /* [81..84] piezo scan location x */
        unsigned short  iDACminY, iDACmaxY; /* [85..88] piezo scan location y */
        char    cScanType[6];           /* [89..94] ASCII/binary separator with encoded scan type info (only used for pre Windows files) */
        short   iProbeType;             /* [95..96] probe type ( STM, AFM...) */
        short   iStageType;             /* [97..98] stage type ( STM, AFM, Aurora, Observer,...) */
        short   iCalFileSource;         /* [99..100] calibration file type used: ZYGO or MTI (from scanner SYSTEM file) */
        float   fOverscanX;             /* [101..104] overscan range in percent of scan range */
        float   fOverscanY;             /* [105..108] overscan range in percent of scan range */
	short           iSetPointUnits;     //  [109..110]  setpoint (fI) unit types
	float           fNcRegAmp;          //  [111..114]  register frequency of non contact
	short           iGainXY;            //  [115..116]  XY gain (0=low,1=high,2 = div 2, 3 = div 4...
	unsigned short  iOffsetX, iOffsetY; //  [117..120]  x and y piezo offset DAC values
	float           fHysteresisX[4];    //  [121..136]  X piezo hysteresis polynomial
	float           fHysteresisY[4];    //  [137..152]  Y piezo hysteresis polynomial
	unsigned short  iOffsetZ;           //  [153..154]  Z piezo offset DAC value
	float           fHysteresisZ[4];    //  [155..170]  Z piezo hysteresis polynomial
	float           fCrossTalkCoef;     //  [171..174]  xy crosstalk coefficient
	float           fSensorResponse;    //  [175..178]  sensor response in nA/nm
	float           fKc;                //  [179..182]  spring constant in N/m
	short           iCantileverType;    //  [183..184]  cantilever type
	char            szScannerSerialNumber[16]; // [185..200] scanner serial number
	short           iZlinearizer;       //  [201..202]  z linearizer type
	int             iADC;               //  [203..206]  adc used
	TOPOBOOL            bNonContact;           //  [207..208]  auto non_contact flag
	short           CantileverType;     //  [209..210]  0: Low Freq, 1: High Freq, 2:General Freq
	float           fDriveAmplitude;    //  [211..214]  Driving Amplitude
	float           fDriveFrequency;   //   [215..218]  Driving Frquency
	short           iNonContactMode;     // [219..220]  0: Amplitude 1: phase
	short           iNonContactPhase;   //  [221..222]  0, 90, 180, 270
	char            cFiller[148];      //    [223..256]  fill bytes to make struct a fixed length
	//technically cFiller should be [34] size but I'm leaving it at 148 to make 3.01 version files easier to parse
        SCANPARAMSLAYER scan3d;         // [257..512] scanning parameters
	char szStageType[64];    //  [513..576]  stage type from stages.ini
	char szStageName[64];    //  [577..640]  specific stage name from stages.ini
	char szStageText[64];     //  [641..704]
	unsigned short  iOldOffsetX, iOldOffsetY; //  [705..708]  x and y piezo offset DAC values
	unsigned short  iOldDACminX, iOldDACmaxX; //  [709..712]    piezo scan location x
	unsigned short  iOldDACminY, iOldDACmaxY; //  [713..716]    piezo scan location y
	short           iOldGainXY;            //  [717..718]  XY gain (0=low,1=high,2 = div 2, 3 = div 4...
	char          cFiller1[370];   //     [719..1024] 512 bytes filler add in ver3.07 layered imaging 
	 /*     370 here doesn't seem to match the inidcated range but I'll leave it at 370 */

} SCANPARAMS, *PSCANPARAMS;


typedef struct _tDocumentInfo { /* binary image information */
        INT     iRelease;               /* [1..4] parsed release number */
        INT     iOffset;                /* [5..8] byte offset to actual data */
        char    szRelease[N_RELEASE];   /* [9..24] release of software */
        char    szDatetime[N_DATETIME]; /* [25..44] date and time generated */
        char    szDescription[N_DESCRIPTION];   /* [45..84] description string*/
        float   fPosX[N_MAX_GRAPHS];    /* [85..116] position of curve in x */
        float   fPosY[N_MAX_GRAPHS];    /* [117..148] position of curve in y */
        short   iCurves;                /* [149..150] number of curves  */
        INT     iRows, iCols;           /* [151..158] number of data points */
        unsigned short iDACmax, iDACmin;/* [159..162] maxand min value in data*/
        float   fXmin, fXmax;           /* [163..170] scan distance in x */
        float   fYmin, fYmax;           /* [171..178] scan distance in y */
        float   fDACtoWorld;            /* [179..182] conversion factor from DAC units to physical units */
        float   fDACtoWorldZero;        /* [183..186] zero set point for physical units */
        unsigned short iDACtoColor;     /* [187..188] conversion factor from DAC units to color indices */
        unsigned short iDACtoColorZero; /* [189..190] zero set point for color indices */
        short   iWorldUnitType;         /* [191..192] physical unit type */
        short   iXYUnitType;            /* [193..194] physical unit type in x and y for display */
        char    szWorldUnit[10];        /* [195..204] string of physical units in z */
        char    szXYUnit[10];           /* [205..214] string of physical units in x and y (see iXYUnitType) */
        char    szRateUnit[10];         /* [215..224] string of scan rate unit*/
        short   iLayers;                /* [225..226] number of image layers */
        TOPOBOOL    bHasEchem;              /* [227..228] has Echem data */
        TOPOBOOL    bHasBkStrip;            /* [229..230] has bkStrip data */
        short   iPts[N_MAX_GRAPHS];     /* [231..246] datapoints per spectroscopy curve */
        short   iXUnitType;             /* [247..248] physical unit type in x for spectroscopy display */
        char    szXUnit[10];            /* [249..258] string of physical units in x (see iXUnitType) */
	TOPOBOOL            bHasAcqDisplay;                 //  [259..260]  has acquisition display parameters
	short           iTilt;                          //  [261..262]  acquisition tilt removal type
	short           iScaleZ;                        //  [263..264]  acquisition data range calculation type
	short           iFilter;                        //  [265..266]  acquisition filter type
	short           iShading;                       //  [267..268]  acquisition shading type
	double          dTiltC[8];                      //  [269..332]  acquisition tilt removal coefficients
	unsigned short  iDACDisplayZero;                //  [333..334]  z adjust zero point in DAC units
	unsigned short  iDACDisplayRange;               //  [335..336]  z adjust range in DAC units
	RECT            rRoi;                           //  [337..344]  active image selection
	/*char            cFiller[424];                   //  [344..768]  fill bytes to make struct a fixed length*/
	/* officially it should be 424 for a 4.0 file but leaving it 510 so I can handle 3.01 files easily */
	char            cFiller[510];                   //  [344..768]  fill bytes to make struct a fixed length
} DOCUMENTINFO, *PDOCUMENTINFO;

typedef struct _tTopoDocument {
        DOCUMENTINFO    sInfo;          /* [1..768] document information */
        SCANPARAMS      sScanParam;     /* [769..1280] scan parameters */
} TOPODOC, *PTOPODOC;

class TopoFile{
	private:
		DOCUMENTINFO sInfo;
		SCANPARAMS sScanParam;
		/*int nRelease;
		int nOffset;
	       		char *header;*/
                void parseReleaseString(char *release);
		int ParseData(int);
		int writeHeader(int);
		int writeData(int);
		int writeTopoFile(int);
		int writeDocInfo(int);
		int writeScanParam(int);
		int writeScanLayers(int);
		char *ParseDocInfo(char*);
		char *ParseScanParam(char*);
		char *ParseScanLayers(char*);
		char *BuildHeader();
		INT valid_header;
		int iSwap(char*);
		float fSwap(char*);
		double dSwap(char*);
		short sSwap(char*);
		unsigned short usFix(unsigned short *s);
		unsigned short usSwap(char*);
		void FixUpDocStruct();		//version 4.0 fix
	public:
		INT valid_grid;
		int nRelease;
		int nOffset;
		char* header;
		unsigned short *griddata;
		TopoFile(){
		  memset( &sInfo, 0, sizeof( DOCUMENTINFO));
		  memset( &sScanParam, 0, sizeof(SCANPARAMS));
		  header=NULL;
		  nRelease=0;
		  nOffset=0;
		  valid_header=0;
		  valid_grid=0;
		 } 
		int scaled_grid(int i, int j){
			float c;
			c=griddata[i*sInfo.iCols+j];
			c=((float)c-sInfo.iDACmin)/(sInfo.iDACmax-sInfo.iDACmin)*255.0;
			return (int)c;
		}	
		~TopoFile(){if(header!=NULL) delete header;
		            if(griddata!=NULL) free(griddata);	}
		int PrintDocInfo();
		int PrintScanParams();
		int PrintScanLayers();
		int PrintData();
		int readTopoFile(const char*);
		int writeTopoFile(char* filename);
		int writeTopoFile(FILE* handle);
                int ParseHeader(const char *header,long length);	      
		int ParseHeader(int);
		int TopoDataToGrid(BCGrid*,const char*);
		int GridToTopoData(BCGrid*);
		int GridToTopoData(BCGrid*,BCPlane*);
		void freeGridData(){ if(griddata!=NULL) delete griddata; valid_grid=0;}
		INT Rows(){ return sInfo.iRows;}
		INT Cols(){ return sInfo.iCols;}
};
#define NRELEASE 400
#define NOFFSET     256 + sizeof(TOPODOC)
#endif
