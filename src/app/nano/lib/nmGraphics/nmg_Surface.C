/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <stdlib.h>

#include <nmb_Dataset.h>
#include <BCPlane.h>

// timing Debug
#include <vrpn_Shared.h>

#include "nmg_Surface.h"
#include "nmg_SurfaceRegion.h"
#include "nmg_SurfaceMask.h"

#include "nmg_State.h"
/**
 
    Access: Public
*/
nmg_Surface::
nmg_Surface()
{
    d_numSubRegions = 0;
    d_maxNumRegions = 2;
    d_subRegions = new nmg_SurfaceRegion*[d_maxNumRegions];
    d_defaultRegion = new nmg_SurfaceRegion(this, 0);
    d_dataset = (nmb_Dataset*)NULL;

    d_defaultRegion->getMaskPlane()->setDrawPartialMask(VRPN_TRUE);
    d_display_lists_in_x = 1;	/* Are display lists strips in X? */
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
    delete [] d_subRegions;
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
    if (d_defaultRegion->init(d_initWidth, d_initHeight)) {
        return -1;
    }

    for(int i = 0; i < d_numSubRegions; i++) {
        if (d_subRegions[i]->init(d_initWidth, d_initHeight)) {
            return -1;
        }
    }
    return 0;
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
    // Make sure default mask doesn't refer to non-existant masks. 
    d_defaultRegion->getMaskPlane()->deriveMask(NULL, d_numSubRegions);
}

/** Helper function to update default region mask */
int nmg_Surface::updateDefaultMask() 
{
    // Update the default region mask. 
    nmg_SurfaceMask ** maskPlanes = new nmg_SurfaceMask*[d_numSubRegions];
    if (!maskPlanes) return -1;
    for (int i = 0; i < d_numSubRegions; i++) {
        maskPlanes[i] = d_subRegions[i]->getMaskPlane();
    }
    d_defaultRegion->getMaskPlane()->deriveMask(maskPlanes, d_numSubRegions);
    delete [] maskPlanes;
    return 0;
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
    if (new_region->init(d_initWidth, d_initHeight)) {
        return -1;
    }

    new_region->copy(d_defaultRegion);

    if (d_numSubRegions >= d_maxNumRegions) {
        d_maxNumRegions *= 2;
        nmg_SurfaceRegion** new_regions = 
            new nmg_SurfaceRegion*[d_maxNumRegions];
        for(int i = 0; i < d_numSubRegions; i++) {
            new_regions[i] = d_subRegions[i];
        }

        delete [] d_subRegions;
        d_subRegions = new_regions;
    }

    d_subRegions[d_numSubRegions] = new_region;
    d_numSubRegions++;

    updateDefaultMask();
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
        
        // Doesn't apply since default region mask is type INVERTMASKS
        //d_defaultRegion->getMaskPlane()->invertSubtract(victim->getMaskPlane());
        for(int i = region; i < d_numSubRegions-1; i++) {
            d_subRegions[i] = d_subRegions[i+1];
        }

        delete victim;
        d_numSubRegions = d_numSubRegions - 1;
        d_defaultRegion->forceRebuildCondition();
        updateDefaultMask();
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
            updateDefaultMask();
        }
        else {
            // I don't think we want to allow this....
            fprintf(stderr, "nmg_Surface::setMaskPlane: can't set default region mask. \n");
            //d_defaultRegion->setMaskPlane(mask);
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
            // I don't think we want to allow this....
            fprintf(stderr, "nmg_Surface::deriveMaskPlane: can't set default region mask. \n");
            //d_defaultRegion->deriveMaskPlane(min_height, max_height);
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
            // I don't think we want to allow this....
            fprintf(stderr, "nmg_Surface::deriveMaskPlane: can't set default region mask. \n");
            //d_defaultRegion->deriveMaskPlane(center_x, center_y, width, 
            //                                 height, angle);
        }
    }
}

/**
 Create a masking plane, using invalid data range
    Access: Public
*/
void nmg_Surface::
deriveMaskPlane(int region)
{
    if (region >= 0 && region <= d_numSubRegions) {
        if (region != 0) {
            region--;
            d_subRegions[region]->deriveMaskPlane();
        }
        else {
            // I don't think we want to allow this....
            fprintf(stderr, "nmg_Surface::deriveMaskPlane: can't set default region mask. \n");
            //d_defaultRegion->deriveMaskPlane();
        }
    }
}

