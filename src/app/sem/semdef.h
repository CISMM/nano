// semdef.h

#ifndef _SEMDEF_H
#define _SEMDEF_H
#include <windows.h>

#define SEM_CURR_FILE_VERS 0.53

typedef struct {
	char	cfgName[12];			// EDISEM32
	float	fVersion;				// 0.53
	float	aVersion;				// Application Version
	LONG	deviceNumber;			// EDI Device Number
	LONG	horzRetrace;			// Horizontal retrace delay in uS
	LONG	vertRetrace;			// Vertical retrace delay in uS
	LONG	maxXSpan;				// Max X Span = 65535
	LONG	maxYSpan;				// Max Y Span = 51200
	LONG	xScanDACSelect;			// Horizontal DAC = 0, Vertical DAC = 1
	LONG	xOffset;				// X axis offset
	LONG	xGain;					// X axis gain
	LONG	yOffset;				// Y axis offset
	LONG	yGain;					// Y axis gain
	LONG	zOffset;				// Z axis offset
	LONG	zGain;					// Z axis gain
	LONG	xMirror;				// X axis scan direction
	LONG	yMirror;				// Y axis scan direction
	LONG	lineSync;				// Line freq sync, 0=Off, 1=50Hz, 2=60Hz
	LONG	videoPolarity[16];		// Video polarity for each physical channel
	// V2.2
	LONG    hasEmia;				// Control display of EMIA button:  0 = No Emia, 1 = Emia
	LONG	filler[255];			// Filler for additional params
} EDI32_SEM_REC, *PEDI32_SEM_REC;

#endif