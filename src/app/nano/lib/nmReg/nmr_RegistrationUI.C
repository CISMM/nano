/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmr_RegistrationUI.h"
#include "nmr_Util.h"
#include <nmb_Dataset.h>
#include <microscape.h> // for disableOtherTextures
#include <nmui_ColorMap.h>
#include "nmr_CoarseToFineSearch.h"

nmr_RegistrationUI::nmr_RegistrationUI
  (nmg_Graphics *g, nmb_Dataset *d,
   nmr_Registration_Proxy *aligner):

   d_registrationImageName3D("reg_surface_cm(color_comes_from)", "none"),
   d_registrationImageName2D("reg_projection_cm(color_comes_from)", "none"),
   d_newResampleImageName("resample_image_name", ""),
   d_newResamplePlaneName("resample_plane_name", ""),
   d_registrationEnabled("reg_window_open", 0),
   d_registrationRequested("registration_needed", 0),
   d_constrainToTopography("reg_constrain_to_topography", 0),
   d_invertWarp("reg_invert_warp", 0),
   d_textureDisplayEnabled("reg_display_texture", 0),
   d_resampleResolutionX("resample_resolution_x", 100),
   d_resampleResolutionY("resample_resolution_y", 100),
   d_resampleRatio("reg_resample_ratio", 0),
   d_registrationColorMap3D("reg_surface_cm(color_map)", "none"),
   d_registrationColorMap2D("reg_projection_cm(color_map)", "none"),
   d_registrationValid(vrpn_FALSE),
   d_graphicsDisplay(g),
   d_imageList(d->dataImages),
   d_dataset(d),
   d_aligner(aligner),
   d_3DImageCMap(NULL),
   d_2DImageCMap(NULL)
{
//      d_newResampleImageName = "";
//      d_resampleResolutionX = 100;
//      d_resampleResolutionY = 100;
//      d_registrationImageName3D = "none";
//      d_registrationImageName2D = "none";

    /*
    int i;
    vrpn_bool set3D = vrpn_FALSE, set2D = vrpn_FALSE; 
    nmb_Image *dataim;
    for (i = 0; i < d_imageList->numImages(); i++) {
        dataim = d_imageList->getImage(i);
        // By default we want to favor picking an image that actually 
        // represents a height field as the 3D data
        if ((set2D && !set3D) || (!set3D && 
                                  !strcmp(dataim->unitsValue()->Characters(),
                                          NMB_HEIGHT_UNITS_STR))) {
           d_registrationImageName3D = dataim->name()->Characters();
           set3D = vrpn_TRUE;
           // XXX - this isn't getting called and I don't know why not
           handle_registrationImage3D_change(d_registrationImageName3D, 
				(void *)this);
        }
        else if (!set2D) {
           d_registrationImageName2D = dataim->name()->Characters();
           set2D = vrpn_TRUE;
           // XXX - this isn't getting called and I don't know why not
           handle_registrationImage2D_change(d_registrationImageName2D, 
				(void *)this);
        }
        if (set2D && set3D) {
           break;
        }
    }
    */
    d_aligner->registerChangeHandler((void *)this, handle_registrationChange);
    d_3DImageCMap = new nmui_ColorMap("reg_surface_cm", 
                                      &d_registrationImageName3D,
                                      (Tclvar_list_of_strings *)d_imageList->imageNameList(),
                                      &d_registrationColorMap3D);
    d_3DImageCMap->setSurfaceColor(255,255,255);
    d_2DImageCMap = new nmui_ColorMap("reg_projection_cm", 
                                      &d_registrationImageName2D,
                                      (Tclvar_list_of_strings *)d_imageList->imageNameList(),
                                      &d_registrationColorMap2D);
    d_2DImageCMap->setSurfaceColor(255,255,255);
}

nmr_RegistrationUI::~nmr_RegistrationUI()
{
}

