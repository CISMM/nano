/*===3rdtech===
  Copyright (c) 2000-2002 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BCGrid.h"
#include "BCPlane.h"
#include "readNanoscopeFile.h"

/* local defines
**/
#define CTL_Z	(26)
#define BSLASH	'\\'
#define RET	(15)
#define NL	'\n'

#define SWAP(a,b)       {((a)^=(b));((b)^=(a));((a)^=(b));}

#ifdef	_WIN32
// Windows doesn't have the strncasecmp function.
int	strncasecmp(const char *s1, const char *s2, size_t n)
{
	unsigned i = 0;	// Index passing through the characters

	for (i = 0; i < n; i++) {
		// See if we've reached the end of one or both strings
		if ( s1[i] == 0 ) {
			if ( s2[i] == 0 ) return 0;
			else return -1;
		} else if ( s2[i] == 0 ) {
			return 1;
		}

		// See if this character breaks the tie
		if ( tolower(s1[i]) < tolower(s2[i]) ) return -1;
		if ( tolower(s1[i]) > tolower(s2[i]) ) return 1;
	}
	return 0;	// Got to the end of the characters to test
}
#endif

/** Reads Version 2 of a DI NanoScope file */
int 
BCGrid::readNanoscopeFileWithoutHeader(FILE* file, const char *filename)
{
    int	i_scrap;
    char s_scrap[255];

    // the first line in the file should contain "_File_Type 7" 
    // or "Data_File_Type 7"
    if (fscanf(file, "%s %d", s_scrap, &i_scrap) != 2)
    {
	perror("BCGrid::readNanoscopeFileWithoutHeader: could not read file type!");
	return -1;
    }	
    if (i_scrap != 7)
    {
	fprintf(stderr,"BCGrid::readNanoscopeFile: unknown file type");
	fprintf(stderr,"   (I can only read type 7, this is type %d)\n",
		i_scrap);
	return -1;
    }	

    char line[255];

    // look for line containing "num_samp = [num]" 
    do
    {
	fgets(line, sizeof(line)-1, file);
	if (strncmp(line, "num_samp", strlen("num_samp")) == 0)
	{
	    if (sscanf(line, "%s %s %d", s_scrap, s_scrap, &i_scrap) != 3)
	    {
		fprintf(stderr,"Error in BCGrid::readNanoscopeFile: could not  parse line containing number of samples!\n");
		return -1;
	    }
	    break; // found line sought so leave loop
	}
    } while (!feof(file));
    
    if (feof(file)) // never found line containing "num_samp = [num]" 
    { 
	fprintf(stderr,"Error in BCGrid::readNanoscopeFile: could not find number of samples!\n");
	return -1;
    }

    _num_x = _num_y = i_scrap;
    // We know the file size, set our own grid size.
    setGridSize(_num_x, _num_y);

    // guess min/max x, and y
    // XXX bug, can get from "scan_sz =" line. 
    _min_x = -10;
    _max_x = 10;
    _min_y = -10;
    _max_y = 10;

    // Weird. Isn't header fixed size, 2048 bytes?
    if (fseek(file, (long)(-_num_x * _num_y * 2), 2) != 0)
    {
	perror("BCGrid::readNanoscopeFileWithoutHeader: could not move file pointer!");
	return -1;
    }

    BCPlane* plane = addNewPlane(filename, "nm", TIMED);

    // XXX. Probably need "z_scale = " parameter, to interpret data correctly.
    if (plane->readNanoscopeFileWithoutHeader(file) == -1)
	return -1;
    
    float scrap;
    
    if (fscanf(file,"%f",&scrap) == 1) 
    {
	fprintf(stderr, "BCGrid::readNanoscopeFile: Not at file end upon completion!\n");
	return 1;
    }

    return 0;
} // readNanoscopeFileWithoutHeader

