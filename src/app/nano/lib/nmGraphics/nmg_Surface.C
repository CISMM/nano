/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <stdlib.h>

#include <nmb_Dataset.h>
#include <BCPlane.h>

#include "nmg_Surface.h"
#include "nmg_SurfaceRegion.h"
#include "nmg_SurfaceMask.h"

/**
 
    Access: Public
*/
nmg_Surface::
nmg_Surface()
{
    d_numSubRegions = 0;
    d_maxNumRegions = 1;
    d_subRegions = (nmg_SurfaceRegion**)malloc(d_maxNumRegions * sizeof(nmg_SurfaceRegion**));
    d_defaultRegion = new nmg_SurfaceRegion(this, 0);
    d_dataset = (nmb_Dataset*)NULL;

    d_defaultRegion->getMaskPlane()->setDrawPartialMask(VRPN_TRUE);
}

/**
 
    Access: Public
*/
nmg_Surface::
~nmg_Surface()
{
    for(int i = 0; i < d_numSubRegions; i++) {
        delete d_subRegions[i];
    }
    free(d_subRegions);
    delete d_defaultRegion;
}

/**
 
    Access: Public
*/
int nmg_Surface::
init(unsigned int width, unsigned int height)
{
    d_initWidth = width;
    d_initHeight = height;
    if (!d_defaultRegion->init(d_initWidth, d_initHeight)) {
        return 0;
    }

    for(int i = 0; i < d_numSubRegions; i++) {
        if (!d_subRegions[i]->init(d_initWidth, d_initHeight)) {
            return 0;
        }
    }
    return 1;
}

/**
 
    Access: Public
*/
void nmg_Surface::
changeDataset(nmb_Dataset *dataset)
{
    d_dataset = dataset;

    //Get rid of all but the default region
    for(int i = 0; i < d_numSubRegions; i++) {
        delete d_subRegions[i];
    }
    d_numSubRegions = 0;
}

/**
 Returns the ID of the newly created region
    Access: Public
*/
int nmg_Surface::
createNewRegion()
{
    //I really wish we could use STL vectors....
    int region_id = d_numSubRegions+1;
    nmg_SurfaceRegion *new_region = new nmg_SurfaceRegion(this, region_id);
    if (!new_region->init(d_initWidth, d_initHeight)) {
        return -1;
    }

    new_region->copy(d_defaultRegion);

    if (d_numSubRegions >= d_maxNumRegions) {
        d_maxNumRegions *= 2;
        nmg_SurfaceRegion** new_regions = 
            (nmg_SurfaceRegion**)malloc(d_maxNumRegions * sizeof(nmg_SurfaceRegion**));
        for(int i = 0; i < d_numSubRegions; i++) {
            new_regions[i] = d_subRegions[i];
        }

        free(d_subRegions);
        d_subRegions = new_regions;
    }

    d_subRegions[d_numSubRegions] = new_region;
    d_numSubRegions++;
    return region_id;
}

/**
 Removes a region from the set of regions 
            sub-dividing the surface
    Access: Public
*/
void nmg_Surface::
destroyRegion(int region)
{
    //I really wish we could use STL vectors....
    if (region > 0 && region <= d_numSubRegions) {
        region--;
        nmg_SurfaceRegion *victim = d_subRegions[region];
        
        d_defaultRegion->getMaskPlane()->invertSubtract(victim->getMaskPlane());
        for(int i = region; i < d_numSubRegions-1; i++) {
            d_subRegions[i] = d_subRegions[i+1];
        }

        delete victim;
        d_numSubRegions = d_numSubRegions - 1;
        d_defaultRegion->forceRebuildCondition();
    }
}

/**
 
    Access: Public
*/
nmg_SurfaceRegion* nmg_Surface::
getRegion(int region)
{
    if (region == 0) {
        return d_defaultRegion;
    }
    if (region > 0 && region <= d_numSubRegions) {
        region--;
        return  d_subRegions[region];
    }
    return (nmg_SurfaceRegion*)NULL;
}

/**

    Access: Public
*/
nmb_Dataset* nmg_Surface::
getDataset()
{
  return d_dataset;
}

/**
 Set the plane that controls the functions that
            automatically derive the masking plane
    Access: Public
*/
void nmg_Surface::
setRegionControl(BCPlane *control, int region)
{
    if (region >= 0 && region <= d_numSubRegions) {
        if (region != 0) {
            region--;
            d_subRegions[region]->setRegionControl(control);
        }
        else {
            d_defaultRegion->setRegionControl(control);
        }
    }
}

/**
 Manually set the mask plane
    Access: Public
*/
void nmg_Surface::
setMaskPlane(nmg_SurfaceMask *mask, int region)
{
    if (region >= 0 && region <= d_numSubRegions) {
        if (region != 0) {
            region--;            
            d_subRegions[region]->setMaskPlane(mask);
        }
        else {
            d_defaultRegion->setMaskPlane(mask);
        }
    }
}

