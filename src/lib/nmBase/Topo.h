#ifndef _TOPO_
#define _TOPO_

#include "BCPlane.h"
class nmb_Image;
#include "afmstm.h"  // for another 15 #defines

#include <stdlib.h>  // free()
#include <nmb_Types.h>
    // architecture-independent sizes

/*
 * Definitions for data types.
 * XXX This is copied from topo/document.h
 * other misc constants from topo/image.h
 */
#define TF_HEIGHT                  (0)
#define TF_CURRENT                 (1)
#define TF_DEFLECTION              (2)
#define TF_AUX                     (3)
#define TF_FORCE_MODULATION_DATA   (8)
#define TF_SWAP_HEIGHT             (10)
#define TF_SWAP_CURRENT            (11)
#define TF_SWAP_DEFLECTION         (12)
#define TF_SWAP_AUX                (13)
#define TF_DO_SWAP                 (10)

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
#define ZDATA_IOMODAMP      15
#define ZDATA_IOMODPHASE    16
//#define ZDATA_USER_DEFINED 17    Conflict with define in microscape. 

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

#ifdef max
#undef max
#endif
#define max(x,y)        (((x)>(y))?(x):(y))

#ifdef min
#undef min
#endif
#define min(x,y)        (((x)<=(y))?(x):(y))

/* Header file stuff */

// Topo cleverly changed N_MAX_GRAPHS from 8 to 16 in rel 5.00;
// This changes the format of the entire data strcture.  We assume
// maximum size here, and compensate in the reading/writing routines
// if it's really only 8, as in versions <=4.0

#define N_MAX_GRAPHS            16
#define N_MAX_ECHEM_LEGS        100
#define N_MAX_EC_DATA           4000

typedef vrpn_uint16  *PDATA;
typedef vrpn_int32        TOPOBOOL;

// This is defined in the topo code in topo/wtvsuprt/wintv.h
struct topoRECT {
	vrpn_int32 left;
	vrpn_int32 top;
	vrpn_int32 right;
	vrpn_int32 bottom;
};

// This is defined as a DWORD in the topo code, in topo/wtvsuprt/wintv.h
// Already defined in a standard windows header
#ifdef _WIN32
typedef long unsigned int COLORREF;
#else
typedef vrpn_uint32 COLORREF;
#endif

// Should be 336 bytes total size, version 5.00
typedef struct  _tScanParamsLayered {
        vrpn_float64   fVzStart;       /*  voltage/distance start value*/
        vrpn_float64   fVzStop;        /*  voltage/distance stop value */
        vrpn_float64   fVzLimit;       /*  force limit */
        vrpn_float64   fVzArray;      /*  array of voltage/distance values
                                   (size determined by nLayers)  should be an Array pointer but can't read a pointer from file anyway*/
  /*        short   fVzArray;        should be a pointer value but can't read pointers from a file*/
        vrpn_float64   fVzSpeed1;      /*  speed of voltage/distance ramp
                                   (sample point) in V/sec or mircons/sec */
        vrpn_float64   fVzSpeed2;      /*  speed of voltage/distance ramp
                                   (pullback) in V/sec or mircons/sec */
        vrpn_float64   fVzSpeed3;      /*  speed of voltage/distance ramp
                                   (first sample point) in V/sec or micron/sec*/
        vrpn_float64   fVzSpeed4;      /*  speed of voltage/distance ramp
                                   (back into feedback) in V/sec or micron/sec*/
        vrpn_float64   fVzPullback;    /*  pullback distance (to get out of
                                   contact) */
        vrpn_int16   iLayers;        /*  number of layers */
        vrpn_int16   iHalfCycles;    /*  number of half cycles (forward = 1,
                                   forward+backward = 2) */
        vrpn_int16   iAvgPoint;      /*  averaging number per layer point */
        vrpn_float64   fDelayStart;    /*  time delay before first sample
                                   point in micro-sec */
        vrpn_float64   fDelaySample;   /*  time delay before each sample
                                   in micro-sec */
        vrpn_float64   fDelayPullback; /*  time delay after pullback in
                                   micro-sec */
        vrpn_float64   fDelayEstFeedbk;/*  time delay to re-establish feedback
                                   in micro-sec */
        TOPOBOOL    bFeedbkPoints;  /*  TRUE: enabled feedback between
                                   points */
        TOPOBOOL    bFeedbkCurves;  /*  TRUE: enabled feedback between
                                   curves */
        TOPOBOOL    bVzRelative;    /*  TRUE: voltage/distance values are
                                   relative */
        vrpn_float64   fModFreq;       /*  modulation frequency */
        vrpn_float64   fVzMod;         /*  voltage/distance modulation */
	vrpn_int16	iExtLayer;	/*  extracted layer */
	TOPOBOOL	bSpecialNCScan;  /*  special NC scanning */
        /*char    cFiller[180];     fill bytes 
		officially it should be size 180 but I am putting it to 184 so I can
		handle 3.01 files more easily */
        char    cFiller[184];   /*  fill bytes */
} SCANPARAMSLAYER, *PSCANPARAMSLAYER;

