/***********************************************************************
*
*	File Name: wsxmHeader.c
*
*	Description: functions for managing the HEADER structure defined in Header.h
*
***********************************************************************/

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "wsxmHeader.h"

/***********************************************************************
*
*	Function void HeaderInit (HEADER *pHeader);
*
*	Description: Initializes the values of the HEADER structure. It should
*				be called always before using any of the other functions
*
***********************************************************************/

void HeaderInit (HEADER *pHeader)
{
	if (pHeader == NULL)
		return;

	pHeader->tszTitles = NULL;
	pHeader->tszLabels = NULL;
	pHeader->tszValues = NULL;

	pHeader->iNumFields = 0;
}

/***********************************************************************
*
*	Function int HeaderRead (HEADER *pHeader, FILE *pFile);
*
*	Description: Reads a HEADER from a file. It is important that the file
*				should have been opened for reading, and the HEADER should
*				have been initializated calling HeaderInit
*
*	Inputs:
*			- pHeader: pointer to the HEADER object to store the data in
*			- pFile: File pointer to read the header from
*
*	Outputs:
*			- pHeader: will be filled with the data read from pFile
*			- pFile: the file pointer will go to the end of the header
*
*	Return value:
*			0 if the header was correctly read, -1 elsewhere
*
***********************************************************************/

int HeaderRead (HEADER *pHeader, FILE *pFile)
{
	char szID [8];

	/* First of all determines if the header is binary looking for the id WSXM_BINARY_HEADER_ID */
	fread (szID, 8, 1, pFile);

	if (strcmp (szID, WSXM_BINARY_HEADER_ID) == 0)
	{
		/* Is a binary header */
		pHeader->bBinary = 1;
		return HeaderBinaryRead (pHeader, pFile);
	}
	else
	{
		/* Is an ASCII header */
		pHeader->bBinary = 0;
		return HeaderASCIIRead (pHeader, pFile);
	}
}

/***********************************************************************
*
*	Function int HeaderASCIIRead (HEADER *pHeader, FILE *pFile);
*
*	Description: Reads a HEADER from a file knowing that is an ASCII header.
*
*	Inputs:
*			- pHeader: pointer to the HEADER object to store the data in
*			- pFile: File pointer to read the header from
*
*	Outputs:
*			- pHeader: will be filled with the data read from pFile
*			- pFile: the file pointer will go to the end of the header
*
*	Return value:
*			0 if the header was correctly read, -1 elsewhere
*
***********************************************************************/

int HeaderASCIIRead (HEADER *pHeader, FILE *pFile)
{
	int iStatus = 0;

	if ((pHeader == NULL) || (pFile == NULL))
		return -1;

	if (pHeader->iNumFields != 0)
		return -1;

	/* Read the file line by line */

	char szTitle[MAX_CHARS_IN_HEADER_LINE*2] = "";

	while (iStatus == 0)
	{
		iStatus = HeaderReadLine (pHeader,pFile, szTitle);
	}

	if (iStatus == -1) /* Error */
	{
		return -1;
	}

	return 0;
}

/***********************************************************************
*
*	Function int HeaderBinaryRead (HEADER *pHeader, FILE *pFile);
*
*	Description: Reads a HEADER from a file knowing that is a binary header.
*
*	Inputs:
*			- pHeader: pointer to the HEADER object to store the data in
*			- pFile: File pointer to read the header from
*
*	Outputs:
*			- pHeader: will be filled with the data read from pFile
*			- pFile: the file pointer will go to the end of the header
*
*	Return value:
*			0 if the header was correctly read, -1 elsewhere
*
***********************************************************************/

