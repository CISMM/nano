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

/**
 
    Access: Public
*/
nmg_SurfaceMask::
nmg_SurfaceMask()
{
    d_maskData = NULL;

    d_height = 0;
    d_width = 0;
    
    // -1 so different from default in interaction.c
    d_centerX = -1;
    d_centerY = -1;
    d_boxWidth = 0;
    d_boxHeight = 0;
    d_boxAngle = 0;
    
    d_minValid = -1;
    d_maxValid = -1;

    d_control = (BCPlane*)NULL;
    d_derivationMode = NONE;
    d_needsUpdate = false;
    d_drawPartialMask = false;
    d_oldDerivation = (nmg_SurfaceMask*)NULL;

    d_numInvertMasks = 0;

}

/**
 
    Access: Public
*/
nmg_SurfaceMask::
~nmg_SurfaceMask()
{
    delete [] d_maskData;
}

/**
 
 Access: Public
 @return -1 on memory error, 0 on success. 
*/
int nmg_SurfaceMask::
init(int width, int height)
{
    if (width != d_width || height != d_height) {  
        int x, y;
        
        if (d_maskData) {
            delete [] d_maskData;
        }

        d_maskData = new char[width * height];
        if (!d_maskData) return -1;
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
        if (!d_oldDerivation) return -1;
        //Can't manually call the init function for d_oldDerivation
        //or we will get infinite recursion
        if (d_oldDerivation->d_maskData) {
            delete [] d_oldDerivation->d_maskData;
        }

        d_oldDerivation->d_width = width;
        d_oldDerivation->d_height = height;
        d_oldDerivation->d_maskData = new char[width * height];
        if (!d_oldDerivation->d_maskData) return -1;
        for(y = 0; y < height; y++) {
            for(x = 0; x < width; x++) {
                d_oldDerivation->d_maskData[x+y*width] = 0;
            }
        }
    }
    return 0;
}

/**
 Helper function for deriveBox that together
            return if the point is inside the rotated box
*/
static float xform_width(float x, float y, float angle) {
    return fabs(cos(angle)*(x) + sin(angle)*(y));
}
static float xform_height(float x, float y, float angle) {
    return fabs(cos(angle)*(y) - sin(angle)*(x));
}

/** Is this pixel masked? */
int nmg_SurfaceMask::value(int x, int y) 
{
    switch(d_derivationMode) {
    case HEIGHT:
        return d_maskData[x + y * d_width];
    case BOX:
        {
            double w_x, w_y;
            //Transform row,col in grid into world coordinates
            d_dataset->inputGrid->gridToWorld(x,y,w_x,w_y);
            // Find out if that coord is within width/height of 
            // region box. Helper functions handle rotation.
            // What a simple, slow method. Should rasterize!
            if ((xform_width(w_x-d_centerX, w_y-d_centerY, 
                             Q_DEG_TO_RAD(d_boxAngle)) < d_boxWidth)&&
                (xform_height(w_x-d_centerX, w_y-d_centerY, 
                              Q_DEG_TO_RAD(d_boxAngle)) < d_boxHeight)){
                return 1;
                //d_empty = false;
            } else {
                return 0;
            }
        }
    case NULLDATA:
        {
            // If y coord is between our bounds,pixel is masked. 
            if ( y>= d_minValid && y<=d_maxValid ) {
                return 1;
            } else {
                return 0;
            }
        }
    case INVERTMASKS:
        {
            int ret = 1;
            // if pixel is masked in all other masks, we're not masked. 
            for (int i = 0; i < d_numInvertMasks; i++) {
                ret &= d_invertMaskList[i]->value(x,y);
            }
            return !ret;
        }
    default:
        return 0;
    }
}

/**
 
    Access: Public
*/
void nmg_SurfaceMask::
setControlPlane(BCPlane *control)
{
    d_control = control;
    if (d_control == (BCPlane*)NULL) {
        d_derivationMode = NONE;
        for(int y = 0; y < d_height; y++) {
            for(int x = 0; x < d_width; x++) {
                if (value(x,y) > 0) {
                    setValue(x,y,0);
                }                
            }            
        }
    }
}