void nmr_RegistrationUI::setupCallbacks() 
{
    d_registrationRequested.addCallback
         (handle_registrationRequest_change, (void *)this);
    d_registrationEnabled.addCallback
         (handle_registrationEnabled_change, (void *)this);
    d_textureDisplayEnabled.addCallback
         (handle_textureDisplayEnabled_change, (void *)this);
    d_newResampleImageName.addCallback
         (handle_resampleImageName_change, (void *)this);
    d_newResamplePlaneName.addCallback
         (handle_resamplePlaneName_change, (void *)this);

    d_registrationImageName3D.addCallback
       (handle_registrationImage3D_change, (void *)this);
    d_registrationImageName2D.addCallback
       (handle_registrationImage2D_change, (void *)this);

    d_registrationColorMap3D.addCallback
       (handle_registrationColorMap3D_change, (void *)this);
    d_registrationColorMap2D.addCallback
       (handle_registrationColorMap2D_change, (void *)this);

    d_3DImageCMap->addMinMaxCallback(handle_registrationMinMax3D_change, this);
    d_2DImageCMap->addMinMaxCallback(handle_registrationMinMax2D_change, this);

}

void nmr_RegistrationUI::teardownCallbacks() 
{
    d_registrationRequested.removeCallback
         (handle_registrationRequest_change, (void *)this);
    d_registrationEnabled.removeCallback
         (handle_registrationEnabled_change, (void *)this);
    d_textureDisplayEnabled.removeCallback
         (handle_textureDisplayEnabled_change, (void *)this);
    d_newResampleImageName.removeCallback
         (handle_resampleImageName_change, (void *)this);
    d_newResamplePlaneName.removeCallback
         (handle_resamplePlaneName_change, (void *)this);

    d_registrationImageName3D.removeCallback
       (handle_registrationImage3D_change, (void *)this);
    d_registrationImageName2D.removeCallback
       (handle_registrationImage2D_change, (void *)this);

    d_registrationColorMap3D.removeCallback
       (handle_registrationColorMap3D_change, (void *)this);
    d_registrationColorMap2D.removeCallback
       (handle_registrationColorMap2D_change, (void *)this);

    d_3DImageCMap->removeMinMaxCallback(handle_registrationMinMax3D_change, this);
    d_2DImageCMap->removeMinMaxCallback(handle_registrationMinMax2D_change, this);
}

void nmr_RegistrationUI::changeDataset(nmb_Dataset *d)
{
  d_imageList=d->dataImages;
  d_dataset = d;
}

