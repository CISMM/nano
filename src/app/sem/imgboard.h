/* IMGBOARD.H   */
#ifndef IMGBOARD_H
#define IMGBOARD_H
#define EDXCALL _stdcall
/***************** FUNCTION DECLARATIONS ************************/
#ifdef _cplusplus
extern "C" {
#endif
	int  EDXCALL  GetBoardId();
	int  EDXCALL  InitBoard(HANDLE, int, unsigned int *);
	int  EDXCALL  InitGpuBoard(ULONG);
	int  EDXCALL  SetStatusWindow(HANDLE);
	int  EDXCALL  PcMtim(unsigned int, float);
	int  EDXCALL  SgHdwr(int *);
	int  EDXCALL  SgRset();
//	int  EDXCALL  ResizeDib(unsigned int);
	int  EDXCALL  GrXlin(HANDLE, unsigned int, unsigned int, unsigned int, unsigned int *);
	int  EDXCALL  SetupCollect(HANDLE, HANDLE, int);
	int  EDXCALL  SetupLineCollect(HANDLE);
	int  EDXCALL  SetupQline(HANDLE);
	int  EDXCALL  SetupQmaps(HANDLE);
	int  EDXCALL  SetupPlayback(HANDLE, HANDLE);
	int  EDXCALL  PcSets(HANDLE, HANDLE, HANDLE);
	int  EDXCALL  SetupScanSegment(HANDLE, int, int);
	int  EDXCALL  ScanSegment(int, int, int);
//	int  EDXCALL  PcScan(int);
	int  EDXCALL  SpVidm(int);
	int  EDXCALL  AutoScale(HANDLE, HANDLE);
	int  EDXCALL  SetupXscale(HANDLE, HANDLE);
	int  EDXCALL  AutoXscale(HANDLE, int);
	int  EDXCALL  CollStrip(HANDLE, HANDLE, unsigned int);
	int  EDXCALL  CollImgLine(HANDLE, HANDLE, unsigned int);
//	int  EDXCALL  PlayStrip(unsigned int, unsigned int, unsigned int);
	int  EDXCALL  PlayImgLine(HANDLE, HANDLE, unsigned int);
	int  EDXCALL  CollPoints(HANDLE, HANDLE, unsigned int);
	int  EDXCALL  CollPoint(int, int);
	int  EDXCALL  CollectLinePoint(int, float *);
	int  EDXCALL  CollectQlinePoint(int);
	int  EDXCALL  CollectXLine(int);
	int  EDXCALL  CollectXPoint(int, int);
	int  EDXCALL  CollectQPoint(int, int);
//	int  EDXCALL  CollPointsStrip(unsigned int, unsigned int, unsigned int);
	int  EDXCALL  CollFast1(HANDLE, int);
	int  EDXCALL  ResetCollect(HANDLE);
	int  EDXCALL  ResetPlayback(HANDLE);
	int  EDXCALL  CollectFirst(int);
//	int  EDXCALL  GetBuffAddr (int);
//	int  EDXCALL  GetRamData (unsigned int, unsigned int *, int);
//	int  EDXCALL  LoadRamData (unsigned int, unsigned int *, int);
	int  EDXCALL  SetupSgColl(int, int, int, int);
	LONG  EDXCALL  CollectSgLine(unsigned char *);
	int  EDXCALL  CollectSgStrip(unsigned char *);
	int  EDXCALL  ResetSgColl();
	int  EDXCALL  ResetGpuBoard();
	int  EDXCALL  SetSgScan(int);
	int  EDXCALL  SetScan(int, int);
	int  EDXCALL  SetupSgScan(int,int,int,int,int,int,int);
	int  EDXCALL  SyncEDS1(int);
	int  EDXCALL  SetupGetLine(HANDLE);
	int  EDXCALL  GetLine(unsigned int, unsigned int *);
	BOOL EDXCALL  ScanInit(HANDLE hGpuScan);

	// Set/Get the gain and offset for the x and y dacs and the z adc

	int	EDXCALL	  SetupDACParams(SHORT GainParams[3], SHORT OffsetParams[3]);
	int EDXCALL	  ReadDACParams(SHORT GainParams[3], SHORT OffsetParams[3]);

	// Set the line frequency synchronization mode (0 = off, 1 = 50 hz, 2 = 60 hz)

	int EDXCALL   SgLineSync(LONG SyncMode);

	// Set the beam blanking mode (0 = Unblanked, 1 = Blanked)

	int EDXCALL   SgBeamBlank(LONG BlankMode);

	// Set/Get the horizontal and vertical retrace delays

	int EDXCALL   SgSetRetrace(LONG Horz, LONG Vert);
	int EDXCALL   SgGetRetrace(PLONG Horz, PLONG Vert);

	// Set/Get the scan direction for the x and y axis

	int EDXCALL	  SgSetScanDir(LONG xMirror, LONG yMirror);
	int EDXCALL	  SgGetScanDir(PLONG xMirror, PLONG yMirror);

	// Set/Get the video polarity of the 16 physical channels

	int EDXCALL   SgSetVideoPolarity(LONG VideoPol[16]); 
	int EDXCALL   SgGetVideoPolarity(LONG VideoPol[16]);
	
	// Set/Get the max horizontal and vertical dac span

	int EDXCALL   SgSetMaxSpan(LONG XSpan, LONG YSpan);
	int EDXCALL   SgGetMaxSpan(PLONG XSpan, PLONG YSpan);

	// Set/Get the mapping between the x/y axis and the physical
	// scan dac's.

	int EDXCALL   SgSetXScanDAC(LONG xDAC);
	int EDXCALL   SgGetXScanDAC(PLONG xDAC);

	// Set/Get the 
	//
	//			scan type: normal/fast (0,1)
	//			DataTransfer: off/on (0,1)
	//			PixelDataSize: byte/word (0,1)

	int EDXCALL	  SgSetScanParams(LONG ScanType, LONG DataTransfer);
	int EDXCALL	  SgGetScanParams(PLONG ScanType, PLONG DataTransfer);

	// Enable/Disable the external interface
	//
	// mode (0 = off, 1 = on)
	// photo (ignored)
	//
	int EDXCALL SpMode(int mode, int iPhoto);

	// Enable/Disable the EMIA for vb.

	int EDXCALL SgEmia(int rwmode, int *onoff);  // VB Only

	// v2.2

	int  EDXCALL  SpMove(int, int, int);
	int  EDXCALL  ScanSegments(unsigned int, int *, int *, int *);
	int  EDXCALL  ScanTable(unsigned int, int *, int *);

	BOOL EDXCALL  IsWin95();
	int  EDXCALL  SpDwel(int, int, int);
#ifdef _cplusplus
	};
#endif

/****************************************************************/

#endif