/** Read DI NanoScope file, version 4. There are at least two flavors, this
    handles them all. 
*/
int 
BCGrid::readNanoscopeFile(FILE* file, const char *filename, int ascii_flag)
{
    char *units_name;
    nmb_diImageInfo file_info;
    if (parseNanoscopeFileHeader(file, &file_info))
    {
	fprintf(stderr, "BCGrid::readBinaryNanoscopeFile: could not parse header!\n");
	return -1;
    }

    // move file pointer to first position at end of header
    if (fseek(file, file_info.data_offset, SEEK_SET ))
    {
	perror("BCGrid::readBinaryNanoscopeFile: could not move file pointer!");
	return -1;
    }

    _min_x = _min_y = 0.0;
    if (file_info.image_mode == NS_HEIGHT_V44) {
        if (strcmp(file_info.scan_units, "~m") == 0) {
            // microns, translate to nm
            _max_x = file_info.scan_size * 1000.0;
            _max_y = file_info.scan_size_y * 1000.0;
        } else {
            // nanometers. Maybe other units?
            _max_x = file_info.scan_size;
            _max_y = file_info.scan_size_y;
        }            
    } else {
        _max_x = _max_y = file_info.scan_size;
    }

    switch (file_info.image_mode) {
        case NS_HEIGHT_V44:
            if (strcmp(file_info.image_data_type, "Zscan") ==0) {
		units_name = (char *)"nm";
            } else if (strcmp(file_info.image_data_type, "Amplitude") ==0) {
		units_name = (char *)"V?";
            } else {
		units_name = (char *)"unknown";
            }
            break;
        case NS_HEIGHT_V41:
            units_name = file_info.z_units;
		break;
	case NS_HEIGHT:
	case NS_DEFLECTION:
		units_name = (char *)"nm";
		break;
	case NS_AUXC:
		units_name = (char *)"V";
		break;
	case NS_CURRENT:
		units_name = (char *)"nA";
		break;
	default:
		units_name = (char *)"unknown";
    }
    BCPlane* plane = addNewPlane(filename, units_name, TIMED);
    
    if (ascii_flag) {
        plane->readAsciiNanoscopeFile(file, &file_info );
    } else {
        plane->readBinaryNanoscopeFile(file, &file_info);
    }
    int  i = 2; // Start with 2 for easy user readability. 
    nmb_diImageInfo * aux_info = file_info.next;
    while (aux_info) {
        char name[255];
        sprintf(name, "%s layer %d", filename, i);
        
        switch (aux_info->image_mode) {
        case NS_HEIGHT_V44:
            if (strcmp(aux_info->image_data_type, "Zscan") ==0) {
		units_name = (char *)"nm";
            } else if (strcmp(aux_info->image_data_type, "Amplitude") ==0) {
		units_name = (char *)"V?";
            } else {
		units_name = (char *)"unknown";
            }
            break;
        case NS_HEIGHT_V41:
            units_name = aux_info->z_units;
		break;
	case NS_HEIGHT:
	case NS_DEFLECTION:
		units_name = (char *)"nm";
		break;
	case NS_AUXC:
		units_name = (char *)"V";
		break;
	case NS_CURRENT:
		units_name = (char *)"nA";
		break;
	default:
		units_name = (char *)"unknown";
        }
        plane = addNewPlane(name, units_name, NOT_TIMED);
        
        if (ascii_flag) {
            plane->readAsciiNanoscopeFile(file, aux_info);
        } else {
            plane->readBinaryNanoscopeFile(file, aux_info);
        }
        aux_info = aux_info->next;
        i++;
	
    }
    
    char scrap;
    
    if (fread(&scrap, 1, 1, file) == 1)
	perror("BCGrid::readBinaryNanoscopeFile: WARNING: Not at file end upon completion!");
        
    fclose(file);

    //No memory leaks!
    nmb_diImageInfo * next_info;
    aux_info = file_info.next;
    while (aux_info) {
        next_info = aux_info->next;
        delete aux_info;
        aux_info = next_info;
    }
    return 0;
    
} // readNanoscopeFile



	
/**
parseNanoscopeFileHeader
   description: This method parses the headers of binary or ascii Nanoscope
                files. In the process, it ignores information that is not
		needed (or understood).
        author: Mark Finch 9-9-96 by Russ Taylor
 last modified: 1-21-02 Aron Helser
*/
int
BCGrid::parseNanoscopeFileHeader(FILE* file, nmb_diImageInfo * file_info)
{
    //char image_type[BUFSIZ];
    //int	pastline;
	
    // counters
    int scale_count = 0;	// How many "Z scale height" entries found?
    int scale_count_2 = 0;	// How many "Z scale" entries found?
    int image_count = 0;	// How many image sets in this file?

    char token[BUFSIZ];
    char *ptoken;
    
    // Get a line from the file. May be a first, partial line. 
    if (fgets(token, BUFSIZ, file)== NULL) return -1;
    // Scan the file until we get to the end or find length of header
    while ( (CTL_Z != *token ) &&
	    (strncasecmp(token, "\\Data length", strlen("\\Data length")) )) {
        if (fgets(token, BUFSIZ, file)== NULL) return -1;
    }

    // Make sure we found a backslash; if not, no header
    if (CTL_Z == *token) {
	fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: Can't read header!\n");
	return -1;
    }
    // Find header length
    if (sscanf(token+12, ": %d", &file_info->data_offset) != 1) {
	fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: Can't read header size!\n");
	return -1;
    }
    // read rest of header
    char * header_text = new char[file_info->data_offset +1];
    int hpos = ftell(file);
    if (hpos < 0) {
	fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: File I/O error!\n");
	return -1;
    }

    if (fread(header_text, sizeof(char), file_info->data_offset - hpos, file) 
        != (unsigned)(file_info->data_offset - hpos)) {
	fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: Can't read header body\n");
	return -1;
    }

    // Scan forward to next divider to determine which version we are reading
    ptoken = strtok(header_text, "\n");
    // Scan the file until we get to the end or find "\*"
    while ( (NULL != ptoken ) && (CTL_Z != *ptoken) &&
	    (strncasecmp(ptoken, "\\*", strlen("\\*")) )) {
        ptoken = strtok(NULL, "\n");
    }
    if ((NULL == ptoken) || (CTL_Z == *ptoken)) {
	fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: Can't read header!");
	return -1;
    }
    printf("*DI Interpreting file header, ");
    if (!strncasecmp(ptoken, "\\*Stm list", strlen("\\*Stm list")) ||
        !strncasecmp(ptoken, "\\*Afm list", strlen("\\*Afm list")) ) {
        printf("Version 2.5 - can't read, call 3rdTech!!!\n");
        return -1;
    } else if (!strncasecmp(ptoken, "\\*NC Afm list", strlen("\\*NC Afm list"))) {
        printf("Version 4.1\n");
        return (parseNSv4_1(file_info));

    } else if (!strncasecmp(ptoken, "\\*Equipment list", strlen("\\*Equipment list"))) {
        printf("Version 4.4\n");
        return (parseNSv4_4(file_info));
        
    } else {
	fprintf(stderr, "Unrecognized separator in header, %s!", ptoken);
	return -1;
    }
    // We know the file size, set our own grid size.
    // Unnecessary -> planes added later adopt new size 
    //setGridSize(_num_x, _num_y);

    // Shouldn't be reached, unhandled version. 
    return -1;        
    
}