void nmr_RegistrationUI::handleRegistrationChange
          (const nmr_ProxyChangeHandlerData &info)
{
  switch(info.msg_type) {
    case NMR_IMAGE_PARAM:
      nmr_ImageType which_image;
      vrpn_int32 res_x, res_y;
      vrpn_float32 size_x, size_y;
      vrpn_bool flip_x, flip_y;
      d_aligner->getImageParameters(which_image, res_x, res_y, 
                                    size_x, size_y, flip_x, flip_y);
      break;
    case NMR_TRANSFORM_OPTION:
      nmr_TransformationType xform_type;
      d_aligner->getTransformationOptions(xform_type);
      break;
    case NMR_REG_RESULT:
      //printf("got transformation\n");
      /* *******************************************************************
         Store the raw result from the registration code and compute inverse
       * *******************************************************************/
      double projImFromTopoIm_matrix[16];
      d_aligner->getRegistrationResult(projImFromTopoIm_matrix);

      nmb_Image *topographyImage = d_imageList->getImageByName
                          (d_registrationImageName3D.string());
      nmb_Image *projectionImage = d_imageList->getImageByName
                          (d_registrationImageName2D.string());
      if (!topographyImage || !projectionImage) {
           fprintf(stderr, "handleRegistrationChange: can't find image\n");
           return;
      }

      double topoAcqDistX, topoAcqDistY, projAcqDistX, projAcqDistY;
      topographyImage->getAcquisitionDimensions(topoAcqDistX, topoAcqDistY);
      projectionImage->getAcquisitionDimensions(projAcqDistX, projAcqDistY);

      // adjust for scaling of images
      int i;
      for (i = 0; i < 4; i++) {
        projImFromTopoIm_matrix[i] *= topoAcqDistX;
        projImFromTopoIm_matrix[i+4] *= topoAcqDistY;
        projImFromTopoIm_matrix[4*i] /= projAcqDistX;
        projImFromTopoIm_matrix[4*i+1] /= projAcqDistY;
      }
      d_ProjImageFromTopoImageTransform.setMatrix(projImFromTopoIm_matrix);

      /* ****************************************************************
         ProjImFromTopoWorld = ProjImFromTopoImage * TopoImFromTopoWorld
       * ****************************************************************/

      nmb_TransformMatrix44 topoImFromTopoWorld;
      topographyImage->getWorldToImageTransform(topoImFromTopoWorld);
  
      d_ProjImageFromTopoWorldTransform.setMatrix(projImFromTopoIm_matrix);

/*
      If things don't look right, this may be useful for debugging:
      double topoWorldCenter[4] = {0,0,0,1};
      topoWorldCenter[2] = topographyImage->getValueInterpolated(
                                            0.5*topographyImage->width(), 
                                            0.5*topographyImage->height());
      topoWorldCenter[0] = 0.25*(
              topographyImage->boundX(nmb_ImageBounds::MIN_X_MIN_Y) +
              topographyImage->boundX(nmb_ImageBounds::MAX_X_MIN_Y) +
              topographyImage->boundX(nmb_ImageBounds::MIN_X_MAX_Y) +
              topographyImage->boundX(nmb_ImageBounds::MAX_X_MAX_Y));
      topoWorldCenter[1] = 0.25*(
              topographyImage->boundY(nmb_ImageBounds::MIN_X_MIN_Y) +
              topographyImage->boundY(nmb_ImageBounds::MAX_X_MIN_Y) +
              topographyImage->boundY(nmb_ImageBounds::MIN_X_MAX_Y) +
              topographyImage->boundY(nmb_ImageBounds::MAX_X_MAX_Y));
      double topoImCenter[4];
      topoImFromTopoWorld.transform(topoWorldCenter, topoImCenter);

      printf("topoImFromTopoWorld: (%g,%g,%g,%g)->(%g,%g,%g,%g)\n",
        topoWorldCenter[0], topoWorldCenter[1], topoWorldCenter[2], 
        topoWorldCenter[3], topoImCenter[0], topoImCenter[1], topoImCenter[2],
        topoImCenter[3]);

      double projImPoint[4];
      d_ProjImageFromTopoWorldTransform.transform(topoImCenter, projImPoint);

      printf("projImFromTopoIm: (%g,%g,%g,%g)->(%g,%g,%g,%g)\n",
        topoImCenter[0], topoImCenter[1], topoImCenter[2], topoImCenter[3],
        projImPoint[0], projImPoint[1], projImPoint[2], projImPoint[3]);
*/
      // topoImFromTopoWorld: center of image in nm -> (0.5, 0.5, 0)


      d_ProjImageFromTopoWorldTransform.
                           compose(topoImFromTopoWorld);

      /* ****************************************************************
         ProjWorldFromTopoWorld = ProjWorldFromProjIm * ProjImFromTopoWorld
       * ****************************************************************/
      //  first invert the transformation matrix that we get from the image
      nmb_TransformMatrix44 projWorldFromProjImage;
      projectionImage->getWorldToImageTransform(projWorldFromProjImage);
      projWorldFromProjImage.invert();

      d_ProjWorldFromTopoWorldTransform = projWorldFromProjImage;
      d_ProjWorldFromTopoWorldTransform.
                     compose(d_ProjImageFromTopoWorldTransform);

      double projImFromTopoWorld_matrix[16];
      // send the right transformation to the graphics code
      d_ProjImageFromTopoWorldTransform.getMatrix(projImFromTopoWorld_matrix);
      d_graphicsDisplay->setTextureTransform(projImFromTopoWorld_matrix);
      d_registrationValid = vrpn_TRUE;
      break;
  }
}

// static
void nmr_RegistrationUI::handle_registrationChange
          (void *ud, const nmr_ProxyChangeHandlerData &info)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->handleRegistrationChange(info);
}

// static 
void nmr_RegistrationUI::handle_resampleImageName_change(const char *name, 
                                                         void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->createResampleImage(name);

}

// static 
void nmr_RegistrationUI::handle_resamplePlaneName_change(const char *name, 
                                                         void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->createResamplePlane(name);

}