// Should be 1344 bytes total size, version 5.00
typedef struct  _tScanParams {
        vrpn_int16   iDataType;              /*  raw data source (Z, sensor, external #1,...) */
        vrpn_int16   iDataDir;               /*  direction data collected */
        vrpn_int16   iDataMode;              /*  collection mode: (2D, CITS, DITS, FIS, MFM, EFM, FS, IV,...) */
        vrpn_float64   fScanZmax;              /*  Z DAC max: 0 ... 0xFFFF */
        vrpn_float64   fScanZmin;              /*  Z DAC min: 0 ... 0xFFFF */
        vrpn_float64   fScanXmax;              /*  X piezo resolution in points */
        vrpn_float64   fScanYmax;              /*  Y piezo resolution */
        vrpn_float64   fVtip;                  /*  STM tip voltage in the range -10.0 ... +10.0 volts */
        vrpn_float64   fI;                     /*  desired sensor feedback/tunneling current in nA */
        vrpn_float64   fVz;                    /*  Z piezo setpoint in the range 0 ... 440 volts */
        vrpn_float64   fRange;                 /*  scanned image range */
        vrpn_float64   fRate;                  /*  XY scan rate in unitType units/sec */
        vrpn_int16   iGain;                  /*  ADC gain flag */
        vrpn_float64   fPro, fInteg, fDer;     /*  PID parameters */
        vrpn_int16   iGainZ;                 /*  Z piezo gain flag */
        vrpn_float64   fRotation;              /*  XY scan rotation, radians */
        vrpn_float64   fModLevel;              /*  modulation mode: modulation in  angstroms */
        vrpn_float64   fAveraging;             /*  modulation mode: number of points to avg per image point */
        vrpn_float64   fSpCalFactor;           /*  modulation mode: */
        vrpn_int16   iCalibType;             /*  XY calibration type */
        vrpn_int16   iLaserIntensity;        /*  laser setting for AFM */
        vrpn_uint16  iScaleFactorZ;  /*  z scale factor */
        vrpn_uint16  iDACminX, iDACmaxX; /*  piezo scan location x */
        vrpn_uint16  iDACminY, iDACmaxY; /*  piezo scan location y */
        char    cScanType[6];           /*  ASCII/binary separator with encoded scan type info (only used for pre Windows files) */
        vrpn_int16   iProbeType;             /*  probe type ( STM, AFM...) */
        vrpn_int16   iStageType;             /*  stage type ( STM, AFM, Aurora, Observer,...) */
        vrpn_int16   iCalFileSource;         /*  calibration file type used: ZYGO or MTI (from scanner SYSTEM file) */
        vrpn_float64   fOverscanX;             /*  overscan range in percent of scan range */
        vrpn_float64   fOverscanY;             /*  overscan range in percent of scan range */
	vrpn_int16           iSetpointUnits;     //    setpoint (fI) unit types
	vrpn_float64           fNcRegAmp;          //    register frequency of non contact
	vrpn_int16           iGainXY;            //    XY gain (0=low,1=high,2 = div 2, 3 = div 4...
	vrpn_uint16  iOffsetX, iOffsetY; //    x and y piezo offset DAC values
	vrpn_float64           fHysteresisX[4];    //    X piezo hysteresis polynomial
	vrpn_float64           fHysteresisY[4];    //    Y piezo hysteresis polynomial
	vrpn_uint16  iOffsetZ;           //    Z piezo offset DAC value
	vrpn_float64           fHysteresisZ[4];    //    Z piezo hysteresis polynomial
	vrpn_float64           fCrossTalkCoef;     //    xy crosstalk coefficient
	vrpn_float64           fSensorResponse;    //    sensor response in nA/nm
	vrpn_float64           fKc;                //    spring constant in N/m
	vrpn_int16           iCantileverType;    //    cantilever type
	char            szScannerSerialNumber[16]; //  scanner serial number
	vrpn_int16           iZlinearizer;       //    z linearizer type
	vrpn_int32      iADC;               //    adc used
	TOPOBOOL        bNonContact;           //    auto non_contact flag
	vrpn_int16           CantileverType;     //    0: Low Freq, 1: High Freq, 2:General Freq
	vrpn_float64           fDriveAmplitude;    //    Driving Amplitude
	vrpn_float64           fDriveFrequency;   //     Driving Frquency
	vrpn_int16           iNonContactMode;     //   0: Amplitude 1: phase
	vrpn_int16           iNonContactPhase;   //    0, 90, 180, 270
	char            cFiller [34];      //      fill bytes to make struct a fixed length
        SCANPARAMSLAYER scan3d;         //  scanning parameters
	char szStageType[64];    //    stage type from stages.ini
	char szStageName[64];    //    specific stage name from stages.ini
	char szStageText[64];     //  
	vrpn_uint16  iOldOffsetX, iOldOffsetY; //    x and y piezo offset DAC values
	vrpn_uint16  iOldDACminX, iOldDACmaxX; //      piezo scan location x
	vrpn_uint16  iOldDACminY, iOldDACmaxY; //      piezo scan location y
	vrpn_int16           iOldGainXY;            //    XY gain (0=low,1=high,2 = div 2, 3 = div 4...

	//char          cFiller1[370];   //      512 bytes filler add in ver3.07 layered imaging 
	 /*     370 here doesn't seem to match the inidcated range but I'll leave it at 370 */

  vrpn_int16     nThermalMode;       //  Thermal scan mode
  vrpn_float64   fThermalTSetpoint;  //  Temperature for Const T scan, C
  vrpn_float64   fThermalISetpoint;  //  Current for Const I scan, calib file
  vrpn_float64   fResistivityToVolt; //  Resistivity to Volt xlation, for IOT DAC
  vrpn_float64   fMilliwattToVolt;   //  For constant power mode
  vrpn_float64   fThermalTC;         //  time const. for IO-T, calib file
  vrpn_float64   fThermalModAmp;     //  Mod amp for 2-D, C
  vrpn_float64   fThermalModFreq;    //  mod Freq, kHz, 2-D
  vrpn_float64   fThermalMaxT;       //  max Temp, Calib file
  vrpn_float64   fThermalMaxI;       //  max Current, calib file
  vrpn_float64   fThermalCal[4];     //  calibration  coefficients, calib file
  char            cFiller1[256];     //   512 bytes filler add in ver3.07

} SCANPARAMS, *PSCANPARAMS;

