/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmg_SurfaceMask.h"
#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <stdio.h>
#include <math.h>
#include <quat.h> // For Q_DEG_TO_RAD

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::Constructor
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
nmg_SurfaceMask::
nmg_SurfaceMask()
{
    d_maskData = (int*)NULL;

    d_height = 0;
    d_width = 0;
    
    d_centerX = 0;
    d_centerY = 0;
    d_boxWidth = 0;
    d_boxHeight = 0;
    d_boxAngle = 0;
    
    d_control = (BCPlane*)NULL;
    d_derivationMode = NONE;
    d_needsDerivation = false;
    d_drawPartialMask = false;
    d_oldDerivation = (nmg_SurfaceMask*)NULL;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::Destructor
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
nmg_SurfaceMask::
~nmg_SurfaceMask()
{
    delete [] d_maskData;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::init
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
init(int width, int height)
{
    if (width != d_width || height != d_height) {  
        int x, y;
        
        if (d_maskData) {
            delete [] d_maskData;
        }

        d_maskData = new int[width * height];

        for(y = 0; y < height; y++) {
            for(x = 0; x < width; x++) {
                d_maskData[x+y*width] = 0;
            }
        }

        d_width = width;
        d_height = height;

        if (!d_oldDerivation) {
            d_oldDerivation = new nmg_SurfaceMask;
        }
        //Can't manually call the init function for d_oldDerivation
        //or we will get infinite recursion
        if (d_oldDerivation->d_maskData) {
            delete [] d_oldDerivation->d_maskData;
        }

        d_oldDerivation->d_width = width;
        d_oldDerivation->d_height = height;
        d_oldDerivation->d_maskData = new int[width * height];

        for(y = 0; y < height; y++) {
            for(x = 0; x < width; x++) {
                d_oldDerivation->d_maskData[x+y*width] = 0;
            }
        }
    }    
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::setControlPlane
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
setControlPlane(BCPlane *control)
{
    d_control = control;
    if (d_control == (BCPlane*)NULL) {
        d_derivationMode = NONE;
        for(int y = 0; y < d_height; y++) {
            for(int x = 0; x < d_width; x++) {
                if (value(x,y) > 0) {
                    addValue(x,y,-1);
                }                
            }            
        }
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::invertAdd
//      Access: Public
// Description: This sets as masked in this mask, the unmasked
//              portions of the other mask
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
invertAdd(nmg_SurfaceMask *other)
{
    if (d_height != other->d_height || d_width != other->d_width) {
        printf("nmg_SurfaceMask::invertAdd\tSize mismatch!\n");
    }

    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            if (!other->value(x,y)) {
                addValue(x,y,1);
            }
        }
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::invertSubtract
//      Access: Public
// Description: This sets as unmasked in this mask, the unmasked
//              portions of the other mask
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
invertSubtract(nmg_SurfaceMask *other)
{
    if (d_height != other->d_height || d_width != other->d_width) {
        printf("nmg_SurfaceMask::invertSubtract\tSize mismatch!\n");
    }

    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            if (!other->value(x,y)) {
                if (value(x,y) > 0) {
                    addValue(x,y,-1);
                }                
            }
        }
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::subtract
//      Access: Public
// Description: This sets as unmasked in this mask, the masked
//              portions of the other mask
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
subtract(nmg_SurfaceMask *other)
{
    if (d_height != other->d_height || d_width != other->d_width) {
        printf("nmg_SurfaceMask::subtract\tSize mismatch!\n");
    }

    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            if (other->value(x,y)) {
                if (value(x,y) > 0) {
                    addValue(x,y,-1);
                }                
            }
        }
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::add
//      Access: Public
// Description: This sets as masked in this mask, the masked
//              portions of the other mask
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
add(nmg_SurfaceMask *other)
{
    if (d_height != other->d_height || d_width != other->d_width) {
        printf("nmg_SurfaceMask::add\tSize mismatch!\n");
    }

    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            if (other->value(x,y)) {
                addValue(x,y,1);
            }
        }
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::clear
//      Access: Public
// Description: Resets to an empty mask
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
clear()
{
    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            addValue(x,y,-value(x,y));
        }
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::deriveMask
//      Access: Public
// Description: Create a masking plane, using a range of
//              height values
////////////////////////////////////////////////////////////
int nmg_SurfaceMask::
deriveMask(float min_height, float max_height)
{
    if (d_minHeight != min_height ||
        d_maxHeight != max_height) 
    {
        d_minHeight = min_height;
        d_maxHeight = max_height;
        d_derivationMode = HEIGHT;

        d_needsDerivation = true;
        return 1;
    }
    return 0;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::deriveHeight
//      Access: Private
// Description: Create a masking plane, using a range of
//              height values
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
deriveHeight()
{   
    if (d_control == (BCPlane*)NULL) {
		//If there is no control plane, then bail		
		return;
	}

    subtract(d_oldDerivation);
    d_oldDerivation->clear();

    float z;
    int maskVal;
	for(int y = 0; y < d_control->numY(); y++) {
        for(int x = 0; x < d_control->numX(); x++) {
            z = d_control->value(x,y);
            maskVal = ((z < d_minHeight) || (z > d_maxHeight));
            d_oldDerivation->addValue(x, y, maskVal);
        }
    }

    
    add(d_oldDerivation);
}

////////////////////////////////////////////////////////////
//    Function: xform_width && xform_height
//      Access: Public
// Description: Helper function for deriveBox that together
//              return if the point is inside the rotated box
////////////////////////////////////////////////////////////
static float xform_width(float x, float y, float angle) {
    return fabs(cos(angle)*(x) + sin(angle)*(y));
}
static float xform_height(float x, float y, float angle) {
    return fabs(cos(angle)*(y) - sin(angle)*(x));
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::deriveMask
//      Access: Public
// Description: Create a masking plane, using a rotated box based
//              method
////////////////////////////////////////////////////////////
int nmg_SurfaceMask::
deriveMask(float center_x, float center_y, float width,float height, 
           float angle)
{
    if (d_centerX != center_x || d_centerY != center_y ||
        d_boxWidth != width || d_boxHeight != height ||
        d_boxAngle != angle) 
    {   
        d_derivationMode = BOX;
        d_centerX = center_x;
        d_centerY = center_y;
        d_boxWidth = width;
        d_boxHeight = height;
        d_boxAngle = angle;

        d_needsDerivation = true;
        return 1;
    }
    return 0;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::deriveMask
//      Access: Public
// Description: Create a masking plane, using a rotated box based
//              method
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
deriveBox(nmb_Dataset *dataset)
{
    subtract(d_oldDerivation);
    d_oldDerivation->clear();

    double w_x, w_y;

    for(int y = 0; y < dataset->inputGrid->numY(); y++) {
        for(int x = 0; x < dataset->inputGrid->numX(); x++) {
            //Transform row,col in grid into world coordinates
            dataset->inputGrid->gridToWorld(x,y,w_x,w_y);
            // Find out if that coord is within width/height of 
            // region box. Helper functions handle rotation.
            if ((xform_width(w_x-d_centerX, w_y-d_centerY, 
                             Q_DEG_TO_RAD(d_boxAngle)) < d_boxWidth)&&
                (xform_height(w_x-d_centerX, w_y-d_centerY, 
                              Q_DEG_TO_RAD(d_boxAngle)) < d_boxHeight)){
                d_oldDerivation->addValue(x,y,1);
            }                    
        }
    }
    add(d_oldDerivation);
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::rederive
//      Access: Public
// Description: Check the plane derivation method that is being
//              used and rederive if necessary.
////////////////////////////////////////////////////////////
int nmg_SurfaceMask::
rederive(nmb_Dataset *dataset)
{
    int derived = 0;
    switch(d_derivationMode) {
    case HEIGHT:
        deriveHeight();
        derived = 1;
        break;
    case BOX:
        if (d_needsDerivation) {
            deriveBox(dataset);
            derived = 1;
        }
        break;
    default:
        break;
    }

    d_needsDerivation = false;
    return derived;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::needsDerivation
//      Access: Public
// Description: Check the plane derivation method that is being
//              used and return whether derivation is needed
////////////////////////////////////////////////////////////
int nmg_SurfaceMask::
needsDerivation()
{
    switch(d_derivationMode) {
    case HEIGHT:
        return 1;
    case BOX:
        return d_needsDerivation;
    default:
        return 0;
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::setDrawPartialMask
//      Access: Public
// Description: Set whether this SurfaceMask is to consider
//              partially masked quads as unmasked or not.
//              The default is for it to consider them masked.
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
setDrawPartialMask(vrpn_bool draw)
{
    d_drawPartialMask = draw;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::quadMasked
//      Access: Public
// Description: Tell whether the quad (with the points given
//              as the upper left of the quad) is to be
//              considered masked or not
////////////////////////////////////////////////////////////
bool nmg_SurfaceMask::
quadMasked(int x, int y, int stride)
{
    int index = x + y * d_width;
    int step = stride*d_width;
    if (d_drawPartialMask) {
        return (d_maskData[index] > 0 ||
                d_maskData[index-step] > 0 ||
                d_maskData[index+stride] > 0 ||
                d_maskData[index+stride-step]> 0 );
    }
    else {
        return (d_maskData[index] > 0 &&
                d_maskData[index-step] > 0 &&
                d_maskData[index+stride] > 0 &&
                d_maskData[index+stride-step]> 0 );
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::completeImage
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
bool nmg_SurfaceMask::
completeImage(nmg_SurfaceMask *other)
{
    if (d_height != other->d_height || d_width != other->d_width) {
        printf("nmg_SurfaceMask::completeImage\tSize mismatch!\n");
    }

    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            int v1 = value(x,y);
            int v2 = other->value(x,y);
            if (v1 && v2) {
                return false;
            }
        }
    }

    return true;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::numberOfHoles
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
int nmg_SurfaceMask::
numberOfHoles()
{
    int count = 0;

    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            if (value(x,y)) {
                count++;
            }
        }
    }

    return count;
}
