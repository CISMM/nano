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
#include "readHamburgFile.h"

/* local defines
**/
#define CTL_Z	(26)
#define BSLASH	'\\'
#define RET	(15)
#define NL	'\n'

#define SWAP(a,b)       {((a)^=(b));((b)^=(a));((a)^=(b));}

/**
   Parse a Hamburg header that has been read in from disk to a
   large string.  Helper function for BCGrid::parseHamburgFileHeader.
*/
int parse (nmb_hhImageInfo * file_info, char * header_text)
{
    int	ngot, total_params = 0;
    char * ptoken;
    char s_scrap[512];

    ptoken = strtok(header_text, "\n");
    while ( (NULL != ptoken ) && (CTL_Z != *ptoken) ) {
        // Here's the list of params we recognize, and they should all be
        // present for each image layer, we think. 
        if (!strncasecmp(ptoken, "\\Data offset:", 
                         strlen("\\Data offset:"))) {
            ngot = sscanf(ptoken + strlen("\\Data offset: "),"%d", 
                   &file_info->data_offset);
	    total_params++;
	    //printf("XXX found Data offset %d\n", file_info->data_offset);
        } else if (!strncasecmp(ptoken, "\\Data length:", 
                         strlen("\\Data length:"))) {
            ngot = sscanf(ptoken + strlen("\\Data length: "),"%d", 
                   &file_info->data_length);
	    total_params++;
	    //printf("XXX found Data length %d\n", file_info->data_length);
        } else if (!strncasecmp(ptoken, "\\X offset:", 
                                strlen("\\X offset:"))) {
            ngot = sscanf(ptoken + strlen("\\X offset: "),"%f %s", 
                   &file_info->x_offset, s_scrap);
	    if (strncasecmp(s_scrap, "nm", strlen("nm"))) {
	      fprintf(stderr, "readHamburgFile::parse(): X offset not in nm (%s)\n",
		s_scrap);
	      return -1;
	    }
	    total_params++;
	    //printf("XXX found X offset\n");
        } else if (!strncasecmp(ptoken, "\\Y offset:", 
                                strlen("\\Y offset:"))) {
            ngot = sscanf(ptoken + strlen("\\Y offset: "),"%f %s", 
                   &file_info->y_offset, s_scrap);
	    if (strncasecmp(s_scrap, "nm", strlen("nm"))) {
	      fprintf(stderr, "readHamburgFile::parse(): Y offset not in nm (%s)\n",
		s_scrap);
	      return -1;
	    }
	    total_params++;
	    //printf("XXX found Y offset\n");
        } else if (!strncasecmp(ptoken, "\\Samps/line:", 
                                strlen("\\Samps/line:"))) {
            ngot = sscanf(ptoken + strlen("\\Samps/line: "),"%d %d", 
                   &file_info->num_x, &file_info->num_y);
	    if (file_info->num_x != file_info->num_y) {
	      fprintf(stderr, "readHamburgFile::parse(): X and Y counts differ (%d, %d)\n",
		file_info->num_x, file_info->num_y);
	      return -1;
	    }
	    total_params++;
	    //printf("XXX found Samps/line %d %d\n", file_info->num_x, file_info->num_y);
        } else if (!strncasecmp(ptoken, "\\Scan size:", 
                                strlen("\\Scan size:"))) {
            ngot = sscanf(ptoken + strlen("\\Scan size: "),"%lg %s", 
                   &file_info->scan_size, file_info->scan_units);
	    if (strncasecmp(file_info->scan_units, "nm", strlen("nm"))) {
	      fprintf(stderr, "readHamburgFile::parse(): Scan units not in nm (%s)\n",
		file_info->scan_units);
	      return -1;
	    }
	    total_params++;
	    //printf("XXX found Scan size %g\n", file_info->scan_size);
        } else if (!strncasecmp(ptoken, "\\Z sensitivity:", 
                                strlen("\\Z sensitivity:"))) {
            ngot = sscanf(ptoken + strlen("\\Z sensitivity: "),"%lg", 
                   &file_info->z_scale);
	    total_params++;
	    //printf("XXX found Z sensitivity %f\n", file_info->z_scale);
        } else {
	  // Some paramter that we don't care about.
	  ngot = 1;
	}
        if (ngot <= 0) { 
            fprintf(stderr, "readHamburgFile::parse(): "
                    "Error reading header parameter!\n");
            return -1;
        }
        ptoken = strtok(NULL, "\n");
    }

    // Check to make sure we got everything we needed.
    if (file_info->scan_size <= 0) {
      fprintf(stderr,"readHamburgFile::parse(): Invalid or unspecified scan size (%g)\n", file_info->scan_size);
      return -1;
    }
    if (file_info->data_offset < 0) {
      fprintf(stderr,"readHamburgFile::parse(): Invalid or unspecified data offset (%d)\n", file_info->data_offset);
      return -1;
    }
    if (file_info->z_scale <= 0) {
      fprintf(stderr,"readHamburgFile::parse(): Invalid or unspecified Z scale (%g)\n", file_info->z_scale);
      return -1;
    }
    if ( (file_info->num_x <= 0) || (file_info->num_y <= 0) ) {
      fprintf(stderr,"readHamburgFile::parse(): Invalid or unspecified scan resolution (%d %d)\n", file_info->num_x, file_info->num_y);
      return -1;
    }

    return 0;
}

