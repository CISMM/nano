/*===3rdtech===
  Copyright (c) 2003 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BCGrid.h"
#include "BCPlane.h"
#include "wsxmHeader.h"

#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 4996 )
#endif

// local defines
#define CTL_Z	(26)
#define BSLASH	'\\'
#define RET	(15)
#define NL	'\n'

#define SWAP(a,b)       {((a)^=(b));((b)^=(a));((a)^=(b));}

/** Read Nanotec Electronica WSxM file, version 2 or 3
*/
int 
BCGrid::readNanotecFile(const char* filename, const char *name, int ascii_flag)
{
    FILE * file;
    char szBuffer[50] = "";
    char units[50];
    //char *units_name;
    nmb_NanotecImageInfo header;
    HeaderInit (&header);
    if (ascii_flag == 0) {
        // Open as text, first. 
        file = fopen (filename,"rt");
    } else {
        file = fopen (filename,"rb");
    }
    if (file == NULL) {
      fprintf(stderr, "BCGrid::readNanotecFile: couldn't open file.\n");
      return -1;
    }
    if (HeaderRead (&header,file) == -1) {
	fprintf(stderr, "BCGrid::readNanotecFile: could not parse header!\n");
        return -1;
    }
    
    // Closes and open the source file for reading in binary mode the image
    // data
    if (ascii_flag == 0) {
        fclose(file);
        file = fopen (filename,"rb");
    }
    if (file == NULL) {
      fprintf(stderr, "BCGrid::readNanotecFile: couldn't re-open file.\n");
      return -1;
    }

    // move file pointer to first position at end of header
    if (fseek(file, HeaderGetSize(&header), SEEK_SET )) {
	perror("BCGrid::readNanotecFile: could not move file pointer!");
	return -1;
    }

    // Get scan size info from header. 
    if(HeaderGetAsString (&header, IMAGE_HEADER_CONTROL, IMAGE_HEADER_CONTROL_X_AMPLITUDE,szBuffer)) {
        fprintf(stderr, "BCGrid::readNanotecFile: No scansize in header!\n");
        return -1;
    }
    if (sscanf(szBuffer, "%lg %s", 
               &header.x_amplitude, units) <=0) {
        fprintf(stderr, "BCGrid::readNanotecFile: scansize problem in header!\n");
        return -1;
    }
    strcpy(header.scan_units, units);
    if(HeaderGetAsString (&header, IMAGE_HEADER_CONTROL, IMAGE_HEADER_CONTROL_Y_AMPLITUDE,szBuffer)) {
        fprintf(stderr, "BCGrid::readNanotecFile: No scansize in header!\n");
        return -1;
    }
    if (sscanf(szBuffer, "%lg %s", 
               &header.y_amplitude, units) <=0) {
        fprintf(stderr, "BCGrid::readNanotecFile: scansize problem in header!\n");
        return -1;
    }
    if (strcmp(header.scan_units, units) !=0) {
        fprintf(stderr, "BCGrid::readNanotecFile: "
                "Scan units mismatch, x & y, possible data distortion.\n");
    }

    _max_x = 0.5 * header.x_amplitude;
	
    _max_y = 0.5 * header.y_amplitude;
    if (strcmp(header.scan_units, "\265m") == 0) {
        // microns, translate to nm
        _max_x *= 1000.0;
        _max_y *= 1000.0;
    } else if (strcmp(header.scan_units, "\305") == 0) {
        // Angstroms, translate to nm
        _max_x *= 0.1;
        _max_y *= 0.1;
    }
    _min_x = -_max_x;
    _min_y = -_max_y;

    // We know the file size, set our own grid size.
    // Planes added later adopt new size 
    // X and Y are reversed in Nanotec files
    _num_y = (int) HeaderGetAsNumber (&header, IMAGE_HEADER_GENERAL_INFO, IMAGE_HEADER_GENERAL_INFO_NUM_COLUMNS);
    _num_x = (int) HeaderGetAsNumber (&header, IMAGE_HEADER_GENERAL_INFO, IMAGE_HEADER_GENERAL_INFO_NUM_ROWS);

    // Finally, get info about Z units and amplitude, for use by BCPlane
    if(HeaderGetAsString (&header, IMAGE_HEADER_GENERAL_INFO, IMAGE_HEADER_GENERAL_INFO_Z_AMPLITUDE,szBuffer)) {
        fprintf(stderr, "BCGrid::readNanotecFile: No z size in header!\n");
        return -1;
    }
    if (sscanf(szBuffer, "%lg %s", 
               &header.z_amplitude, header.z_units) <=0) {
        fprintf(stderr, "BCGrid::readNanotecFile: z size problem in header!\n");
        return -1;
    }
    
    char pl_name[255];
    BCPlane * plane;
    sprintf(pl_name, "%s", name);
        
    plane = addNewPlane(pl_name, header.z_units, NOT_TIMED);
    plane->readNanotecFile(file, &header);
    
    char scrap;
    
    if (fread(&scrap, 1, 1, file) == 1)
	perror("BCGrid::readNanotecFile: WARNING: Not at file end upon completion!");
        
    fclose(file);

    return 0;
    
} // readNanotecFile