/**
   Parse a 4.1 Nanoscope header, probably also a 4.2
 Depends on strtok context from parseNanoscopeFileHeader!!!
*/
int
BCGrid::parseNSv4_1 (nmb_diImageInfo * file_info)
{
    int	ngot = 1;
    char * ptoken;
    ptoken = strtok(NULL, "\n");
    // Scan the file until we get to the end or find "\*NCAFM image list"
    // We also have one file where separator is \*Image list, 
    // otherwise the same. 
    while ( (NULL != ptoken ) && (CTL_Z != *ptoken) &&
	    strncasecmp(ptoken, "\\*NCAFM image list", 
                         strlen("\\*NCAFM image list")) && 
             strncasecmp(ptoken, "\\*Image list", 
                         strlen("\\*Image list")) ) {
        ptoken = strtok(NULL, "\n");
    }
    if ((NULL == ptoken) || (CTL_Z == *ptoken)) {
	fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: Can't find image sub-header!\n");
	return -1;
    }
    file_info->image_mode = NS_HEIGHT_V41;
    nmb_diImageInfo * curr_info = file_info;

    ptoken = strtok(NULL, "\n");
    while ( (NULL != ptoken ) && (CTL_Z != *ptoken) ) {
        // Here's the list of params we recognize, and they should all be
        // present for each image layer, we think. 
        if (!strncasecmp(ptoken, "\\Data offset:", 
                         strlen("\\Data offset:"))) {
            ngot = sscanf(ptoken + strlen("\\Data offset: "),"%d", 
                   &curr_info->data_offset);
        } else if (!strncasecmp(ptoken, "\\Samps/line:", 
                                strlen("\\Samps/line:"))) {
            ngot = sscanf(ptoken + strlen("\\Samps/line: "),"%d", 
                   &_num_x);
        } else if (!strncasecmp(ptoken, "\\Number of lines:", 
                                strlen("\\Number of lines:"))) {
            ngot = sscanf(ptoken + strlen("\\Number of lines: "),"%d", 
                   &_num_y);
        } else if (!strncasecmp(ptoken, "\\Scan size:", 
                                strlen("\\Scan size:"))) {
            ngot = sscanf(ptoken + strlen("\\Scan size: "),"%lg %s", 
                   &curr_info->scan_size, curr_info->scan_units);
        } else if (!strncasecmp(ptoken, "\\Z scale:", strlen("\\Z scale:"))) {
            ngot = sscanf(ptoken + strlen("\\Z scale: "),"%lg %2s", 
                   &curr_info->z_scale, curr_info->z_units);
        } else if (!strncasecmp(ptoken, "\\*NCAFM image list", 
                                strlen("\\*NCAFM image list")) ||
                   !strncasecmp(ptoken, "\\*Image list", 
                                strlen("\\*Image list"))) {
            if (curr_info->z_scale == -1) {
                fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: "
                        "Required Z scale param not read.\n");
                return -1;
            }
            // We found second image information. Allocate and use
            // new image object.
            printf("DI Second image layer found. Reading header info.\n");
            curr_info->next = new nmb_diImageInfo();
            curr_info = curr_info->next;
            curr_info->image_mode = NS_HEIGHT_V41;
        }
        if (ngot <= 0) { 
            fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: "
                    "Error reading header parameter!\n");
            return -1;
        }
        ptoken = strtok(NULL, "\n");
    }
    if (curr_info->z_scale == -1) {
        fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: "
                "Required Z scale param not read.\n");
        return -1;
    }
    return 0;

}
/**
   Parse a 4.4 Nanoscope header, probably also later formats, if they exist.
 Depends on strtok context from parseNanoscopeFileHeader!!!
*/
int
BCGrid::parseNSv4_4 (nmb_diImageInfo * file_info)
{
    int	ngot = 1;
    char * ptoken;
    ptoken = strtok(NULL, "\n");
    // Scan the file until we get to the end or find "\*Ciao image list"
    while ( (NULL != ptoken ) && (CTL_Z != *ptoken) &&
	    (strncasecmp(ptoken, "\\*Ciao image list", 
                         strlen("\\*Ciao image list")) )) {
        ptoken = strtok(NULL, "\n");
    }
    if ((NULL == ptoken) || (CTL_Z == *ptoken)) {
	fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: Can't find image sub-header!\n");
	return -1;
    }
    file_info->image_mode = NS_HEIGHT_V44;
    nmb_diImageInfo * curr_info = file_info;

    ptoken = strtok(NULL, "\n");
    while ( (NULL != ptoken ) && (CTL_Z != *ptoken) ) {
        // Here's the list of params we recognize, and they should all be
        // present for each image layer, we think. 
        if (!strncasecmp(ptoken, "\\Data offset:", 
                         strlen("\\Data offset:"))) {
            ngot = sscanf(ptoken + strlen("\\Data offset: "),"%d", 
                   &curr_info->data_offset);
        } else if (!strncasecmp(ptoken, "\\Samps/line:", 
                                strlen("\\Samps/line:"))) {
            ngot = sscanf(ptoken + strlen("\\Samps/line: "),"%d", 
                   &_num_x);
        } else if (!strncasecmp(ptoken, "\\Number of lines:", 
                                strlen("\\Number of lines:"))) {
            ngot = sscanf(ptoken + strlen("\\Number of lines: "),"%d", 
                   &_num_y);
        } else if (!strncasecmp(ptoken, "\\Scan size:", 
                                strlen("\\Scan size:"))) {
            ngot = sscanf(ptoken + strlen("\\Scan size: "),"%lg %lg %s", 
                   &curr_info->scan_size, &curr_info->scan_size_y, 
                          curr_info->scan_units);
        } else if (!strncasecmp(ptoken, "\\@2:Z scale:", 
                                strlen("\\@2:Z scale:"))) {
            ngot = sscanf(ptoken + strlen("\\@2:Z scale: "),
                          "V [Sens. %[^]]] (%*g V/LSB) %lg V", 
                   curr_info->image_data_type, &curr_info->z_scale);
        } else if (!strncasecmp(ptoken, "\\*Ciao image list", 
                                strlen("\\*Ciao image list"))) {
            if (curr_info->z_scale == -1) {
                fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: "
                        "Required Z scale param not read.\n");
                return -1;
            }
            // We found second image information. Allocate and use
            // new image object.
            printf("DI Second image layer found. Reading header info.\n");
            curr_info->next = new nmb_diImageInfo();
            curr_info = curr_info->next;
            curr_info->image_mode = NS_HEIGHT_V44;
        }
        if (ngot <= 0) { 
            fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: "
                    "Error reading header parameter!\n");
            return -1;
        }
        ptoken = strtok(NULL, "\n");
    }
    if (curr_info->z_scale == -1) {
        fprintf(stderr, "BCGrid::parseNanoscopeFileHeader: "
                "Required Z scale param not read.\n");
        return -1;
    }
    return 0;

}
/*
Not yet obsolete, may be useful in interpreting version 3 file, as 
I only have docs for version 2.5 and 4.1,
Aron Helser 1/25/02
int
BCGrid::parseNSv3maybe ()
{
    char image_type[BUFSIZ];
    int	ngot = 1;
    int	pastline;
	
    // counters
    int scale_count = 0;	// How many "Z scale height" entries found?
    int scale_count_2 = 0;	// How many "Z scale" entries found?
    int image_count = 0;	// How many image sets in this file?

    auxiliary_grids = -1;
	
    char token[BUFSIZ];
    do {
	// For ASCII file parsing, this is used to watch for a completely
	// blank line.  This indicates we didn't just pass one.
	pastline = 0;

	// Read in all characters up to but not including a colon or a
	// newline.  The colon indicates a value follows on the same
	// line; the newline indicates we've gotten to the next line
	// in the file (which starts a new token). 
        // We strip the backslash which starts all tokens at the end 
        // of this loop

	fscanf(file, "%[^:\\\n]", token);

	// Check to see if we recognize the token.  If so, gather the
	// parameter (after the colon).

	if (!strncasecmp(token, "Scan size", strlen("Scan size"))) {
            if (scan_size < 0) {
                ngot = fscanf(file, ": %lg", &scan_size);
            } else {
                // Ignore it if we've seen one before. 
                 fscanf(file, ": %*lg");
                 ngot =1;
            }                
	    printf("DI Scan size is %g\n",scan_size);
	} else if (!strncasecmp(token, "Z atten.", strlen("Z atten."))) {
	    ngot = fscanf(file, ": %lg", &_attenuation_in_z);
	    printf("DI Z atten is %g\n",_attenuation_in_z);
	} else if (!strncasecmp(token, "Z scale auxc", strlen("Z scale auxc"))){
	    ngot = fscanf(file, ": %lg", &_z_scale_auxc );
	    printf("DI Z scale auxc is %g\n",_z_scale_auxc);
	} else if (!strncasecmp(token,"Z scale height",
                                strlen("Z scale height"))) {
	    // read multi-layer data (added by qliu on 6/27/95)
	    if (scale_count > 0) {
		ngot = fscanf(file, ": %lg",
			&(auxiliary_grid_scale[scale_count-1]));
	    } else {
		ngot = fscanf(file, ": %lg", &_z_scale );
            }
	    printf("DI Z scale height is %g\n",_z_scale);
	    scale_count++;
	} else if (!strncasecmp( token, "Z scale", strlen(token))) {
	    // XXX This used to have code to read multi-mode data, written
	    // by Qiang Liu.  The problem is that they seem to have changed
	    // the format (or it doesn't match the book), and when you do
	    // a resampling of a file, it puts out the actual scale to units
	    // of nm on the line, rather than an index into a list of scales.
	    // I've changed it to work with this type of file.  I hope it will
	    // also work with multi-mode types still.
	    // I'm working by the seat of my pants and noticing that the scale
	    // looks very close when we take the number in parentheses:
	    //    \Z scale: 47.0512 nm (862)
	    // rather than the 47, we take the 862.  Is this generally true???
	    double scrapd;
	    char   scraps[100];
	    if (scale_count_2 > 0) {
		ngot = fscanf(file, ": %lg %s (%lg)", &scrapd, scraps,
			&(auxiliary_grid_scale[scale_count_2-1]));
	    } else {
		ngot = fscanf(file, ": %lg %s (%lg)", &scrapd,scraps,&_z_scale);
		if (ngot == 0) {
		  fprintf(stderr,
			"ERROR: Can't read Z scale params in Nanoscope file\n");
		  return -1;
		}
		printf("DI Z scale is %g\n",_z_scale);
	    }
	    scale_count_2++;
	}     
	else if (!strncasecmp(token, "Image data", strlen("Image data")))
	{
	    // read  multi-layer data (added by qliu 6/27/95)
	    if (image_count > 0) {	// Assume all but first are NS_AUXC
		char temp[BUFSIZ]; // not used for now
		ngot = fscanf(file, ": %s", temp);
		auxiliary_grid_image_mode[image_count-1] = NS_AUXC;
	    } else {
		ngot = fscanf(file, ": %s", image_type);
	    }

	    image_count++;
	}

	else if (!strncasecmp(token, "Detect sens.", strlen("Detect sens."))) {
		ngot = fscanf(file, ": %lg", &_detection_sensitivity );
		printf("DI Detector sens. is %g\n",_detection_sensitivity);
	} else if (!strncasecmp(token, "Z sensitivity", strlen(token))) {
		ngot = fscanf(file, ": %lg", &_z_sensitivity);
		printf("DI Z sensitivity is %g\n",_z_sensitivity);
	} else if (!strncasecmp(token, "In sensitivity",strlen(token))) {
		ngot = fscanf(file, ": %lg", &_input_sensitivity);
		printf("DI In sensitivity is %g\n",_input_sensitivity);
	} else if (!strncasecmp(token, "Z max", strlen("Z max"))) {
		ngot = fscanf(file, ": %lg", &_z_max);
		printf("DI Z max is %g\n",_z_max);
	} else if (!strncasecmp(token, "In1 max", strlen("In1 max"))) {
		ngot = fscanf(file, ": %lg", &_input_1_max);
		printf("DI In1 max is %g\n",_input_1_max);
	} else if (!strncasecmp(token, "In2 max", strlen("In2 max"))) {
		ngot = fscanf(file, ": %lg", &_input_2_max);
		printf("DI In2 max is %g\n",_input_2_max);
	} else if (!strncasecmp(token, "Samps/line", strlen("Samps/line"))) {
	    ngot = fscanf(file, ": %hd %hd", &_num_x, &_num_y);
	    
	    if (ngot < 2) {
		_num_y =_num_x;
	    }
            printf("DI Samps/line is %d\n",_num_x);
	} else if (!strncasecmp(token, "Lines", strlen("Lines"))) {
	    ngot = fscanf(file, ": %hd", &_num_y);
            printf("DI Lines is %d\n",_num_y);
	} else if (!strncasecmp(token, "Data offset", strlen("Data offset"))) {
	    // read multi-layer data (added by qliu on 6/27/95)
	    if (auxiliary_grids >= 0)
		ngot = fscanf(file, ": %d", &(auxiliary_grid_offset[auxiliary_grids]) );
	    else
		ngot = fscanf(file, ": %d", &header_size);

	    auxiliary_grids++;
            printf("DI Data Offset is %d\n",header_size);
	} else if (!strncasecmp(token, "*File list end", strlen("*File list end"))) {
            printf("*DI End of header\n");
        }
	// Since ngot is set to 1 to start with and only reset when we are
	// reading parameters, if it is zero that means we couldn't get some
	// parameter we were looking for.
	if (!ngot) {
	    fprintf(stderr,
		"BCGrid:parseNanoscopeFileHeader: error reading %s!\n", token);
	    return -1;
	}

        // skip to next token or end of header
	// (EOF is CTL_Z on binary or blank line on Ascii)
	while ( (CTL_Z != *token)
		&&
		(CTL_Z != (*token = fgetc(file)))
		&&
		(BSLASH != *token) )
	{
	    // If we passed a newline and have passed no tokens since, we
	    // must be at the end of the header.  Mark as CTL_Z to drop us
	    // out of the parsing loop.	    
		if (NL == *token) {
		if (pastline)
		    *token = CTL_Z;
		else
		    pastline = 1;
	    }

		// If the token is -1, we are at the end of the file, so quit.
		// This is needed in Cygwin (windows NT), but not in the Unix
		// code.
#ifdef	_WIN32
		if (-1 == *token) {
			*token = CTL_Z;
		}
#endif
	}

    } while (CTL_Z != *token);

    // set image_mode according to image_type (image_mode
    // is used in transform to determine how to convert a short
    // to a height value
    if (!strncasecmp(image_type, "Height", strlen("Height")))
	image_mode = NS_HEIGHT;
    else if (!strncasecmp(image_type, "Current", strlen("Current")))
	image_mode = NS_CURRENT;
    else if (!strncasecmp(image_type, "Deflection", strlen("Deflection")))
	image_mode = NS_DEFLECTION;
    else if (!strncasecmp(image_type, "AUX", strlen("AUX")))
	image_mode = NS_AUXC;	// XXX Assuming Aux C... could be others

    // We know the file size, set our own grid size.
    setGridSize(_num_x, _num_y);

    
    return 0;
    
} // parseNanoscopeFileHeader
*/