// static
void nmr_RegistrationUI::handle_registrationImage3D_change(const char *name,
                                                           void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    nmb_Image *im = me->d_imageList->getImageByName(name);
    if (!im) {
        fprintf(stderr, "nmr_RegistrationUI::image not found: %s\n", name);
        return;
    }
    // send image off to the proxy
    me->d_aligner->setImage(NMR_SOURCE, im, vrpn_FALSE, vrpn_FALSE);
    // We have a choice, and I'm not sure which is right. Either
    // Set the new image to use the existing colormap params:
    double dmin,dmax,cmin,cmax;
    me->d_3DImageCMap->getDataColorMinMax(&dmin, &dmax, &cmin, &cmax);
    me->d_3DImageCMap->setColorMinMaxLimit(0,1);
    me->d_aligner->setColorMinMax(NMR_SOURCE, dmin, dmax, cmin, cmax);
    // Or reset the colormap params to their default:
    //me->d_3DImageCMap->setColorMinMaxLimit(0,1);
}

// static
void nmr_RegistrationUI::handle_registrationImage2D_change(const char *name,
                                                           void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    nmb_Image *im = me->d_imageList->getImageByName(name);
    if (!im) {
        fprintf(stderr, "nmr_RegistrationUI::image not found: %s\n", name);
        return;
    }
    // send image off to the proxy
    me->d_aligner->setImage(NMR_TARGET, im, vrpn_FALSE, vrpn_FALSE);

    // We have a choice, and I'm not sure which is right. Either
    // Set the new image to use the existing colormap params:
    double dmin,dmax,cmin,cmax;
    me->d_2DImageCMap->getDataColorMinMax(&dmin, &dmax, &cmin, &cmax);
    me->d_2DImageCMap->setColorMinMaxLimit(0,1);
    me->d_aligner->setColorMinMax(NMR_TARGET, dmin, dmax, cmin, cmax);
    // Or reset the colormap params to their default:
    //me->d_2DImageCMap->setColorMinMaxLimit(0,1);

    // set up texture in graphics
    me->d_graphicsDisplay->setRealignTexturesConversionMap(
        me->d_2DImageCMap->getColorMapName(), "");
    me->d_graphicsDisplay->setRealignTextureSliderRange(dmin, dmax, cmin,cmax);
    //printf("creating realign texture for %s\n", name);
    me->d_graphicsDisplay->createRealignTextures(name);
}

// static
void nmr_RegistrationUI::handle_registrationColorMap3D_change(const char *name,
                                                           void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    nmb_ColorMap * cmap =  me->d_3DImageCMap->currentColorMap();
    if (!cmap) {
        fprintf(stderr, "nmr_RegistrationUI::colormap not found: %s\n", name);
        return;
    }
    // send changes off to the proxy
    me->d_aligner->setColorMap(NMR_SOURCE, cmap);
}

// static
void nmr_RegistrationUI::handle_registrationColorMap2D_change(const char *name,
                                                           void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    nmb_ColorMap * cmap =  me->d_2DImageCMap->currentColorMap();
    if (!cmap) {
        fprintf(stderr, "nmr_RegistrationUI::colormap not found: %s\n", name);
        return;
    }
    // send changes off to the proxy
    me->d_aligner->setColorMap(NMR_TARGET, cmap);

    // set up texture in graphics
    me->d_graphicsDisplay->setRealignTexturesConversionMap(
        me->d_2DImageCMap->getColorMapName(), "");
    //printf("creating realign texture for %s\n", name);
    me->d_graphicsDisplay->createRealignTextures(me->d_registrationImageName2D.string());
}

//static 
void nmr_RegistrationUI::handle_registrationMinMax3D_change(vrpn_float64, void *ud) {
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    double dmin,dmax,cmin,cmax;
    me->d_3DImageCMap->getDataColorMinMax(&dmin, &dmax, &cmin, &cmax);
    // send changes off to the proxy
    me->d_aligner->setColorMinMax(NMR_SOURCE, dmin, dmax, cmin, cmax);
}

//static 
void nmr_RegistrationUI::handle_registrationMinMax2D_change(vrpn_float64, void *ud) {
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    double dmin,dmax,cmin,cmax;
     me->d_2DImageCMap->getDataColorMinMax(&dmin, &dmax, &cmin, &cmax);
    // send changes off to the proxy
    me->d_aligner->setColorMinMax(NMR_TARGET, dmin, dmax, cmin, cmax);
    // set up texture in graphics
//      me->d_graphicsDisplay->setRealignTexturesConversionMap(
//          me->d_2DImageCMap->getColorMapName(), "");
    me->d_graphicsDisplay->setRealignTextureSliderRange(dmin, dmax, cmin,cmax);
    //printf("creating realign texture for %s\n", name);
    me->d_graphicsDisplay->createRealignTextures(me->d_registrationImageName2D.string());
}