/**
 This sets as masked in this mask, the unmasked
            portions of the other mask
@Note Obsolete?
*/
void nmg_SurfaceMask::
invertAdd(nmg_SurfaceMask *other)
{
    if (d_height != other->d_height || d_width != other->d_width) {
        printf("nmg_SurfaceMask::invertAdd\tSize mismatch!\n");
    }

    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            if (!other->value(x,y)) {
                setValue(x,y,1);
            }
        }
    }
}

/**
 This sets as unmasked in this mask, the unmasked
            portions of the other mask
@Note Obsolete?
*/
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
                    setValue(x,y,0);
                }                
            }
        }
    }
}

/**
 This sets as unmasked in this mask, the masked
            portions of the other mask
@Note Obsolete?
*/
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
                    setValue(x,y,0);
                }                
            }
        }
    }
}

/**
 This sets as masked in this mask, the masked
            portions of the other mask
@Note Obsolete?
*/
void nmg_SurfaceMask::
add(nmg_SurfaceMask *other)
{
    if (d_height != other->d_height || d_width != other->d_width) {
        printf("nmg_SurfaceMask::add\tSize mismatch!\n");
    }

    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            if (other->value(x,y)) {
                setValue(x,y,1);
            }
        }
    }
}

/**
 Resets to an empty mask
*/
void nmg_SurfaceMask::
clearOff()
{
    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            setValue(x,y,0);
        }
    }
}

/**
 Resets to a completely on mask
*/
void nmg_SurfaceMask::
clearOn()
{
    for(int y = 0; y < d_height; y++) {
        for(int x = 0; x < d_width; x++) {
            setValue(x,y,1);
        }
    }
}

/**
 Create a masking plane, using a range of
            height values
@return 1 if mask method or parameters changed since last time
@return 0 otherwise. 
    Access: Public
*/
int nmg_SurfaceMask::
deriveMask(float min_height, float max_height)
{
    if (d_derivationMode != HEIGHT ||
        d_minHeight != min_height ||
        d_maxHeight != max_height) 
    {
        d_derivationMode = HEIGHT;
        d_minHeight = min_height;
        d_maxHeight = max_height;

        d_needsUpdate = true;

        // Make the mask data correct. 
        //clearOn();
        if (d_control == NULL) {
            //If there is no control plane, then bail		
            return 1;
        }
        float z;
        int maskVal;
	for(int y = 0; y < d_control->numY(); y++) {
            for(int x = 0; x < d_control->numX(); x++) {
            z = d_control->value(x,y);
            maskVal = ((z < d_minHeight) || (z > d_maxHeight));
            setValue(x, y, maskVal);
        }
    }
        
        return 1;
    }
    return 0;
}

/**
 Create a masking plane, using a range of
            height values
    Access: Private
 @return 1 if whole region needs rebuilt, 0 if only strips between
 low_row and high_row needs rebuilt. 
*/
int nmg_SurfaceMask::
deriveHeight(int low_row, int high_row, int strips_in_x)
{   
    if (d_control == NULL) {
        //If there is no control plane, then bail		
        return 0;
    }

    float z;
    int maskVal;
    if (strips_in_x) {
        for(int y = low_row; y <= high_row; y++) {
            for(int x = 0; x < d_control->numX(); x++) {
                z = d_control->value(x,y);
                maskVal = ((z < d_minHeight) || (z > d_maxHeight));
                setValue(x, y, maskVal);
            }
        }
    } else {
	for(int y = 0; y < d_control->numY(); y++) {
            for(int x = low_row; x <= high_row; x++) {
                z = d_control->value(x,y);
                maskVal = ((z < d_minHeight) || (z > d_maxHeight));
                setValue(x, y, maskVal);
            }
        }
    }
    // didn't affect any values outside low_row and high_row,
    // but might have rederived whole plane above. 
    return d_needsUpdate;
}


/**
 Create a masking plane, using a rotated box based
            method
@return 1 if mask method or parameters changed since last time
@return 0 otherwise. 
    Access: Public
*/
int nmg_SurfaceMask::
deriveMask(float center_x, float center_y, float width,float height, 
           float angle)
{
    if (d_derivationMode != BOX ||
        d_centerX != center_x || d_centerY != center_y ||
        d_boxWidth != width || d_boxHeight != height ||
        d_boxAngle != angle) 
    {
        d_derivationMode = BOX;
        d_centerX = center_x;
        d_centerY = center_y;
        d_boxWidth = width;
        d_boxHeight = height;
        d_boxAngle = angle;

        d_needsUpdate = true;
        return 1;
    }
    return 0;
}