/**
 Create a masking plane, using a range of
            height values
    Access: Public
*/
void nmg_Surface::
deriveMaskPlane(float min_height, float max_height, int region)
{
    if (region >= 0 && region <= d_numSubRegions) {
        if (region != 0) {
            region--;
            d_subRegions[region]->deriveMaskPlane(min_height, max_height);
        }
        else {
            d_defaultRegion->deriveMaskPlane(min_height, max_height);
        }
    }
}

/**
 Create a masking plane, using a box method
    Access: Public
*/
void nmg_Surface::
deriveMaskPlane(float center_x, float center_y, float width,float height, 
                float angle, int region)
{
    if (region >= 0 && region <= d_numSubRegions) {
        if (region != 0) {
            region--;            
            d_subRegions[region]->deriveMaskPlane(center_x, center_y, width, 
                                                  height, angle);
        }
        else {
            d_defaultRegion->deriveMaskPlane(center_x, center_y, width, 
                                             height, angle);
        }
    }
}

/**
 
    Access: Public
*/
void nmg_Surface::
rederive(int region) 
{
    if (region >= 0 && region <= d_numSubRegions) {
        if (region != 0) {
            region--;
            if (d_subRegions[region]->needsDerivation()) {                
                nmg_SurfaceMask *mask;

                mask = d_subRegions[region]->getMaskPlane(); 
                // Remove old area from default region
                d_defaultRegion->getMaskPlane()->invertSubtract(mask);
            
                d_subRegions[region]->rederiveMaskPlane(d_dataset);
            
                mask = d_subRegions[region]->getMaskPlane();
                // Default region contains all space not in other regions. 
                d_defaultRegion->getMaskPlane()->invertAdd(mask);
                d_defaultRegion->forceRebuildCondition();
            }
        }
        else {
            d_defaultRegion->rederiveMaskPlane(d_dataset);
        }
    }
}

/**
 
    Access: Public
*/
int nmg_Surface::
rebuildRegion(int region)
{
    if (region == 0) {
        rebuildSurface();
    }
    if (region > 0 && region <= d_numSubRegions) {        
        region--;
        return d_subRegions[region]->rebuildRegion(d_dataset);
    }
    return 0;
}

/**
 Force the entire surface to rebuild.  This
            SHOULD be mostly avoided, as in the general
            case you will only be calling rebuildRegion 
            on the particular region of the surface that
            changed
    Access: Public
*/
int nmg_Surface::
rebuildSurface(vrpn_bool force)
{
    if (d_dataset != (nmb_Dataset*)NULL) {
        for(int i = 0; i < d_numSubRegions; i++) {
            if (!d_subRegions[i]->rebuildRegion(d_dataset, force)) {
                return 0;
            }
        }

        if (!d_defaultRegion->rebuildRegion(d_dataset, force)) {
            return 0;
        }
    }

    return 1;
}

/**
 rebuild the interval of the entire surface
            that changed
    Access: Public
*/
int nmg_Surface::
rebuildInterval(int low_row, int high_row, int strips_in_x)
{
    if (d_dataset != (nmb_Dataset*)NULL) {
        for(int i = 0; i < d_numSubRegions; i++) {
            if (!d_subRegions[i]->rebuildInterval(d_dataset, low_row, 
                                                  high_row, strips_in_x)) {
                return 0;
            }
        }

        if (!d_defaultRegion->rebuildInterval(d_dataset, low_row, 
                                              high_row, strips_in_x)) {
            return 0;
        }
    }
    return 1;
}

/**
 Render the entire surface
    Access: Public
*/
void nmg_Surface::
renderSurface()
{
    if (d_dataset != (nmb_Dataset*)NULL) {
        d_defaultRegion->renderRegion();
        for(int i = 0; i < d_numSubRegions; i++) {
            d_subRegions[i]->renderRegion();
        }
    }
}

/**
 As with all the graphics mutator functions,
            a region value of 0 means apply to all regions
            that aren't unassociate.  And a normal value will
            override this
    Access: Public
*/
void nmg_Surface::
setAlpha(float alpha, int region)
{
    if (region == 0) {
        d_defaultRegion->setAlpha(alpha, VRPN_TRUE);
        for(int i = 0; i < d_numSubRegions; i++) {
            d_subRegions[i]->setAlpha(alpha, VRPN_TRUE);
        }
        return;
    }

    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->setAlpha(alpha, VRPN_FALSE);
    }
}

/**
 As with all the graphics mutator functions,
            a region value of 0 means apply to all regions
            that aren't unassociate.  And a normal value will
            override this
    Access: Public
*/
void nmg_Surface::
enableFilledPolygons(int enable, int region)
{
    if (region == 0) {
        d_defaultRegion->enableFilledPolygons(enable, VRPN_TRUE);
        for(int i = 0; i < d_numSubRegions; i++) {
            d_subRegions[i]->enableFilledPolygons(enable, VRPN_TRUE);
        }
        return;
    }

    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->enableFilledPolygons(enable, VRPN_FALSE);
    }
}