// Version 5.0 has float64 instead of float32
// Should be 1544 bytes total size, version 5.00
typedef struct _tDocumentInfo { /* binary image information */
  vrpn_int32      iRelease;               /*  parsed release number */
  vrpn_int32      iOffset;                /*  byte offset to actual data */
  char            szRelease[N_RELEASE];   /*  release of software */
  char            szDatetime[N_DATETIME]; /*  date and time generated */
  char            szDescription[N_DESCRIPTION];   /*  description string*/
  vrpn_float64    fPosX[N_MAX_GRAPHS];    /*  position of curve in x */
  vrpn_float64    fPosY[N_MAX_GRAPHS];    /*  position of curve in y */
  vrpn_int16      iCurves;                /*  number of curves  */
  vrpn_int32      iRows, iCols;           /*  number of data points */
  vrpn_uint16     iDACmax, iDACmin;/*  maxand min value in data*/
  vrpn_float64    fXmin, fXmax;           /*  scan distance in x */
  vrpn_float64    fYmin, fYmax;           /*  scan distance in y */
  vrpn_float64    fDACtoWorld;            /*  conversion factor from DAC units to physical units */
  vrpn_float64    fDACtoWorldZero;        /*  zero set point for physical units */
  vrpn_uint16     iDACtoColor;     /*  conversion factor from DAC units to color indices */
  vrpn_uint16     iDACtoColorZero; /*  zero set point for color indices */
  vrpn_int16      iWorldUnitType;         /*  physical unit type */
  vrpn_int16      iXYUnitType;            /*  physical unit type in x and y for display */
  char    szWorldUnit[10];        /*  string of physical units in z */
  char    szXYUnit[10];           /*  string of physical units in x and y (see iXYUnitType) */
  char    szRateUnit[10];         /*  string of scan rate unit*/
  vrpn_int16      iLayers;                /*  number of image layers */
  TOPOBOOL        bHasEchem;              /*  has Echem data */
  TOPOBOOL        bHasBkStrip;            /*  has bkStrip data */
  vrpn_int16      iPts[N_MAX_GRAPHS];     /*  datapoints per spectroscopy curve */
  vrpn_int16      iXUnitType;             /*  physical unit type in x for spectroscopy display */
  char    szXUnit[10];            /*  string of physical units in x (see iXUnitType) */
  TOPOBOOL        bHasAcqDisplay;                 //    has acquisition display parameters
  vrpn_int16      iTilt;                          //    acquisition tilt removal type
  vrpn_int16      iScaleZ;                        //    acquisition data range calculation type
  vrpn_int16      iFilter;                        //    acquisition filter type
  vrpn_int16      iShading;                       //    acquisition shading type
  double          dTiltC[8];                      //    acquisition tilt removal coefficients
  vrpn_uint16     iDACDisplayZero;                //    z adjust zero point in DAC units
  vrpn_uint16     iDACDisplayRange;               //    z adjust range in DAC units
  topoRECT        rRoi;                           //    active image selection

  // copied in from topo/document.h 5 August 99

  vrpn_int16      nNumChannels[N_MAX_GRAPHS];     //  
  vrpn_int16      nNumX[N_MAX_GRAPHS];            //  
  COLORREF        Colors[N_MAX_GRAPHS][10];        //  
  char            szXUnit1[10];                   //  
  char            szXUnit2[10];                   //  
  vrpn_int16      iXUnitType1;                    //  
  vrpn_int16      iXUnitType2;                    //  
  vrpn_int32      iDibWidth;
  vrpn_int32      iDibHeight;
  vrpn_int16      iTiltV;
  vrpn_int16      iRot;
  vrpn_uint16     iZmax, iZmin;
  vrpn_int16      iImgX, iImgY, iScrX, iScrY;
  vrpn_float64    fZadjust, fRatio, fSize, fView, fZoom;
  vrpn_float64    f11, f12, f13, f21, f22, f23, f32, f33;
  vrpn_float64    phiMax, phiMin;
  vrpn_float64    mapMax, mapMin;
  vrpn_int16      itD2Factor;
  vrpn_int16      view;
  vrpn_int16      ViewType;
  vrpn_int16      shade;
  vrpn_int16      map;

  char            cFiller [44];

  // cFiller has shrunk from 510 bytes to 44 between v3.06 and 5.0

} DOCUMENTINFO, *PDOCUMENTINFO;