double
BCGrid::transform(short* datum, nmb_diImageInfo * file_info, int do_swap)
{
    char  *b1 = (char *) datum;
    char  *b0 = b1 + 1;

    // Swap the bytes in the datum if we need to. 
    if (do_swap) {
	SWAP(*b1, *b0);
    }


/*XXX
{ static int min = 0;
  static int max = 0;
  if (*datum < min) { min = *datum; printf("min %d, max %d\n",min,max); }
  if (*datum > max) { max = *datum; printf("min %d, max %d\n",min,max); }
}*/

    // Convert the raw data to its correct units.  See section A.2 in
    // version 2.5 of: Digital Instruments Nanoscope III Scanning Probe
    // Microscope Control System User's Manual (orange cover).  This
    // describes how to convert from raw values to units.

    switch (file_info->image_mode)
    {
      case NS_HEIGHT_V44: 
      case NS_HEIGHT_V41: 
          {
          //Very simple, from DI Nanoscope v 4.1 manual, appendix B
          double val = (*datum)*file_info->z_scale/65536.0;
          return val;

      }
      case NS_HEIGHT: {
	double normalized = (*datum)/65536.0;
	double volts = normalized * (2*file_info->z_max);
	double nm = volts * file_info->z_sensitivity;
	double attenuated_nm = nm * (file_info->attenuation_in_z / 65536.0);
	double scaled = attenuated_nm * (file_info->z_scale / 65536.0);

/*XXX
{ static float min = 0;
  static float max = 0;
  float value = scaled * transform_scale;
  if (value < min) { min = value; printf("minh %f, maxh %f\n",min,max); }
  if (value > max) { max = value; printf("minh %f, maxh %f\n",min,max); }
}*/

	return scaled;
      }

      case NS_DEFLECTION: {
	double normalized = (*datum)/65536.0;
	double scaled = normalized * (file_info->z_scale / 65536.0);
	double nm = scaled * (2*file_info->input_1_max) * file_info->input_sensitivity /
		    file_info->detection_sensitivity;

/*XXX
{ static float min = 0;
  static float max = 0;
  float value = nm * transform_scale;
  if (value < min) { min = value; printf("mind %f, maxf %d\n",min,max); }
  if (value > max) { max = value; printf("mind %f, maxf %d\n",min,max); }
}*/

	return nm ;
      }

      case NS_AUXC: {
	double normalized = (*datum)/65536.0;
	double scaled = normalized * (file_info->z_scale_auxc / 65536.0);
//XXX The DI seems to be off by about a factor of 8 from our equation.
// Dave thinks that the input sensitivity is really always 1, so I'll tak
// that out.
//	double volts = scaled * (2*file_info->input_2_max) * file_info->input_sensitivity;
	double volts = scaled * (2*file_info->input_2_max);

/*XXX
{ static float min = 0;
  static float max = 0;
  float value = volts * transform_scale;
  if (value < min) { min = value; printf("minv %f, maxv %f\n",min,max); }
  if (value > max) { max = value; printf("minv %f, maxv %f\n",min,max); }
}*/

	return volts ;
      }

      default :
	return -1;
    }

}

