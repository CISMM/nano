#ifndef STGCTRL_H
#define STGCTRL_H
#include "srchfld.h"
#include <windows.h>
#include <winbase.h>

#define EDXCALL _stdcall
/********************************************************/
extern "C" {   /* Function prototypes for stage control */
	int 	EDXCALL 	InitStage (STGDEF &);
	int 	EDXCALL 	HomeStage (int);
	int 	EDXCALL 	ResetStage (int);
	int		EDXCALL 	ReadStageLoca (int, STGLOCA &);
	int 	EDXCALL 	ReadStageError (int, STGLOCA &);
	int		EDXCALL 	MoveStageLoca (int, STGLOCA &);
	int 	EDXCALL 	GetLimits(int, STGLOCA &, STGLOCA &);
	int 	EDXCALL 	SetupStage (int, int);
	int 	EDXCALL 	StepStage (int, STGLOCA &);
	int 	EDXCALL  GetStageReply (char*);
	int    	EDXCALL  DisplayStageError(int, int);
	int		EDXCALL	StopStage(int);
	int		EDXCALL	EnableJoystick(int, int);
//=========================== VB interface ==================
	int 	EDXCALL 	OpenStage();
	int		EDXCALL 	CloseStage();
	int		EDXCALL 	GetStgDef(STGDEF &);
	int		EDXCALL 	SetStgDef(STGDEF &);
	int		EDXCALL  SetStageHwnd(unsigned int);
	int		EDXCALL	NotifyStage(unsigned int);
	int		EDXCALL	GetTiltStatus(int &);
	int		EDXCALL	GetStageTilt(float &);
	int		EDXCALL	SetStageTilt(float);
	int		EDXCALL	SetStageJog(int, int, long, long, long,
									long, long);
	int 	EDXCALL 	SetStageCmd(char far *, int);
	int 	EDXCALL 	GetStageCmd(char far *);
	int 	EDXCALL 	WhatStage();
	int 	EDXCALL 	Backlash(int);
	int 	EDXCALL 	GetCommProp(COMMPROP &commpr);
	}
//**********************************************************************
#endif