// static
void nmr_RegistrationUI::handle_textureDisplayEnabled_change(
      vrpn_int32 value, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    printf("enabling reg texture: %d\n", value);
    if (value) {
      disableOtherTextures(REGISTRATION);
      me->d_graphicsDisplay->setTextureMode(nmg_Graphics::COLORMAP,
                                            nmg_Graphics::REGISTRATION_COORD);
    } else {
      if (me->d_graphicsDisplay->getTextureMode() == nmg_Graphics::COLORMAP) {
          me->d_graphicsDisplay->setTextureMode(nmg_Graphics::NO_TEXTURES,
                                             nmg_Graphics::RULERGRID_COORD);
      }
    }
}

// static
void nmr_RegistrationUI::handle_registrationRequest_change(
      vrpn_int32 value, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    if (value) {
      me->d_registrationValid = vrpn_FALSE;
      me->d_aligner->registerImages();
    }
}

// static
void nmr_RegistrationUI::handle_registrationEnabled_change(
      vrpn_int32 value, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    if (value) {
        me->d_aligner->setGUIEnable(vrpn_TRUE);
        // We're popping up our window. Set the 3D image to 
        // height plane. 
        me->d_registrationImageName3D = me->d_dataset->heightPlaneName->string();
        // Also guess at the 2D image name
        int i;
        nmb_Image *dataim;
        for (i = 0; i < me->d_imageList->numImages(); i++) {
            dataim = me->d_imageList->getImage(i);
            //printf("Considering %s\n", dataim->name()->Characters());
            if (strcmp(dataim->name()->Characters(),
                       me->d_dataset->heightPlaneName->string())) {
                me->d_registrationImageName2D = dataim->name()->Characters();
                break;
            }
        }
    } else {
        me->d_aligner->setGUIEnable(vrpn_FALSE);
    }
}


