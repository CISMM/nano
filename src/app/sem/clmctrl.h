/* EMCTRL.H */
#ifndef EMCTRL_H
#define EMCTRL_H

#include "srchfld.h"
#include "semmsgid.h"

#define EDXCALL _stdcall

#ifndef CLMVECT_H
#define CLMVECT_H
extern "C" {
	typedef struct tagCLMVECT {
		float	kv;
		long	mag;
		float	spot;
		float	wd;   
		float	contr;    
		float	bright;
		int		scanType;	//--1=extXY,0=semXY
		int		scanLines;
		int		scanTime;
		int		beamOnOf;
		int		kvOnOf;
		int		detIndex;
		float	f1,f2;		//--reserved
		int		tvOn;		//--1=ON,0=OFF
		int		ymodOn;		//--1=ON,0=OFF
		} CLMVECT;			//--64 bytes
	typedef struct tagSEMPOLLING {
		int		stgPoll;
		unsigned long	stgMoveTimeout;
		int		clmPoll;
		unsigned long	clmPollInterval;
		unsigned long	clmPollSingleInterval;
		int		clmPollKv;
		int		clmPollMag;
		int		clmPollSpot;
		int		clmPollWd;
		int		clmPollBright;
		int		clmPollContr;
		int		clmPollScanType;
		int		clmPollKvOnOf;
		int		clmPollBeamOnOf;
		int		clmPollScanRate;
		int		clmPollYmodOn;
		int		res[4];	//--- reserverd -----
		} SEMPOLLING;		
	};
#endif

enum SEM_CMD {
	SEM_IDLE,CLM_KV_SET,CLM_KV_GET,CLM_MAG_SET,CLM_MAG_GET,CLM_SPOT_SET,CLM_SPOT_GET,
	CLM_WD_SET,CLM_WD_GET,CLM_ACB,CLM_AFC,CLM_AFF,CLM_AFS,
	CLM_KVONOF_SET,CLM_KVONOF_GET,CLM_BEAMONOF_SET,CLM_BEAMONOF_GET,CLM_BRIGHT_SET,
	CLM_BRIGHT_GET,CLM_CONTR_SET,CLM_CONTR_GET,CLM_DET_SET,CLM_DET_GET,
	CLM_SCANTYPE_SET,CLM_SCANTYPE_GET,CLM_SCANRATE_SET,CLM_SCANRATE_GET,CLM_YMOD_SET,
	CLM_YMOD_GET,STG_POS_SET,STG_POS_GET,SEM_SUSPEND
	};

extern "C" {
	int  EDXCALL InitControl(CLMDEF &);
	int  EDXCALL ReadLoca(int, CLMLOCA &);
	int  EDXCALL WriteLoca(int, CLMLOCA &);
	int  EDXCALL GetScopeId();
	int  EDXCALL GetClmVersion(int &, int &);
	int  EDXCALL ReadAcc(int, unsigned long&);
	int  EDXCALL ReadMag(int, unsigned long&);
//***************** new interface ********* 13-SEP-94 *************
//======== common to all columns ==================================
	int  EDXCALL OpenColumn();
	int  EDXCALL CloseColumn();
	int  EDXCALL GetClmDef(CLMDEF &);
	int  EDXCALL SetClmDef(CLMDEF &);
	int  EDXCALL GetColumnReply(char  *);
	void EDXCALL SetColumnHwnd(unsigned int);
	void EDXCALL SetColumnDiag(int);
	int  EDXCALL DisplayColumnError(int, int);
//======== column dependent =======================================
	int  EDXCALL SetAccOnOff(int);          
	int  EDXCALL GetAccOnOff(int &);       
	int  EDXCALL GetGunStatus(float &,float &);	//--- 22-JAN-98
	int  EDXCALL SetAccKv(float);       
	int  EDXCALL GetAccKv(float &);     
	int  EDXCALL SetMag(long);           
	int  EDXCALL GetMag(long &);         
	int  EDXCALL SetSpotSize(float);     
	int  EDXCALL GetSpotSize(float &);   
	int  EDXCALL SetWorkDist(float);    
	int  EDXCALL GetWorkDist(float &);  
	int  EDXCALL SetCon(float);         
	int  EDXCALL GetCon(float &);       
	int  EDXCALL SetBright(float);       
	int  EDXCALL GetBright(float &);    
	int  EDXCALL SetScanRota(float);     
	int  EDXCALL GetScanRota(float &);   
	int  EDXCALL SetScanShift(float, float);     
	int  EDXCALL GetScanShift(float &, float &);  
	int  EDXCALL AutoBC();               
	int  EDXCALL AutoFocusC();           
	int  EDXCALL AutoFocusF();          
	int  EDXCALL AutoFocusStig();       
	int  EDXCALL SetPhoto();            
	int  EDXCALL SetDet(int, int);          
	int  EDXCALL GetDet(int &, int &);        
	int  EDXCALL SetVacHighLow(int);     
	int  EDXCALL SetVacLevel(int);      
	int  EDXCALL SetScanType(int);      
	int  EDXCALL GetScanType(int &);        
	int  EDXCALL GetSpecimenCrt(float &);   
	int  EDXCALL GetBBlankOnOff(int &);     
	int  EDXCALL SetBBlankOnOff(int);       
	int  EDXCALL GetVideoDisplay(int &, int &);     
	int  EDXCALL SetVideoDisplay(int, int);     
	int  EDXCALL SetScanRate(int, int);      
	int  EDXCALL GetScanRate(int &, int&);      
	int  EDXCALL SetBeamLoca(float, float); 
	int  EDXCALL SetColumnCmd(char  *, int);
	int  EDXCALL GetColumnCmd(char  *);
	int  EDXCALL GetScanMetrics(float &, float &);
	int  EDXCALL CheckAccKv(float &);    // 9-mar-95
	int  EDXCALL CheckMag(long &);       // 9-mar-95
	int  EDXCALL CheckSpotSize(float &); // 9-mar-95
	int  EDXCALL CheckWorkDist(float &); // 9-mar-95
	int  EDXCALL GetClmLimits(CLMVECT &, CLMVECT &); // 9-mar-95
	int  EDXCALL GetStageTilt(float &);  // 14-dec-95
	int  EDXCALL SetStageTilt(float );   // 14-dec-95
	int  EDXCALL GetVideoBand(float &);  // 22-jan-96
	int  EDXCALL SetVideoBand(float);    // 22-jan-96
	int  EDXCALL GetClmParam(char  * , float &);
	int  EDXCALL GetFrameImage(unsigned int,int,int);
	int  EDXCALL ConnectToSem(unsigned int,SEM_MESSAGE &);
	};
/****************************************************************/

#endif