/**
   Call before generating display lists for a region.
    Access: Public
    @return 1 if entire region must be rebuilt, 0 if only the
    strips between \a low_row and \a high_row need to be rebuilt
    (and will be taken care of later by rebuildInterval)

    @note Update default region first, because it has dependencies on other
    masks. 
*/
/*
int nmg_Surface::
updateMask(int low_row = -1, int high_row = -1, int region)
{
//      struct timeval then, now;
//      gettimeofday(&then, NULL);


    if (region >= 0 && region <= d_numSubRegions) {
        if (region != 0) {
            region--;
        if (d_subRegions[region]->needsUpdate()) {                
            d_subRegions[region]->updateMaskPlane(d_dataset, low_row, high_row);
        } else {
            d_defaultRegion->updateMaskPlane(d_dataset, low_row, high_row);

        }
    }
//      gettimeofday(&now, NULL);
//      now = vrpn_TimevalDiff(now, then);
//      printf("updateMask took %ld %ld\n", now.tv_sec, now.tv_usec);
    return 0;
}
*/
/**
   Too slow - shouldn't be triggered outside rebuildSurface and 
   rebuildInterval below
*/
/*
int nmg_Surface::
rebuildRegion(nmg_State * state, int region)
{
    if (region == 0) {
        rebuildSurface(state);
    }
    if (region > 0 && region <= d_numSubRegions) {        
        region--;
        return d_subRegions[region]->rebuildRegion(d_dataset, state, 
                                                   d_display_lists_in_x );
    }
    return 0;
}
*/

/**
 Force the entire surface to reconstruct its display lists. 
  Triggered by causeGridRebuild in nmg_GraphicsImpl.C
  Don't call every frame - much too slow!
    Access: Public
    @return -1 on error, 0 on success. 
*/
int nmg_Surface::
rebuildSurface(nmg_State * state)
{
  if (d_dataset != NULL) {

    d_dataset->range_of_change.ChangeAll();

    // Now that we've marked the whole surface changed...
    return (rebuildInterval(state));
  }

  return 0;
}

/**
 rebuild the interval of the entire surface
            that changed
    Access: Public
 @return -1 on error, 0 on success. 
*/
int nmg_Surface::
rebuildInterval(nmg_State * state)
{
    // Semi-optimized, thread-safe version of display list redrawing code.
    // TCH 17 June 98
    
    // Trys to figure out which direction the scanning tip is moving
    // to reduce the amount of redrawing done.  Delays by one graphics
    // loop drawing strips that it expects to have to redraw after
    // receiving the next update from the microscope.
    
    // (Nearly) starvation-free:  if we ever stop receiving updates from
    // the microscope, all old cached lines (stored in last_marked)
    // are updated on the next pass through this routine.  There is one
    // starvation case:  if the microscope ever continuously scans and
    // rescans the same line of the sample more often than the graphics
    // process runs, that new data will never be drawn.
    
    // If the scan direction is inefficiently using the display 
    // lists, switch to using display lists in the other direction.
    if (d_dataset->range_of_change.Changed()) {
        float	ratio;
        
        /* See which way more changes occurred */
        ratio = d_dataset->range_of_change.RatioOfChange();
        
        /* If the ratio is very skewed, make sure we are
	 * scanning in the correct direction. */
        if (ratio > 4) {		/* 4x as much in y */
            if (d_display_lists_in_x) {	/* Going wrong way */
                d_display_lists_in_x = 0;
            }
        } else if (ratio < 0.25) {	/* 4x as much in x */
            if (!d_display_lists_in_x) {	/* Going wrong way */
                d_display_lists_in_x = 1;
            }
        }
    }

    int low_row;
    int high_row;
    
    // Get the data (low and high X, Y values changed) atomically
    // so we have bulletproof synchronization.
    
    d_dataset->range_of_change.GetBoundsAndClear
        (&state->minChangedX, &state->maxChangedX, &state->minChangedY, &state->maxChangedY);
    if (state->PRERENDERED_COLORS || state->PRERENDERED_DEPTH) {
        state->prerenderedChange->GetBoundsAndClear
            (&state->minChangedX, &state->maxChangedX, &state->minChangedY, &state->maxChangedY);
    }
    
    if (d_display_lists_in_x) {
        low_row = state->minChangedY;
        high_row = state->maxChangedY;
    } else {
        low_row = state->minChangedX;
        high_row = state->maxChangedX;
    }

    if (d_dataset != NULL) {
        // Call default region first because it's mask depends on
        // other masks, and whether they've been changed. 
        if (d_defaultRegion->rebuildInterval(d_dataset, state, low_row, 
                                      high_row, d_display_lists_in_x)) {
            return -1;
        }
        for(int i = 0; i < d_numSubRegions; i++) {
            if (d_subRegions[i]->rebuildInterval(d_dataset, state, low_row, 
                                      high_row, d_display_lists_in_x)) {
                return -1;
            }
        }

    }
    return 0;
}


/**
 Recolor the entire surface, without redoing the display lists or normals.
    Access: Public
@return -1 on error, 0 on success. 
*/
int nmg_Surface::
recolorSurface()
{
    if (d_dataset != (nmb_Dataset*)NULL) {
        for(int i = 0; i < d_numSubRegions; i++) {
            if (d_subRegions[i]->recolorRegion()) {
                return -1;
            }
        }

        if (d_defaultRegion->recolorRegion()) {
            return -1;
        }
    }

    return 0;
}

/**
 Render the entire surface
    Access: Public
*/
void nmg_Surface::
renderSurface(nmg_State * state)
{
    if (d_dataset != (nmb_Dataset*)NULL) {
        d_defaultRegion->renderRegion(state, d_dataset);
        for(int i = 0; i < d_numSubRegions; i++) {
            d_subRegions[i]->renderRegion(state, d_dataset);
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