void nmr_RegistrationUI::createResampleImage(const char * /*imageName */)
{
    printf("nmr_RegistrationUI::createResampleImage\n");
    nmb_Image *new_image;
    nmb_Image *im_3D = d_imageList->getImageByName
                          (d_registrationImageName3D.string());
    nmb_Image *im_2D = d_imageList->getImageByName
                          (d_registrationImageName2D.string());

    // d_imageTransformWorldSpace is the transformation that takes points from
    // height image to points in the texture image

    // Make sure we have all the information we need and
    // see if the user has given a name to the resampled plane
    // other than "".  If so, we should create a new plane and set the value
    // back to "".
    if (!d_registrationValid) {
        fprintf(stderr,
                "Error, cannot resample if registration is not valid\n");
        return;
    } else if (!im_3D || !im_2D ||
               (strlen(d_newResampleImageName.string()) == 0)){
        return;

    } else if (d_invertWarp) {    // warp the AFM image to the SEM image
        new_image = new nmb_ImageGrid(
                (const char *)(d_newResampleImageName.string()),
                (const char *)(im_3D->unitsValue()),
                d_resampleResolutionX, d_resampleResolutionY);
        d_newResampleImageName = (const char *) "";
        nmb_ImageBounds im2D_bounds;
        im_2D->getBounds(im2D_bounds);
        new_image->setBounds(im2D_bounds);
        nmb_TransformMatrix44 topoImageFromProjImageTransform;
        topoImageFromProjImageTransform = d_ProjImageFromTopoImageTransform;
        topoImageFromProjImageTransform.invert();
        nmr_Util::createResampledImageWithImageSpaceTransformation((*im_3D), 
                 topoImageFromProjImageTransform, (*new_image));

        // an extra step: combine the two datasets in a somewhat arbitrary
        // way so that you can see features from both in a single image
        // it would be nice to create a color image from these two instead:
        nmr_Util::addImage((*im_2D), (*new_image), (double)d_resampleRatio,
                  1.0-(double)d_resampleRatio);

    } else {                      // warp the SEM image to the AFM image
      if (d_constrainToTopography){
      // resample the SEM data onto the AFM grid only
        // it is possible to change resolution or select a subregion
        // of the height field at this point by just changing what
        // resolution we give to the constructor and what image
        // bounds we set and everything should just work correctly:
        new_image = new nmb_ImageGrid(
                (const char *)(d_newResampleImageName.string()),
                (const char *)(im_2D->unitsValue()),
                d_resampleResolutionX, d_resampleResolutionY);
        nmb_ImageBounds im3D_bounds;
        im_3D->getBounds(im3D_bounds);
        new_image->setBounds(im3D_bounds);
        TopoFile tf;
        im_3D->getTopoFileInfo(tf);
        new_image->setTopoFileInfo(tf);
        d_newResampleImageName = (const char *) "";
        nmr_Util::createResampledImage((*im_2D), (*im_3D),
                 d_ProjWorldFromTopoWorldTransform, (*new_image));
      } else {
     
        // here we figure out what resolution is required to preserve
        // the entire 2D projection image while keeping the resolution of
        // height field constant
        int min_i, min_j, max_i, max_j;
        nmr_Util::computeResampleExtents((*im_3D), (*im_2D), 
                    d_ProjWorldFromTopoWorldTransform, 
                                         min_i, min_j, max_i, max_j);
        int res_x = max_i - min_i;
        int res_y = max_j - min_j;
        printf("got extents: %d x %d\n", res_x, res_y);

        new_image = new nmb_ImageGrid(
                (const char *)(d_newResampleImageName.string()),
                (const char *)(im_2D->unitsValue()),
                res_x, res_y);
        printf("allocated image\n");
        TopoFile tf;
        im_3D->getTopoFileInfo(tf);
        new_image->setTopoFileInfo(tf);
        printf("%d, %d, %d, %d\n", min_i, min_j, max_i, max_j);
        nmr_Util::setRegionRelative((*im_3D), (*new_image),
                 min_i, min_j, max_i, max_j);
        nmr_Util::createResampledImage((*im_2D), 
                 d_ProjWorldFromTopoWorldTransform,
                 (*new_image));

        // HACK:
        // an extra step: combine the two datasets in a somewhat arbitrary
        // way so that you can see features from both in a single image
        // it would be nice to create a color image from these two instead:
        nmr_Util::addImage((*im_3D), (*new_image), (double)d_resampleRatio,
                  1.0-(double)d_resampleRatio);

      }
    }
    printf("finished resampling image\n");
    // now make it available elsewhere:
    d_imageList->addImage(new_image);
}

void nmr_RegistrationUI::createResamplePlane(const char * /*imageName */)
{
    printf("nmr_RegistrationUI::createResamplePlane\n");
    nmb_ImageGrid *new_image;
    nmb_Image *im_3D = d_imageList->getImageByName
                          (d_registrationImageName3D.string());
    nmb_Image *im_2D = d_imageList->getImageByName
                          (d_registrationImageName2D.string());

    // d_imageTransformWorldSpace is the transformation that takes points from
    // height image to points in the texture image

    // Make sure we have all the information we need and
    // see if the user has given a name to the resampled plane
    // other than "".  If so, we should create a new plane and set the value
    // back to "".
    if (!d_registrationValid) {
        fprintf(stderr,
                "Error, cannot resample if registration is not valid\n");
        return;
    } else if (!im_3D || !im_2D ||
               (strlen(d_newResamplePlaneName.string()) == 0)){
        return;

    } 
    // resample the SEM data onto the AFM grid only 
    //Region is the same as the AFM grid, resolution is the same as the AFM
    //grid
        new_image = new nmb_ImageGrid(
                (const char *)(d_newResamplePlaneName.string()),
                (const char *)(im_2D->unitsValue()),
                im_3D->width(), im_3D->height());
        nmb_ImageBounds im3D_bounds;
        im_3D->getBounds(im3D_bounds);
        new_image->setBounds(im3D_bounds);
        TopoFile tf;
        im_3D->getTopoFileInfo(tf);
        new_image->setTopoFileInfo(tf);
        d_newResamplePlaneName = (const char *) "";
        nmr_Util::createResampledImage((*im_2D), (*im_3D),
                 d_ProjWorldFromTopoWorldTransform, (*new_image));

    printf("finished resampling image\n");
    // now make it available elsewhere:
    d_dataset->addImageToGrid(new_image);
    //d_imageList->addImage(new_image);
    nmb_Image::deleteImage(new_image);
}