/**
 Create a masking plane, using a rotated box based
  method. Merely records which dataset to use when
  value() is called. 
    Access: Public
 @return 1 if whole region needs rebuilt, 0 if only strips between
 low_row and high_row needs rebuilt. 
*/
int nmg_SurfaceMask::
deriveBox(nmb_Dataset *dataset, int low_row, int high_row, int strips_in_x)
{
    d_dataset = dataset;
    return d_needsUpdate;
    /*
    subtract(d_oldDerivation);
    d_oldDerivation->clear();

    double w_x, w_y;
    //d_empty = true;
    for(int y = 0; y < dataset->inputGrid->numY(); y++) {
        for(int x = 0; x < dataset->inputGrid->numX(); x++) {
            //Transform row,col in grid into world coordinates
            dataset->inputGrid->gridToWorld(x,y,w_x,w_y);
            // Find out if that coord is within width/height of 
            // region box. Helper functions handle rotation.
            // What a simple, slow method. Should rasterize!
            if ((xform_width(w_x-d_centerX, w_y-d_centerY, 
                             Q_DEG_TO_RAD(d_boxAngle)) < d_boxWidth)&&
                (xform_height(w_x-d_centerX, w_y-d_centerY, 
                              Q_DEG_TO_RAD(d_boxAngle)) < d_boxHeight)){
                d_oldDerivation->addValue(x,y,1);
                //d_empty = false;
            }                    
        }
    }
    add(d_oldDerivation);
    */
}

/**
 Create a masking plane, using the invalid data in the control plane
    Access: Public
@return 1 if mask method or parameters changed since last time
@return 0 otherwise. 
*/
int nmg_SurfaceMask::
deriveMask()
{
    d_needsUpdate = true;
    if (d_derivationMode != NULLDATA) {
        d_derivationMode = NULLDATA;
        return 1;
    }

    // Look at that. We don't need to do anything here, it's all in rederive
    return 0;
}
/*
static void deriveNullDataLoop(nmg_SurfaceMask * sm, 
                               short ymin, short ymax, short numx, 
                               short top, short left, 
                               short bottom, short right) {
    short y;
    for(y = ymin; y <= ymax; y++) {
        for(short x = 0; x < numx; x++) {
            if ( (y <= top) && (y >=bottom) &&
                 (x <= right) && (x >= left)) {
                // Inside valid region.
                if (!sm->value(x,y)) sm->addValue(x,y,1);
            } else {
                if (sm->value(x,y)) sm->addValue(x,y,-sm->value(x,y));
            }
        }
    }
}
*/
/**
 Create a masking plane, using invalid data in the control plane
    Access: Private
 @return 1 if whole region needs rebuilt, 0 if only strips between
 low_row and high_row needs rebuilt. 
*/
int nmg_SurfaceMask::
deriveNullData(int stride, int low_row, int high_row, int strips_in_x)
{
    if (d_control == (BCPlane*)NULL) {
        //If there is no control plane, then bail	
	fprintf(stderr, "nmg_SurfaceMask::deriveNullData:No control plane!\n");
        return 0;
    }

    // XXX Should do more intelligent checking to see if
    // valid data range and low_row, high_row correspond. 
    // For now assume we never need to force extra rebuilds. 
    short top, left, bottom, right;
    if (d_control->findValidDataRange(&left,&right,&bottom,&top)) {
        if ( d_minValid != -1 || d_maxValid !=-1) {
            // There is no valid data. Unmask whole plane
            d_minValid = -1; 
            d_maxValid = -1;
        }
    } else {
        // Here we need to compare the data range with
        // the one just reported, and make the minimal changes. 

        // Expand "bottom" and "top" so we get connection with one null row,
        // make it prettier to be connected to 0 plane with fence. 
        if (strips_in_x) {
            bottom = max (0, bottom-stride);
            top = min(d_control->numY()-1, top+stride);
            d_minValid = bottom; 
            d_maxValid = top;
        } else {
            left = max (0, left-stride);
            right = min(d_control->numX()-1, right+stride);
            d_minValid = left; 
            d_maxValid = right;
        }
    }
    return d_needsUpdate;
}