/**
 As with all the graphics mutator functions,
            a region value of 0 means apply to all regions
            that aren't unassociate.  And a normal value will
            override this
    Access: Public
*/
void nmg_Surface::
setTextureDisplayed(int display, int region)
{
    if (region == 0) {
        d_defaultRegion->setTextureDisplayed(display, VRPN_TRUE);
        for(int i = 0; i < d_numSubRegions; i++) {
            d_subRegions[i]->setTextureDisplayed(display, VRPN_TRUE);
        }
        return;
    }

    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->setTextureDisplayed(display, VRPN_FALSE);
    }
}

/**
 As with all the graphics mutator functions,
            a region value of 0 means apply to all regions
            that aren't unassociate.  And a normal value will
            override this
    Access: Public
*/
void nmg_Surface::
setTextureMode(int mode, int region)
{
    if (region == 0) {
        d_defaultRegion->setTextureMode(mode, VRPN_TRUE);
        for(int i = 0; i < d_numSubRegions; i++) {
            d_subRegions[i]->setTextureMode(mode, VRPN_TRUE);
        }
        return;
    }

    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->setTextureMode(mode, VRPN_FALSE);
    }
}

/**
 As with all the graphics mutator functions,
            a region value of 0 means apply to all regions
            that aren't unassociate.  And a normal value will
            override this
    Access: Public
*/
void nmg_Surface::
setTextureTransformMode(int mode, int region)
{
    if (region == 0) {
        d_defaultRegion->setTextureTransformMode(mode, VRPN_TRUE);
        for(int i = 0; i < d_numSubRegions; i++) {
            d_subRegions[i]->setTextureTransformMode(mode, VRPN_TRUE);
        }
        return;
    }

    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->setTextureTransformMode(mode, VRPN_FALSE);
    }
}

/**
 As with all the graphics mutator functions,
            a region value of 0 means apply to all regions
            that aren't unassociate.  And a normal value will
            override this
    Access: Public
*/
void nmg_Surface::
setStride(unsigned int stride, int region)
{
    if (region == 0) {
        d_defaultRegion->setStride(stride, VRPN_TRUE);
        for(int i = 0; i < d_numSubRegions; i++) {
            d_subRegions[i]->setStride(stride, VRPN_TRUE);
        }
        return;
    }

    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->setStride(stride, VRPN_FALSE);
    }
}

/**
 
    Access: Public
*/
void nmg_Surface::
associateAlpha(vrpn_bool associate, int region)
{
    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->associateAlpha(associate);
        //Reassociate with the surface
        if (associate == VRPN_FALSE) {
            d_subRegions[region]->setAlpha(d_defaultRegion->getAlpha(), VRPN_TRUE);
        }
    }
}

/**
 
    Access: Public
*/
void nmg_Surface::
associateFilledPolygons(vrpn_bool associate, int region)
{
    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->associateFilledPolygons(associate);
        //Reassociate with the surface
        if (associate == VRPN_FALSE) {
            d_subRegions[region]->enableFilledPolygons(d_defaultRegion->getFilledPolygonsEnabled(), VRPN_TRUE);
        }
    }
}

/**
 
    Access: Public
*/
void nmg_Surface::
associateTextureDisplayed(vrpn_bool associate, int region)
{
    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->associateTextureDisplayed(associate);
        //Reassociate with the surface
        if (associate == VRPN_FALSE) {
            d_subRegions[region]->setTextureDisplayed(d_defaultRegion->getTextureDisplayed(), VRPN_TRUE);
        }
    }
}

/**
 
    Access: Public
*/
void nmg_Surface::
associateTextureMode(vrpn_bool associate, int region)
{
    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->associateTextureMode(associate);
        //Reassociate with the surface
        if (associate == VRPN_FALSE) {
            d_subRegions[region]->setTextureMode(d_defaultRegion->getTextureMode(), VRPN_TRUE);
        }
    }
}

/**
 
    Access: Public
*/
void nmg_Surface::
associateTextureTransformMode(vrpn_bool associate, int region)
{
    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->associateTextureTransformMode(associate);
        //Reassociate with the surface
        if (associate == VRPN_FALSE) {
            d_subRegions[region]->setTextureTransformMode(d_defaultRegion->getTextureTransformMode(), VRPN_TRUE);
        }
    }
}

/**
 
    Access: Public
*/
void nmg_Surface::
associateStride(vrpn_bool associate, int region)
{
    if (region > 0 && region <= d_numSubRegions) {
        region--;
        d_subRegions[region]->associateStride(associate);
        //Reassociate with the surface
        if (associate == VRPN_FALSE) {
            d_subRegions[region]->setStride(d_defaultRegion->getStride(), VRPN_TRUE);
        }
    }
}