typedef struct _tTopoDocument {
        DOCUMENTINFO    sInfo;          /* [1..1544] document information */
        SCANPARAMS      sScanParam;     /* [1545..2888] scan parameters */
} TOPODOC, *PTOPODOC;

// Apparently there are two entry points into this class
// used by microscape (as of 5 Aug 99):
//   BCGrid::readTopometrixFile (TopoFile &, const char *);
//   BCGrid::readTopometrixFile (FILE *, const char *);
// All calls to the latter have been commented out, but the code is
// still here.  Of course, it's that code that we found and fixed first.

class TopoFile{
private:
    // We want to extract _only_ the info we are interested in.
    // Otherwise we read and write the header in a block
    // Here's the data members we are interested in:
    // All are part of the DOCUMENTINFO sInfo structure, and 
    // they appear in this order. 
    vrpn_int32      iRelease, iOffset;
	/* Parsed release number and offset from begin of file to data */
    char            szRelease[N_RELEASE];   
    	/*  release of software */
    char            szDatetime[N_DATETIME]; 
    	/*  date and time generated */
    char            szDescription[N_DESCRIPTION];   
    	/*  description string*/
    vrpn_int32      iRows, iCols;           
    	/* number of data points */
    vrpn_float64    fXmin, fXmax;           /*  scan distance in x */
    vrpn_float64    fYmin, fYmax;           /*  scan distance in y */
    vrpn_float64    fDACtoWorld;            
    	/*  conversion factor from DAC units to physical units */
    vrpn_float64    fDACtoWorldZero;        
    	/*  zero set point for physical units */
    vrpn_int32	    iWorldUnitType;
    vrpn_int32	    iXYUnitType;
    char    szWorldUnit[10];        
    	/*  string of physical units in z */
    char    szXYUnit[10];
	/*  string of physical units in x and y (see iXYUnitType) */
    vrpn_int32	    iLayers;
	/* number of layers of data */
    topoRECT        rRoi;
    	// Region of the image which contains valid data.