/**
parseHamburgFileHeader.  Helper function for readHamburgFile
   description: This method parses the headers of binary Hamburg
                files. In the process, it ignores information that is not
		needed (or understood).
        author: Russ Taylor 4/25/2004
 last modified: 
*/
int parseHamburgFileHeader(FILE* file, nmb_hhImageInfo * file_info)
{
    char token[BUFSIZ];
    
    // Get a line from the file. May be a first, partial line. 
    if (fgets(token, BUFSIZ, file)== NULL) { return -1; }
    // Scan the file until we get to the end or find length of header
    while ( (CTL_Z != *token ) &&
	    (strncasecmp(token, "\\Data offset", strlen("\\Data offset")) )) {
      if (fgets(token, BUFSIZ, file)== NULL) {
	fprintf(stderr, "BCGrid::parseHamburgFileHeader: Can't read header!\n");
	return -1;
      }
    }

    // Make sure we found a backslash; if not, no header
    if (CTL_Z == *token) {
	fprintf(stderr, "BCGrid::parseHamburgFileHeader: No header found!\n");
	return -1;
    }
    // Find header length
    if (sscanf(token+12, ": %d", &file_info->data_offset) != 1) {
	fprintf(stderr, "BCGrid::parseHamburgFileHeader: Can't read header size!\n");
	return -1;
    }
    // Read the whole header into one giant string, starting back at the
    // beginning.  We know how large it is because we just found out above.
    char *header_text = new char[file_info->data_offset +1];
    if (header_text == NULL) {
	fprintf(stderr, "BCGrid::parseHamburgFileHeader: Out of memory!\n");
	return -1;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
	fprintf(stderr, "BCGrid::parseHamburgFileHeader: Can't Seek back to start of file!\n");
	delete [] header_text;
	return -1;
    }

    if (fread(header_text, sizeof(char), file_info->data_offset, file) !=
	    (unsigned)(file_info->data_offset)) {
	fprintf(stderr, "BCGrid::parseHamburgFileHeader: Can't read header body\n");
	delete [] header_text;
	return -1;
    }

    int ret = parse(file_info, header_text);        
    delete [] header_text;
    return ret;
}

/** Read Hamburg SPM file. 
*/
int BCGrid::readHamburgFile(FILE* file, const char *filename)
{
    char *units_name;
    nmb_hhImageInfo file_info;
    if (parseHamburgFileHeader(file, &file_info)) {
	fprintf(stderr, "BCGrid::readHamburgFile: could not parse header!\n");
	return -1;
    }

    // move file pointer to first position in the data set
    if (fseek(file, file_info.data_offset, SEEK_SET )) {
	perror("BCGrid::readHamburgFile: could not move file pointer!");
	return -1;
    }

    // Compute the scan region in nanometers.
    // We rely on the input function having converted from any other units
    // into no when the header was read.
    _min_x = file_info.x_offset - file_info.scan_size / 2;
    _max_x = file_info.x_offset + file_info.scan_size / 2;
    _min_y = file_info.y_offset - file_info.scan_size / 2;
    _max_y = file_info.y_offset + file_info.scan_size / 2;

    // We know the file size, set our own grid size.
    // Planes added later adopt the new size.
    _num_x = file_info.num_x;
    _num_y = file_info.num_y;

    char name[255];
    BCPlane * plane;
    sprintf(name, "%s", filename);

    // Determine the type of data based on the last digit of
    // the file name.
    switch (filename[strlen(filename)-1]) {
      case '1': // Topography forwards
      case '5': // Topography backwards
	file_info.image_mode = HH_HEIGHT;
	units_name = "nm";
	break;
      case '2': // Current forwards
      case '6': // Current backwards
	file_info.image_mode = HH_CURRENT;
	units_name = "nA";
	break;
      case '3': // DI/DV forwards
      case '7': // DI/DV backwards
	file_info.image_mode = HH_DI_DV;
	units_name = "AU";
	break;
      default:
	units_name = "unknown";
	fprintf(stderr, "BCGrid::readHamburgFile: Could not interpret image type\n"
	  "  (based on last digit of file name: got %c)\n", filename[strlen(filename)-1]);
	return -1;
    }
    // First plane used to be timed, don't know if this matters. 
    plane = addNewPlane(name, units_name, NOT_TIMED);
    plane->readHamburgFile(file, &file_info);
    
    char scrap;
    
    if (fread(&scrap, 1, 1, file) == 1) {
	perror("BCGrid::readHamburgFile: WARNING: Not at file end upon completion!");
    }
    fclose(file);

    return 0;
    
} // readHamburgFile

double
BCGrid::transformHamburg(short* datum, nmb_hhImageInfo * file_info, int do_swap)
{
    char  *b1 = (char *) datum;
    char  *b0 = b1 + 1;

    // Swap the bytes in the datum if we need to. 
    if (do_swap) {
	SWAP(*b1, *b0);
    }

    /* Convert the raw data to its correct units.  The following description
       of how to do this is from an email from Christian Meyer at Hamburg:

	  From the raw data to the heigth data the formular is simple:
	  height_in_nm = HVgain * nm/V * raw_data/3276.8
	    with HVgain==20 and nm/V=\Z sensitivity from the header

	  Current data is:
	    current_in_nA = raw_data/3276.8

	  And dI/dV data is arb. units. (one can get simens out of these values
	  but for that one need the setup of the lockin (sensitivity/offset and
	  so on) which is written down on the measurement paper.
    */

    switch (file_info->image_mode)
    {
      case HH_HEIGHT: 
          {
          return (*datum) * HH_HVGAIN * file_info->z_scale / 3276.8;
          
          }
      case HH_CURRENT: 
          {
          return (*datum) / 3276.8;

          }
      case HH_DI_DV: 
          {
	  return (double)(*datum);
	  }

      default :
	return -1;
    }
}
