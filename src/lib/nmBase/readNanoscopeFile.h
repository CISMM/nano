/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef READNANOSCOPEFILE_H
#define READNANOSCOPEFILE_H

// Stuff for reading DI Nanoscope files
#define	NS_HEIGHT			(0)
#define NS_CURRENT			(1)
#define NS_DEFLECTION			(2)
#define NS_AUXC				(3)
#define	NS_HEIGHT_V41			(4)
#define	NS_HEIGHT_V43			(5)
//#define NS_DO_SWAP			(10)

class nmb_diImageInfo {
public:
    double scan_size; ///< Scan size for square, or x scan size
    double scan_size_y;  ///< y scan size, if needed. 
    char scan_units[10]; ///< units of scan, usually "nm"
    int image_mode; ///< One of the NS_* variables above, inferred from header
    char image_data_type[50]; ///< Text description of data, trans to units?
    int data_offset;
    double z_scale;
    double soft_z_scale; ///< v4.3+ only
    char z_units[10];
    int num_x;  ///< number of data points in x
    int num_y;  ///< number of data points in y
    double z_scale_auxc;
    double attenuation_in_z;
    double detection_sensitivity;
    double z_sensitivity;
    double input_sensitivity;
    double z_max;
    double input_1_max;
    double input_2_max;

    nmb_diImageInfo * next;

    nmb_diImageInfo () :
        scan_size(-1),
        image_mode(NS_HEIGHT),
        data_offset(8192),
        z_scale(-1),
        soft_z_scale(1),
        num_x(0),
        num_y(0),
        z_scale_auxc(1),
        attenuation_in_z(65536),
        detection_sensitivity(0.04),
        z_sensitivity(8.1),
        input_sensitivity(0.125),
        z_max(220),
        input_1_max(10),
        input_2_max(10),
        next(NULL) 
    { 
        scan_units[0] = '\0';
        z_units[0] = '\0';
    };
};

#endif