int HeaderBinaryRead (HEADER *pHeader, FILE *pFile)
{
	/* Binary header fields */

	short sHeaderLenght;
	short sNumRows;
	short sNumCols;
	short sImageType;
	short sXOffset;
	short sYOffset;
	short sMedia;
	short sHeadType;       // (i.e. AFM, STM...)
	float fPixAmp;         // Not used
	float fYSpacing;       // Not used
	float fXAmp;
	float fYAmp;
	float fZGain;
	float fXNor;           // Not used
	float fYNor;           // Not used
	float fForceConstant;  // Only for AFM
	float fZCut;           // Not used
	char szFill[90];       // Filling
	char szVersion[10];    // Version
	float fZAmp;
	float fBias;
	float fFreq;
	char szComments[80];
	float fXYCal;
	float fZCal;
	short sFirstRow;       // Not used
	short sFirstCol;       // Not used
	float fIV;             // mV / nA
	float fSetPoint;
	char szFill2[236];     // Filling
	short sMaximum;
	short sMinimum;

	int iStatus = 0;

	char szBuffer[100]; // To add values to the HEADER

	if ((pHeader == NULL) || (pFile == NULL))
		return -1;

	if (pHeader->iNumFields != 0)
		return -1;


	/* Reads each field of the header */

	if (fread (&sHeaderLenght, sizeof (short), 1, pFile) != 1) return -1;
	if (sHeaderLenght != 512)
	{
		/* Wrong header size */
		return -1;
	}

	if (fread (&sNumRows, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&sNumCols, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&sImageType, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&sXOffset, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&sYOffset, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&sMaximum, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&sMinimum, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&sMedia, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&sHeadType, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&fPixAmp, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fYSpacing, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fZGain, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fXNor, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fYNor, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fForceConstant, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fZCut, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (szFill, 90, 1, pFile) != 1) return -1;
	if (fread (szVersion, 10, 1, pFile) != 1) return -1;
	if (fread (&fZAmp, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fXAmp, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fYAmp, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fBias, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fFreq, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (szComments, 80, 1, pFile) != 1) return -1;
	if (fread (&fXYCal, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fZCal, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&sFirstRow, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&sFirstCol, sizeof (short), 1, pFile) != 1) return -1;
	if (fread (&fIV, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (&fSetPoint, sizeof (float), 1, pFile) != 1) return -1;
	if (fread (szFill2, 236, 1, pFile) != 1) return -1;


	/* Adds the important values to the HEADER */

	HeaderSetAsInt (pHeader, IMAGE_HEADER_GENERAL_INFO, IMAGE_HEADER_GENERAL_INFO_NUM_COLUMNS, sNumCols);
	HeaderSetAsInt (pHeader, IMAGE_HEADER_GENERAL_INFO, IMAGE_HEADER_GENERAL_INFO_NUM_ROWS, sNumRows);

	sprintf (szBuffer, "%f �", fXAmp);
	HeaderSetAsString (pHeader, IMAGE_HEADER_CONTROL, IMAGE_HEADER_CONTROL_X_AMPLITUDE, szBuffer);
	sprintf (szBuffer, "%f �", fYAmp);
	HeaderSetAsString (pHeader, IMAGE_HEADER_CONTROL, IMAGE_HEADER_CONTROL_Y_AMPLITUDE, szBuffer);
	sprintf (szBuffer, "%f �", fZAmp);
	HeaderSetAsString (pHeader, IMAGE_HEADER_GENERAL_INFO, IMAGE_HEADER_GENERAL_INFO_Z_AMPLITUDE, szBuffer);

	sprintf (szBuffer, "%f �/V", fXYCal);
	HeaderSetAsString (pHeader, IMAGE_HEADER_HEADS, IMAGE_HEADER_HEADS_X_CALIBRATION, szBuffer);
	sprintf (szBuffer, "%f �/V", fZCal);
	HeaderSetAsString (pHeader, IMAGE_HEADER_HEADS, IMAGE_HEADER_HEADS_Z_CALIBRATION, szBuffer);

	return 0;
}

/* Header write */

/***********************************************************************
*
*	Function int HeaderRead (HEADER *pHeader, FILE *pFile);
*
*	Description: Writes the HEADER object to a file. It is important that the file
*				should have been opened for writing (not append).
*
*	Inputs:
*			- pHeader: pointer to the HEADER object with the data
*			- pFile: File pointer to write the data header
*
*
*	Return value:
*			0 if the header was correctly write, -1 elsewhere
*
***********************************************************************/

int HeaderWrite (HEADER *pHeader, FILE *pFile)
{
	int i, j;
	char *pCharAux;                                         /* for swapping in bubble sort */	                                                        
	char szAuxTitle[1000], szAuxLabel[1000], szAuxValue[1000]; /* for writting in file        */

	/* sorts the HEADER object by title and label using bubble sort algorithm */
                   
	for (i=0; i < pHeader->iNumFields; i++)
	{
		for (j = 0; j < (pHeader->iNumFields-i-1); j++)
		{
			if ((strcmp (pHeader->tszTitles[j], pHeader->tszTitles[j+1]) > 0)||
                (strcmp (pHeader->tszTitles[j], pHeader->tszTitles[j+1]) == 0)&&
                (strcmp (pHeader->tszLabels[j], pHeader->tszLabels[j+1]) > 0))
			{
				/* the element j is bigger than j+1 one so we swap them    */
				/* we must swap the elements j and j+1 in the three arrays */

				/* swaps the titles */
				pCharAux = pHeader->tszTitles[j];
				pHeader->tszTitles[j] = pHeader->tszTitles[j+1];
				pHeader->tszTitles[j+1] = pCharAux;

				/* swaps the labels */
				pCharAux = pHeader->tszLabels[j];
				pHeader->tszLabels[j] = pHeader->tszLabels[j+1];
				pHeader->tszLabels[j+1] = pCharAux;

				/* swaps the values */
				pCharAux = pHeader->tszValues[j];
				pHeader->tszValues[j] = pHeader->tszValues[j+1];
				pHeader->tszValues[j+1] = pCharAux;
			}
		}
	}

	/* writes the pre-header info in the file */
	fprintf (pFile, "%s", TEXT_COPYRIGHT_NANOTEC);
	fprintf (pFile, "%s", STM_IMAGE_FILE_ID);
	fprintf (pFile, "%s%d\n", IMAGE_HEADER_SIZE_TEXT, HeaderGetSize (pHeader));

	/* In the first time there is no previous title */
	strcpy (szAuxTitle, "");

	/* writes all the fields in the file */
	for (i=0; i<pHeader->iNumFields; i++)
	{
		/* if the title is diferent than the previous one we write the title in the file */
		if (strcmp (szAuxTitle, pHeader->tszTitles[i]) != 0)
		{
			/* Special characteres in the title */
			strcpy (szAuxTitle, pHeader->tszTitles[i]);
			ReplaceStringInString (szAuxTitle, "\\","\\\\");
			ReplaceStringInString (szAuxTitle, "\r\n","\\n");

			/* writes the title in the file */
			fprintf (pFile, "\n[%s]\n\n", szAuxTitle);
		}

		/* Special characteres in the label */
		strcpy (szAuxLabel, pHeader->tszLabels[i]);
		ReplaceStringInString (szAuxLabel, "\\","\\\\");
		ReplaceStringInString (szAuxLabel, "\r\n","\\n");

		/* Special characteres in the value */
		strcpy (szAuxValue, pHeader->tszValues[i]);
		ReplaceStringInString (szAuxValue, "\\","\\\\");
		ReplaceStringInString (szAuxValue, "\r\n","\\n");
		RemoveLeftAndRightWhitesFromString (szAuxValue);

		/* writes the label and the value in the file */
		fprintf (pFile, "    %s: %s\n", szAuxLabel, szAuxValue);
	}

	/* writes the header end */
	fprintf (pFile, "\n[%s]\n",IMAGE_HEADER_END_TEXT);

	return 0;
}

/* Header access to one field */

/* Read */

/***********************************************************************
*
*	Function double HeaderGetAsNumber (HEADER *pHeader, char *szTitle, char *szLabel);
*
*	Description: gets the title-label couple value as a number from the 
*               HEADER object, the HEADER should have been initializated
*               calling HeaderInit
*
*	Inputs:
*			- pHeader: pointer to the HEADER object
*			- szTitle: title of the field to be accesed
*           - szLabel: label of the field to be accesed
*
*	Return value:
*			a double with the title-label couple value, 0 on error
*
***********************************************************************/

double HeaderGetAsNumber	(HEADER *pHeader, char *szTitle, char *szLabel)
{
	char szValue[1000] = "";
	
	if (HeaderGetAsString (pHeader, szTitle, szLabel, szValue) != 0) /* error */
	{
		return 0;
	}

    return atof (szValue);
}

/***********************************************************************
*
*	Function int HeaderGetAsString (HEADER *pHeader, char *szTitle, char *szLabel, char *szValue);
*
*	Description: gets the title-label couple value as a string from the 
*               HEADER object
*
*	Inputs:
*			- pHeader: pointer to the HEADER object
*			- szTitle: title of the field to be accesed
*           - szLabel: label of the field to be accesed
*
*	Outputs:
*			- szValue: will be filled with the title-label couple value as a string
*                     (this must be allocated)
*
*	Return value:
*			0 if the value was correctly accessed, -1 elsewhere
*
***********************************************************************/

int HeaderGetAsString		(HEADER *pHeader, char *szTitle, char *szLabel, char *szValue)
{
	int iIndex;

	/* seeks the title-label couple */
	for (iIndex=0; iIndex<pHeader->iNumFields; iIndex++)
	{
		if ((strcmp (szTitle, pHeader->tszTitles[iIndex]) == 0)&&(strcmp (szLabel, pHeader->tszLabels[iIndex]) == 0))
		{
			/* title-label couple found at position iIndex */
			break;
		}
	}

	if (iIndex == pHeader->iNumFields)
	{
		/* the for loop didn't found the title-label couple in HEADER object */
		return -1;
	}

	/* asign the value of the title-label couple to the pointer szValue */
	strcpy (szValue, pHeader->tszValues[iIndex]);

	/* access succesful */
	return 0;
}

/***********************************************************************
*
*	Function void HeaderReadTitle (char *szLine, char *szTitle);
*
*	Description: reads a title from a line
*
*	Inputs:
*           - szLine: line with the title
*
*   Outputs:
*			- szTitle: title readed from the line (this must be allocated),
*                     szTitle=="" if there is no title in the line.
*
*	Return value:
*			none
*
***********************************************************************/

void HeaderReadTitle (char *szLine, char *szTitle)
{
	int iIndex=0;

	/* If the line don't begin by a left square bracket it isn't a title line */
	if (szLine[0] != '[')
	{
		strcpy (szTitle, "");
		return;
	}

	/* we seek the right square bracket */
	while (szLine[iIndex] != ']')
	{
		iIndex++;
	}

	/* at this point iIndex is the position of the right square bracket */

	/* szLine[1] to skip the left square bracket */
	strncpy (szTitle, &szLine[1], iIndex-1);
	szTitle[iIndex-1] = '\0';
}

/***********************************************************************
*
*	Function void HeaderReadLabel (szLine, szLabel);
*
*	Description: reads a label from a line
*
*	Inputs:
*           - szLine: line with the label
*
*   Outputs:
*			- szLabel: label readed from the line (this must be allocated),
*                     szValue=="" if there is no label in the line.
*
*	Return value:
*			none
*
***********************************************************************/

void HeaderReadLabel (char *szLine, char *szLabel)
{
	unsigned int iIndex=0;

	/* we seek the ':' character */
	while (szLine[iIndex] != ':')
	{
		if (iIndex == strlen (szLine))
		{
			/* there is no label in this line */
			strcpy (szLabel, "");
			return;
		}
		iIndex++;
	}

	/* at this point iIndex is the position of the ':' character */

	/* we copy the label from the line */
	strncpy (szLabel, szLine, iIndex);
	szLabel[iIndex] = '\0';
}

/***********************************************************************
*
*	Function void HeaderReadValue (char *szLine, char *szValue);
*
*	Description: reads a value from a line
*
*	Inputs:
*           - szLine: line with the value
*
*   Outputs:
*			- szLabel: value readed from the line (this must be allocated),
*                     szValue=="" if there is no value in the line.
*
*	Return value:
*			none
*
***********************************************************************/

void HeaderReadValue (char *szLine, char *szValue)
{
	unsigned int iIndex=0;

	/* we seek the ':' character and skip the blank after */
	while (szLine[iIndex] != ':')
	{
		if (iIndex == strlen (szLine))
		{
			/* there is no value in this line */
			strcpy (szValue, "");
			return;
		}
		iIndex++;
	}

	/* skips the ': ' */
	iIndex+=2;

	/* at this point iIndex is the position of the first value character */

	/* szLine[iIndex] to skip the line before ': ' and -1 not to copy return carriage */
	strncpy (szValue, &szLine[iIndex], strlen (szLine)-iIndex-1);
	szValue[strlen (szLine)-iIndex-1] = '\0';
}

/* Write */

/***********************************************************************
*
*	Function void HeaderSetAsFloating (HEADER *pHeader, char *szTitle, char *szLabel, double lfValue);
*
*	Description: sets a title-label couple value in double format to the
*               HEADER object
*
*	Inputs:
*			- pHeader: pointer to the HEADER object to set the value in
*			- szTitle: title of the field to be set
*           - szLabel: label of the field to be set
*           - lfValue: value to be set
*
*	Return value:
*			none
*
***********************************************************************/

void HeaderSetAsFloating	(HEADER *pHeader, char *szTitle, char *szLabel, double lfValue)
{
	char szValue[1000] = "";

	/* converts the double value to a string value */
	sprintf (szValue, "%.6lf",lfValue);

	/* sets the value in the HEADER object as a string */
	HeaderSetAsString (pHeader, szTitle, szLabel, szValue);
}

/***********************************************************************
*
*	Function void HeaderSetAsInt (HEADER *pHeader, char *szTitle, char *szLabel, int iValue);
*
*	Description: sets a title-label couple value in int format to the
*               HEADER object
*
*	Inputs:
*			- pHeader: pointer to the HEADER object to set the value in
*			- szTitle: title of the field to be set
*           - szLabel: label of the field to be set
*           - iValue: value to be set
*
*	Return value:
*			none
*
***********************************************************************/

void HeaderSetAsInt			(HEADER *pHeader, char *szTitle, char *szLabel, int iValue)
{
    char szValue[1000] = "";

	/* converts the int value to a string value */
    sprintf (szValue, "%d", iValue);

	/* sets the value in the HEADER object as a string */
    HeaderSetAsString (pHeader, szTitle, szLabel, szValue);
}

/***********************************************************************
*
*	Function void HeaderSetAsInt (HEADER *pHeader, char *szTitle, char *szLabel, int iValue);
*
*	Description: sets a title-label couple value in string format to the
*               HEADER object
*
*	Inputs:
*			- pHeader: pointer to the HEADER object to set the value in
*			- szTitle: title of the field to be set
*           - szLabel: label of the field to be set
*           - szValue: string value to be set
*
*	Return value:
*			none
*
***********************************************************************/

void HeaderSetAsString		(HEADER *pHeader, char *szTitle, char *szLabel, char *szValue)
{
	int iIndex;

	/* sets the value in the HEADER object */

	/* seeks the title-label couple */
	for (iIndex=0; iIndex<pHeader->iNumFields; iIndex++)
	{
		if ((strcmp (szTitle, pHeader->tszTitles[iIndex]) == 0)&&(strcmp (szLabel, pHeader->tszLabels[iIndex]) == 0))
		{
			/* title-label couple found at position iIndex */
			/* updates the title-label couple value */
			pHeader->tszValues[iIndex] = (char *) realloc (pHeader->tszValues[iIndex], strlen (szValue)+1 * sizeof(char));
			strcpy (pHeader->tszValues[iIndex], szValue);

			return;
		}
	}

	/* title-label couple not found, is a new field */
	HeaderAddValue (pHeader, szTitle, szLabel, szValue);
}

/***********************************************************************
*
*	Function void HeaderDestroy (HEADER *pHeader);
*
*	Description: frees all the allocated memory in the Header
*
***********************************************************************/

void HeaderDestroy (HEADER *pHeader)
{
	int i; /* Counter */

	if (pHeader->iNumFields == 0) /* No need to destroy */
	{
		return;
	}

	for (i=0; i < pHeader->iNumFields; i++)
	{
		free (pHeader->tszTitles[i]);
		free (pHeader->tszLabels[i]);
		free (pHeader->tszValues[i]);
	}

	free (pHeader->tszTitles);
	free (pHeader->tszLabels);
	free (pHeader->tszValues);

	pHeader->tszTitles = NULL;
	pHeader->tszLabels = NULL;
	pHeader->tszValues = NULL;

	pHeader->iNumFields = 0;
}

/* Internally used functions */

/***********************************************************************
*
*	Function int HeaderReadLine (HEADER *pHeader, FILE *pFile);
*
*	Description: Reads a text line from a file. It is important that the file
*				should have been opened for reading, and the HEADER should
*				have been initializated calling HeaderInit
*
*	Inputs:
*			- pHeader: pointer to the HEADER object to store the data in
*			- pFile: File pointer to read the header from
*
*	Outputs:
*			- pHeader: will be filled with the data read from pFile
*			- pFile: the file pointer will go to the end of the header
*
*	Return value:
*			0 if the header was correctly read, -1 elsewhere
*
***********************************************************************/

int HeaderReadLine (HEADER *pHeader, FILE *pFile, char * szTitle)
{
	/* multiplication in the string sizes is because the string can grow later */
	char szLine[MAX_CHARS_IN_HEADER_LINE*2]; 

	char szTryTitle[MAX_CHARS_IN_HEADER_LINE*2];
	char szLabel[MAX_CHARS_IN_HEADER_LINE*2];
	char szValue[MAX_CHARS_IN_HEADER_LINE*2];

	char *szReturn;

	szReturn = fgets (szLine,MAX_CHARS_IN_HEADER_LINE,pFile);

	if (szReturn == NULL)
		return -1; /* File Error */

	/* Line read. We see the data it has */

	RemoveLeftAndRightWhitesFromString (szLine);

	/* Special characteres */

	ReplaceStringInString (szLine,"\\n","\r\n");
	ReplaceStringInString (szLine,"\\\\","\\");

	/* We try to read a label-value couple */

	HeaderReadLabel (szLine,szLabel);
	HeaderReadValue (szLine,szValue);

	/* We try to read it as a title */

    HeaderReadTitle (szLine,szTryTitle);

    if (strlen (szTryTitle) > 0) /* It is a title */
    {
		/* If it indicates the header end, we will return 1 */

		if (strcmp (szTryTitle,IMAGE_HEADER_END_TEXT) == 0)
			return 1;
		else
		{
			strcpy (szTitle,szTryTitle);
			return 0;
		}
    }

	/* If it is not a header field (we have not read any title yet) we ignore it */

	if (strlen (szTitle) == 0)
	{
		return 0;
	}

	/* If it is not a couple label: value, we don't mind about this line */

    if (strlen (szLabel) != 0)
    {
        /* We have a new value. We add it to the header */

		HeaderAddValue (pHeader,szTitle,szLabel,szValue);
    }

	return 0;
}

/***********************************************************************
*
*	Function int HeaderGetSize (HEADER *pHeader);
*
*	Description: Gets the size of the image header in the file.
*
*	Inputs:
*			- pHeader: pointer to the HEADER object end of the header
*
*	Return value:
*			the size of the header, -1 on error
*
***********************************************************************/

int HeaderGetSize (HEADER *pHeader)
{
	int i;
	int iSize = 0;         /* size at this moment                        */
	int iNumOfTitles = 0;  /* Number of titles written in the file       */
	int iNumDigits;        /* Number of digits for the image header size */
	char szTitle[1000] = "", szLabel[1000], szValue[1000]; 

	if (pHeader->bBinary)
	{
		/* fixed size to  512 */
		iSize = 512;
	}
	else
	{
		/* size of the pre-header and the carriage return */
		iSize = sizeof (TEXT_COPYRIGHT_NANOTEC) + sizeof (STM_IMAGE_FILE_ID) + sizeof (IMAGE_HEADER_SIZE_TEXT);

		/* size of all the fields in the HEADER object */
		for (i=0; i<pHeader->iNumFields; i++)
		{
			/* if there is a new title we increase the number of titles and adds its size */
			if (strcmp (szTitle, pHeader->tszTitles[i]) != 0)
			{
				strcpy (szTitle, pHeader->tszTitles[i]);
				iNumOfTitles++;
				iSize += strlen (szTitle);
			}

			strcpy (szLabel, pHeader->tszLabels[i]);

			/* Special characteres in the szLabel string */
			ReplaceStringInString (szLabel,"\\n","\r\n");
			ReplaceStringInString (szLabel,"\\\\","\\");

			strcpy (szValue, pHeader->tszValues[i]);

			/* Special characteres in the szValue string */
			ReplaceStringInString (szValue,"\\n","\r\n");
			ReplaceStringInString (szValue,"\\\\","\\");
			RemoveLeftAndRightWhitesFromString (szValue);

			/* adds the size of the label and the value */
			iSize += strlen (szLabel) + strlen (szValue);
		}

		/* adds the size of the square brackets (2 bytes), the carriage return (3*2 bytes  */ 
		/* for each title)                                                                 */
		iSize += iNumOfTitles * (6 + 2);

		/* adds the size of the carriage return (2 bytes for each label) and the blanks (6 bytes for each label) */
		iSize += pHeader->iNumFields * (6 + 2);

		/* adds the size of the header end text, the square brackets and the carriage return */
		iSize += sizeof (IMAGE_HEADER_END_TEXT) + 2 + 4;

		/* adds the size of the characteres for the image header size in pre-header */
		iNumDigits = (int)floor(log10(iSize)) + 1;
		iSize += iNumDigits;
		if (iNumDigits != (int)floor((log10(iSize)) + 1))
			iSize++;
	}

    return iSize;
}

/***********************************************************************
*
*	Function int HeaderAddValue (HEADER *pHeader, char *szTitle, char *szLabel,char *szValue);
*
*	Description: adds the tripla title-label-value to the HEADER object
*
*	Inputs:
*			- pHeader: pointer to the HEADER object to store the data in
*			- szTitle: title of the field to be added
*           - szLabel: label of the field to be added
*           - szValue: string value to be added
*
*	Return value:
*			0 if the header was correctly updated, -1 elsewhere
*
***********************************************************************/

int	HeaderAddValue (HEADER *pHeader, char *szTitle, char *szLabel,char *szValue)
{
	/* grows the three arrays for the new title-label-value tripla */
	pHeader->tszTitles = (char **) realloc (pHeader->tszTitles, (pHeader->iNumFields+1)*sizeof (char *));
	pHeader->tszLabels = (char **) realloc (pHeader->tszLabels, (pHeader->iNumFields+1)*sizeof (char *));
	pHeader->tszValues = (char **) realloc (pHeader->tszValues, (pHeader->iNumFields+1)*sizeof (char *));

	/* adds the title in the last position of the titles array allocating memory */
	pHeader->tszTitles[pHeader->iNumFields] = (char *) calloc (strlen (szTitle)+1, sizeof(char));
	if (pHeader->tszTitles[pHeader->iNumFields] == NULL) /* error */
	{
		return -1;
	}
	strcpy (pHeader->tszTitles[pHeader->iNumFields], szTitle);

	/* adds the label in the last position of the labels array allocating memory */
	pHeader->tszLabels[pHeader->iNumFields] = (char *) calloc (strlen (szLabel)+1, sizeof(char));
	if (pHeader->tszLabels[pHeader->iNumFields] == NULL) /* error */
	{
		/* frees the memory allocated in this function */
		free (pHeader->tszTitles[pHeader->iNumFields]);

		return -1;
	}
	strcpy (pHeader->tszLabels[pHeader->iNumFields], szLabel);

	/* adds the value in the last position of the values array allocating memory */
	pHeader->tszValues[pHeader->iNumFields] = (char *) calloc (strlen (szValue)+1, sizeof(char));
	if (pHeader->tszValues[pHeader->iNumFields] == NULL) /* error */
	{
		/* frees the memory allocated in this function */
		free (pHeader->tszTitles[pHeader->iNumFields]);
		free (pHeader->tszLabels[pHeader->iNumFields]);

		return -1;
	}
	strcpy (pHeader->tszValues[pHeader->iNumFields], szValue);

	/* increments the number of fields */
	pHeader->iNumFields++;

	return 0;
}

/***********************************************************************
*
*	Function void RemoveLeftAndRightWhitesFromString (char *szString);
*
*	Description: removes the white-space characteres (0x09, 0x0D or 0x20)
*               at the right and left of the string szString.
*
*	Inputs:
*			- szString: the string we want to remove the white-space characteres
*
*   Outputs:
*           - szString: the string without white-space character
*
*	Return value:
*			none
*
***********************************************************************/

void RemoveLeftAndRightWhitesFromString (char *szString)
{
	char szAuxString[1000];
	int iIndex = 0;

	/* Seeks the first non white-space character (0x09, 0x0D or 0x20) */
	while (isspace (szString[iIndex]))
	{
		iIndex++;
	}

	/* We copy the string without white-space characters at the left */
	strcpy (szAuxString, &szString[iIndex]);

	/* Seeks the last non white-space character (0x09, 0x0D or 0x20) */
	iIndex = strlen (szAuxString);
	while (isspace (szString[iIndex]))
	{
		iIndex--;
	}

	/* Puts the zero (mark of end of string) after the last non white-space character */
	szString[iIndex+1] = '\0';

	/* We copy the new szString without white-space characters */
	strcpy (szString, szAuxString);
}

/***********************************************************************
*
*	Function void ReplaceStringInString (char *szDest, const char *szOld, const char *szNew);
*
*	Description: replaces instances of the substring szOld with 
*               instances of the string szNew.
*
*	Inputs:
*			- szDest: the string where will be made the replacemets
*           - szOld: the substring to be replaced by lpszNew
*           - szNew: the substring replacing lpszOld
*
*   Outputs:
*           - szDest: the string with the replacements
*
*	Return value:
*           none
*
***********************************************************************/

void ReplaceStringInString (char *szDest, const char *szOld, const char *szNew)
{
	int iIndex=0, iAuxStringIndex=0;
	char szAuxString[1000];

	/* We copy the string with the replacements to the auxiliar string */

	/* Searches for the substring szOld */
	while (szDest[iIndex]!='\0') /* while not end of string */
	{
		if (strncmp (&szDest[iIndex], szOld, strlen(szOld)) == 0)
		{
			/* Writes the substring szNew in the substring szOld position */
			strcpy (&szAuxString[iAuxStringIndex], szNew);
			iAuxStringIndex += strlen (szNew);
			iIndex += strlen (szOld);
		}
		else
		{
			/* We copy the character without any change */
			szAuxString[iAuxStringIndex] = szDest [iIndex];
			iAuxStringIndex++;
			iIndex++;
		}
	}

	/* Puts the zero (mark of end of string) to the auxiliar string */
	szAuxString[iAuxStringIndex] = '\0';

	/* We copy the new string to szDest */
	strcpy (szDest, szAuxString);
}

