#include "nmg_SurfaceMask.h"
#include <BCPlane.h>
#include <stdio.h>

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
    d_control = (BCPlane*)NULL;
    d_derivationMode = NONE;
    d_drawPartialMask = false;
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
    
    rederive(true);
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::remove
//      Access: Public
// Description: This sets as masked in this mask, the unmasked
//              portions of the other mask
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
remove(nmg_SurfaceMask *other)
{
    if (d_height != other->d_height || d_width != other->d_width) {
        printf("nmg_SurfaceMask::remove\tSize mismatch!\n");
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
//    Function: nmg_SurfaceMask::add
//      Access: Public
// Description: This sets as unmasked in this mask, the unmasked
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
            if (!other->value(x,y)) {
                if (value(x,y) > 0) {
                    addValue(x,y,-1);
                }                
            }
        }
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::deriveMask
//      Access: Public
// Description: Create a masking plane, using a range of
//              height values
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
deriveMask(float min_height, float max_height)
{
    d_minHeight = min_height;
    d_maxHeight = max_height;
    d_derivationMode = HEIGHT;

    if (d_control == (BCPlane*)NULL) {
		//If there is no control plane, then bail		
		return;
	}

    float z;
    int maskVal;
	for(int y = 0; y < d_control->numY(); y++) {
        for(int x = 0; x < d_control->numX(); x++) {
            if (value(x,y) > 0) {
                addValue(x, y, -1);
            }
            z = d_control->value(x,y);
            maskVal = ((z < min_height) || (z > max_height));
            addValue(x, y, maskVal);
        }
    }
}


////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceMask::rederive
//      Access: Public
// Description: Check the plane derivation method that is being
//              used and rederive if necessary.
////////////////////////////////////////////////////////////
void nmg_SurfaceMask::
rederive(vrpn_bool force)
{
    switch(d_derivationMode) {
    case HEIGHT:
        deriveMask(d_minHeight, d_maxHeight);
        break;
    default:
        break;
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
                d_maskData[index+1] > 0 ||
                d_maskData[index+1-step]);
    }
    else {
        return (d_maskData[index] > 0 &&
                d_maskData[index-step] > 0 &&
                d_maskData[index+1] > 0 &&
                d_maskData[index+1-step]);
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