/**
 Create a masking plane, using the inverse of other masks. 
    Access: Public
@return 1 if mask method or parameters changed since last time
@return 0 otherwise. 
*/
int nmg_SurfaceMask::
deriveMask(nmg_SurfaceMask** masks, int numInvertMasks)
{
    d_needsUpdate = true;
    for (int i = 0; i < numInvertMasks; i++) {
        d_invertMaskList[i] = masks[i];       
    }
    d_numInvertMasks = numInvertMasks;

    if (d_derivationMode != INVERTMASKS) {
        d_derivationMode = INVERTMASKS;
        return 1;
    }
    return 0;
}

/**
 Create a masking plane, using the inverse of other masks.
    Access: Private
 @return 1 if whole region needs rebuilt, 0 if only strips between
 low_row and high_row needs rebuilt. 
*/
int nmg_SurfaceMask::
deriveInvertMasks(nmb_Dataset *dataset, int stride, int low_row, int high_row, int strips_in_x)
{
    int ret = d_needsUpdate;
    // We need total rebuild if our params have changed or any of our
    // dependent masks need rebuilt.
    for (int i = 0; i < d_numInvertMasks; i++) {
        ret |= d_invertMaskList[i]->update(dataset, stride, low_row, high_row, strips_in_x, false);
    }
    return ret;
}

/**
 Check the plane derivation method that is being
 used and rederive if necessary.
 Called once per frame for each mask!
    Access: Public
 @return 1 if whole region needs rebuilt, 0 if only strips between
 low_row and high_row needs rebuilt. 
*/
int nmg_SurfaceMask::
update(nmb_Dataset *dataset, int stride, 
       int low_row, int high_row, int strips_in_x, bool clear_needs_update)
{
    int derived = 0;
    //if (d_needsUpdate) {
        switch(d_derivationMode) {
        case HEIGHT:
            derived = deriveHeight(low_row, high_row, strips_in_x);
            break;
        case BOX:
            derived = deriveBox(dataset, low_row, high_row, strips_in_x);
            break;
        case NULLDATA:
            derived = deriveNullData(stride, low_row, high_row, strips_in_x);
            break;
        case INVERTMASKS:
            derived = deriveInvertMasks(dataset, stride, low_row, high_row, strips_in_x);
            break;
        default:
            break;
        }
        //}
        if (clear_needs_update) {
            d_needsUpdate = false;
        }
    return derived;
}

/**
   Has the mask changed since the last time we called
nmg_SurfaceMask::rederive?
    Access: Public
*/
bool nmg_SurfaceMask::
needsUpdate()
{
    return d_needsUpdate;
}

/**
 Set whether this SurfaceMask is to consider
            partially masked quads as unmasked or not.
            The default is for it to consider them masked.
    Access: Public
*/
void nmg_SurfaceMask::
setDrawPartialMask(vrpn_bool draw)
{
    d_drawPartialMask = draw;
}

/**
 Tell whether the quad (with the points given
            as the upper left of the quad) is to be
            considered masked or not
    Access: Public
*/
bool nmg_SurfaceMask::
quadMasked(int x, int y, int stride)
{
    // Special case - no quads are ever drawn for NULLDATA region.
    if (d_derivationMode == NULLDATA) return true;
    // DEBUG
    //if (d_derivationMode == BOX) return true;

    //int index = x + y * d_width;
    //int step = stride*d_width;
    if (d_drawPartialMask) {
        return (value(x,y) > 0 ||
                value(x+stride,y) > 0 ||
                value(x,y-stride) > 0 ||
                value(x+stride,y-stride) > 0 );
    }
    else {
        return (value(x,y) > 0 &&
                value(x+stride,y) > 0 &&
                value(x,y-stride) > 0 &&
                value(x+stride,y-stride) > 0 );
    }
}

/**
 
    Access: Public
*/
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

/**
 
    Access: Public
*/
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
