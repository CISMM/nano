/*===3rdtech===
  Copyright (c) 2003 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef READNANOTECFILE_H
#define READNANOTECFILE_H

/// Info contained in a Nanotec Electronica ".stp" file
class nmb_NanotecImageInfo {
public:
   // Straight from the WSxM header routines. 
	char **tszTitles;
	char **tszLabels;
	char **tszValues;

	int iNumFields;

	int bBinary;      // header can be binary or ASCII

   // Need some units, too. 
    double x_amplitude; ///< Scan size for square, or x scan size
    double y_amplitude;  ///< y scan size, if needed. 
    char scan_units[10]; ///< units of scan, usually "nm"
    double z_amplitude;  ///< z scan range
    char z_units[10]; ///< units of height, usually "nm"
    nmb_NanotecImageInfo () :
        x_amplitude(100),
        y_amplitude(100),
        z_amplitude(100)
    { 
        scan_units[0] = '\0';
        z_units[0] = '\0';
    };
   /* Don't need all of this, but don't know what I need yet. 
    int data_offset; ///< How big is the header?
    // Control section
    double signal_gain;
    double z_gain;
    // General Info section
    char head_type[50]; ///< Type of head, STM, AFM, NSOM, etc
    int num_x;  ///< number of data points in x
    int num_y;  ///< number of data points in y

    // Head settings section
    double preamp_gain;
    double x_calibration;
    double y_calibration;
    
    nmb_NanotecImageInfo * next;

    nmb_NanotecImageInfo () :
        data_offset(0),
        signal_gain(1),
        x_amplitude(100),
        y_amplitude(100),
        z_gain(1),
        num_x(0),
        num_y(0),
        z_amplitude(100),
        preamp_gain(1000),
        x_calibration(1),
        y_calibration(1),
        next(NULL) 
    { 
        scan_units[0] = '\0';
        z_units[0] = '\0';
        head_type[0] = '\0';
    };
    */
};

#define HEADER nmb_NanotecImageInfo

#endif