    // maybe scanparams.iDataDir for orientation of data. 
    // from the SCANPARAMS sScanParam structure. 

    vrpn_bool valid_grid;
    char* header;
    vrpn_uint16 *griddata;


    int parseReleaseString(const char *release, vrpn_int32 * verNumber, vrpn_int32* nOffset);
    int getGridInfoFromHeader (const char * temp);
    int printGridInfoFromHeader(void);
    int putGridInfoIntoHeader (char * temp);
    int parseData(int);
    int writeHeader(int);
    int writeData(int);
    int writeTopoFile(int);
    
    vrpn_bool valid_header;
    void fixUpDocStruct(void);		//version 4.0 fix
    void convertWin31ToNT (const char *);  // version 5.0 fix
public:
    TopoFile(){
		  header=NULL;
		  valid_header=0;
		  valid_grid=0;
		  griddata = NULL;
    } 
    ~TopoFile(){
		    if(header!=NULL) delete [] header;
		    if(griddata!=NULL) delete [] griddata;	
    }
    int initForConversionToTopo(double scale_nm,double offset_nm);
    int printDocInfo();
    int printScanParams();
    int printScanLayers();
    int printData();
    int readTopoFile(const char *);
    int writeTopoFile(const char * filename);
    int writeTopoFile(FILE* handle);
    int parseHeader(const char *header,vrpn_int32 length);	      
    int parseHeader(int);
    int topoDataToGrid(BCGrid*,const char*);
    int gridToTopoData(BCGrid*);
    int gridToTopoData(BCGrid*,BCPlane*);
    int imageToTopoData(nmb_Image *I);
    void freeGridData(){ 
	if(griddata!=NULL) delete [] griddata; 
	griddata = NULL; valid_grid=0;}
    vrpn_int32 Rows(){ return iRows;}
    vrpn_int32 Cols(){ return iCols;}
};
#define VERNUMBER 500
//#define NOFFSET   3144
#define NOFFSET     256 + sizeof(TOPODOC)
#endif