/**
   Parse a v2 or later Nanotec header.
 To Do: handle other image data types 
int
BCGrid::parseNanotecV2 (nmb_NanotecImageInfo * curr_info, char * header_text)
{
    int	ngot = 1;
    size_t nspace = 0;
    char * ptoken;
    char units[50];

    ptoken = strtok(header_text, "\n");
    // Scan the file until we get to the end or find "[Control]" section
    while ( (NULL != ptoken ) && (CTL_Z != *ptoken) &&
	    strncasecmp(ptoken, "[Control]", 
                         strlen("[Control]")) ) {
        ptoken = strtok(NULL, "\n");
    }
    if ((NULL == ptoken) || (CTL_Z == *ptoken)) {
	fprintf(stderr, "BCGrid::parseNanotecFileHeader: Can't find [Control] header!\n");
	return -1;
    }
    ptoken = strtok(NULL, "\n");
    while ( (NULL != ptoken ) && (CTL_Z != *ptoken) ) {
        // Skip spaces
        if ((nspace = strspn(ptoken, " \t") ) > 0) {
            ptoken += nspace;
        }
        // Break out if we see another header, starts with [
        if (ptoken[0] == '[') break;
        // Here's the list of params we recognize, and they should all be
        // present for each image layer, we think. 
        if (!strncasecmp(ptoken, "Signal Gain:", 
                         strlen("Signal Gain:"))) {
            ngot = sscanf(ptoken + strlen("Signal Gain: "),"%lg", 
                   &curr_info->signal_gain);
        } else if (!strncasecmp(ptoken, "X Amplitude:", 
                                strlen("X Amplitude:"))) {
            ngot = sscanf(ptoken + strlen("X Amplitude: "),"%lg %s", 
                   &curr_info->x_amplitude, units);
            if(curr_info->scan_units[0] == '\0' ) strcpy(curr_info->scan_units, units);
            else if (strcmp(curr_info->scan_units, units) !=0) {
               fprintf(stderr, "BCGrid::parseNanotecFileHeader: "
               "Scan units mismatch, x & y, possible data distortion.\n");
            }
        } else if (!strncasecmp(ptoken, "Y Amplitude:", 
                                strlen("Y Amplitude:"))) {
            ngot = sscanf(ptoken + strlen("Y Amplitude: "),"%lg %s", 
                   &curr_info->y_amplitude, units);
            if(curr_info->scan_units[0] == '\0' ) strcpy(curr_info->scan_units, units);
            else if (strcmp(curr_info->scan_units, units) !=0) {
               fprintf(stderr, "BCGrid::parseNanotecFileHeader: "
               "Scan units mismatch, x & y, possible data distortion.\n");
            }
        } else if (!strncasecmp(ptoken, "Z Gain:", 
                         strlen("Z Gain:"))) {
            ngot = sscanf(ptoken + strlen("Z Gain: "),"%lg", 
                   &curr_info->z_gain);
        }
        if (ngot <= 0) { 
            fprintf(stderr, "BCGrid::parseNanotecFileHeader: "
                    "Error reading header parameter!\n");
            return -1;
        }
        ptoken = strtok(NULL, "\n");
    }
    if ((NULL == ptoken) || (CTL_Z == *ptoken) || 
        strncasecmp(ptoken, "[General Info]", 
                    strlen("[General Info]")) ) {
	fprintf(stderr, "BCGrid::parseNanotecFileHeader: Can't find [General Info] header!\n");
	return -1;
    }
    // Parse the parameters we know about in this section. 
    ptoken = strtok(NULL, "\n");
    while ( (NULL != ptoken ) && (CTL_Z != *ptoken) ) {
        // Skip spaces
        if ((nspace = strspn(ptoken, " \t") ) > 0) {
            ptoken += nspace;
        }
        // Break out if we see another header, starts with [
        if (ptoken[0] == '[') break;
        // Here's the list of params we recognize, and they should all be
        // present for each image layer, we think. 
        if (!strncasecmp(ptoken, "Head type:", 
                         strlen("Head type:"))) {
            ngot = sscanf(ptoken + strlen("Head type: "),"%s", 
                   &curr_info->head_type);
        } else if (!strncasecmp(ptoken, "Number of columns:", 
                                strlen("Number of columns:"))) {
            ngot = sscanf(ptoken + strlen("Number of columns: "),"%d", 
                   &curr_info->num_x);
        } else if (!strncasecmp(ptoken, "Number of rows:", 
                                strlen("Number of rows:"))) {
            ngot = sscanf(ptoken + strlen("Number of rows: "),"%d", 
                   &curr_info->num_y);
        } else if (!strncasecmp(ptoken, "Z Amplitude:", 
                         strlen("Z Amplitude:"))) {
            ngot = sscanf(ptoken + strlen("Z Amplitude: "),"%lg %s", 
                   &curr_info->z_amplitude, curr_info->z_units);
            
        }
        if (ngot <= 0) { 
            fprintf(stderr, "BCGrid::parseNanotecFileHeader: "
                    "Error reading header parameter!\n");
            return -1;
        }
        ptoken = strtok(NULL, "\n");
    }

//      if (curr_info->z_scale == -1) {
//          fprintf(stderr, "BCGrid::parseNanotecFileHeader: "
//                  "Required Z scale param not read.\n");
//          return -1;
//      }
    return 0;

}
*/

#ifdef _WIN32
#pragma warning( pop )
#endif
