/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef READHAMBURGFILE_H
#define READHAMBURGFILE_H

// Possible image modes (data types in the file)
#define	HH_HEIGHT   (0)
#define	HH_CURRENT  (1)
#define HH_DI_DV    (2)

static double HH_HVGAIN = 20.0;	///< High-voltage gain used in the file

class nmb_hhImageInfo {
public:
    double scan_size; ///< Scan size (all scans are square)
    double x_offset;  ///< Offset from scanner center to actual scan center
    double y_offset;  ///< Offset from scanner center to actual scan center
    int num_x;  ///< number of data points in x
    int num_y;  ///< number of data points in y
    double z_scale; ///< Z Sensitivity parameter (nm per Volt)
    int data_offset;  ///< Where the data starts in the file
    int data_length;  ///< Length of the data in the file
    char scan_units[10]; ///< units of scan, usually "nm"
    int image_mode; ///< One of the NS_* variables above, inferred from header

    char image_data_type[50]; ///< Text description of data, trans to units?

    nmb_hhImageInfo () :
        scan_size(-1),
        image_mode(-1),
        data_offset(-1),
        z_scale(-1),
        num_x(0),
        num_y(0),
	x_offset(0),
	y_offset(0)
    { 
        scan_units[0] = '\0';
    };
};

#endif
