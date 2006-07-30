/***********************************************************************
*
*	File Name: Header.h
*
*	Description: Header file for Header.c
*
***********************************************************************/
#ifndef _HEADER_H_
#define _HEADER_H_

#include <stdio.h>

#define MAX_CHARS_IN_HEADER_LINE	1000

#define WSXM_BINARY_HEADER_ID  "STM_IMG" // ID for the old files with binary header

#define IMAGE_HEADER_VERSION        "1.0 (April 2000)"

#define IMAGE_HEADER_SIZE_TEXT      "Image header size: "
#define	IMAGE_HEADER_END_TEXT		"Header end"
#define TEXT_COPYRIGHT_NANOTEC      "WSxM file copyright Nanotec Electronica\n"
#define STM_IMAGE_FILE_ID           "SxM Image file\n"

#define IMAGE_HEADER_GENERAL_INFO                 "General Info"
#define IMAGE_HEADER_GENERAL_INFO_NUM_COLUMNS	  "Number of columns"
#define IMAGE_HEADER_GENERAL_INFO_NUM_ROWS		  "Number of rows"
#define IMAGE_HEADER_GENERAL_INFO_Z_AMPLITUDE	  "Z Amplitude"
#define IMAGE_HEADER_GENERAL_INFO_IMAGE_DATA_TYPE "Image Data Type"

#define IMAGE_DATA_TYPE_SHORT                     "short"
#define IMAGE_DATA_TYPE_DOUBLE                    "double"

#define IMAGE_HEADER_MISC_INFO			     "Miscellaneous"
#define IMAGE_HEADER_MISC_INFO_MAXIMUM	     "Maximum"
#define IMAGE_HEADER_MISC_INFO_MINIMUM	     "Minimum"
#define IMAGE_HEADER_MISC_INFO_COMMENTS	     "Comments"
#define IMAGE_HEADER_MISC_INFO_VERSION	     "Version"

#define IMAGE_HEADER_CONTROL                "Control"
#define IMAGE_HEADER_CONTROL_X_AMPLITUDE	"X Amplitude"
#define IMAGE_HEADER_CONTROL_Y_AMPLITUDE	"Y Amplitude"

#define IMAGE_HEADER_HEADS	"Head Settings"

#define IMAGE_HEADER_HEADS_X_CALIBRATION	"X Calibration"
#define IMAGE_HEADER_HEADS_Z_CALIBRATION	"Z Calibration"




/***********************************************************************
*
*	HEADER structure
*
*	This is the structure we will use to represent a header of a WSxM
*	It will have three strings representing each value in the header
*
*	- The title will indicate the group of values this value is included
*	in the header
*
*	- The label will precisate what is the value for
*
*	- The value will be an ASCII representation of the value
*
*	In the structure we can find too the total number of fields in the
*	structure
*
***********************************************************************/

/*
typedef struct typeHeader
{
	char **tszTitles;
	char **tszLabels;
	char **tszValues;

	int iNumFields;

	int bBinary;      // header can be binary or ASCII
} HEADER;
*/
// Replace with my own class
#include "readNanotecFile.h"

/* Initialization of the header */

void HeaderInit (HEADER *pHeader);

/* Header file input/output */

/* Header read */

int HeaderRead (HEADER *pHeader, FILE *pFile);
int HeaderASCIIRead (HEADER *pHeader, FILE *pFile);
int HeaderBinaryRead (HEADER *pHeader, FILE *pFile);

/* Header write */

int HeaderWrite (HEADER *pHeader, FILE *pFile);

/* Header access to one field */

/* Read */

double HeaderGetAsNumber	(HEADER *pHeader, char *szTitle, char *szLabel);
int HeaderGetAsString		(HEADER *pHeader, char *szTitle, char *szLabel, char *szValue);
void HeaderReadTitle (char *szLine, char *szTitle);
void HeaderReadLabel (char *szLine, char *szLabel);
void HeaderReadValue (char *szLine, char *szValue);

/* Write */

void HeaderSetAsFloating	(HEADER *pHeader, char *szTitle, char *szLabel, double lfValue);
void HeaderSetAsInt			(HEADER *pHeader, char *szTitle, char *szLabel, int iValue);
void HeaderSetAsString		(HEADER *pHeader, char *szTitle, char *szLabel, char *szValue);

/* Header destroy */

void HeaderDestroy (HEADER *pHeader);


/* Internally used functions */

int HeaderReadLine (HEADER *pHeader,FILE *pFile, char * szTitle);
int HeaderGetSize (HEADER *pHeader);
int	HeaderAddValue (HEADER *pHeader, char *szTitle, char *szLabel,char *szValue);
 
void RemoveLeftAndRightWhitesFromString (char *szString);
void ReplaceStringInString (char *szDest, const char *szOld, const char *szNew);

#endif //_HEADER_H_

