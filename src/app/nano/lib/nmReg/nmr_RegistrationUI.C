#include "nmr_RegistrationUI.h"
#include "nmr_Util.h"
#include <microscape.h> // for disableOtherTextures

nmr_RegistrationUI::nmr_RegistrationUI
  (nmg_Graphics *g, nmb_ImageList *im,
   nmr_Registration_Proxy *aligner):

   d_registrationImageName3D("reg_surface_comes_from", "none"),
   d_registrationImageName2D("reg_projection_comes_from", "none"),
   d_newResampleImageName("resample_image_name", ""),
   d_registrationEnabled("reg_window_open", 0),
   d_registrationRequested("registration_needed", 0),
   d_constrainToTopography("reg_constrain_to_topography", 0),
   d_textureDisplayEnabled("reg_display_texture", 0),
   d_resampleResolutionX("resample_resolution_x", 100),
   d_resampleResolutionY("resample_resolution_y", 100),
   d_resampleRatio("reg_resample_ratio", 0),
   d_registrationValid(vrpn_FALSE),
   d_graphicsDisplay(g),
   d_imageList(im),
   d_aligner(aligner),
   d_imageTransform(4,4)
{
    d_registrationRequested.addCallback
         (handle_registrationRequest_change, (void *)this);
    d_registrationEnabled.addCallback
         (handle_registrationEnabled_change, (void *)this);
    d_textureDisplayEnabled.addCallback
         (handle_textureDisplayEnabled_change, (void *)this);
    d_newResampleImageName = "";
    d_newResampleImageName.addCallback
         (handle_resampleImageName_change, (void *)this);
    d_resampleResolutionX = 100;
    d_resampleResolutionY = 100;

    d_registrationImageName3D.addCallback
       (handle_registrationImage3D_change, (void *)this);
    d_registrationImageName2D.addCallback
       (handle_registrationImage2D_change, (void *)this);

    int i;
    vrpn_bool set3D = vrpn_FALSE, set2D = vrpn_FALSE; 
    nmb_Image *dataim;
    for (i = 0; i < d_imageList->numImages(); i++) {
        dataim = d_imageList->getImage(i);
        if (!set3D && dataim->isHeightField()) {
           d_registrationImageName3D = dataim->name()->Characters();
           set3D = vrpn_TRUE;
           // XXX - this isn't getting called and I don't know why not
           handle_registrationImage3D_change(d_registrationImageName3D, 
				(void *)this);
        }
        if (!set2D && !dataim->isHeightField()) {
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

    d_aligner->registerChangeHandler((void *)this, handle_registrationChange);
}

nmr_RegistrationUI::~nmr_RegistrationUI()
{
}

void nmr_RegistrationUI::handleRegistrationChange
          (const nmr_ProxyChangeHandlerData &info)
{
    switch(info.msg_type) {
      case NMR_IMAGE_PARAM:
        nmr_ImageType which_image;
        vrpn_int32 res_x, res_y;
        vrpn_bool height_field;
        d_aligner->getImageParameters(which_image, res_x, res_y, height_field);
        break;
      case NMR_TRANSFORM_OPTION:
        nmr_TransformationType xform_type;
        d_aligner->getTransformationOptions(xform_type);
        break;
      case NMR_REG_RESULT:
        d_imageTransform.print();
        printf("got transformation\n");
        vrpn_float64 transform_matrix[16];
        d_aligner->getRegistrationResult(transform_matrix);
        // convert it into the proper units
        double worldToImage_matrix[16];
        nmb_Image *im = d_imageList->getImageByName
                          (d_registrationImageName3D.string());
        if (!im) {
             fprintf(stderr, "handleRegistrationChange: can't find image\n");
             return;
        }
        im->getWorldToImageTransform(worldToImage_matrix);
        nmr_ImageTransformAffine worldToImage(4,4);
        worldToImage.setMatrix(transform_matrix);
        worldToImage.print();
        // save it for future reference when resampling
        d_imageTransform.setMatrix(worldToImage_matrix);
        d_imageTransform.print();
        d_imageTransform.compose(worldToImage);
        d_imageTransform.print();
        // and send it off to graphics
        d_imageTransform.getMatrix(transform_matrix);
        d_registrationValid = vrpn_TRUE;
        d_graphicsDisplay->setTextureTransform(transform_matrix);
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
void nmr_RegistrationUI::handle_registrationImage3D_change(const char *name,
                                                           void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    nmb_Image *im = me->d_imageList->getImageByName(name);
    if (!im) {
        fprintf(stderr, "image not found: %s\n", name);
        return;
    }
    // send image off to the proxy
    me->d_aligner->setImage(NMR_SOURCE, im, vrpn_FALSE);
}

// static
void nmr_RegistrationUI::handle_registrationImage2D_change(const char *name,
                                                           void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    nmb_Image *im = me->d_imageList->getImageByName(name);
    if (!im) {
        fprintf(stderr, "image not found: %s\n", name);
        return;
    }
    // send image off to the proxy
    me->d_aligner->setImage(NMR_TARGET, im, vrpn_FALSE);
    // set up texture in graphics
    printf("creating realign texture for %s\n", name);
    me->d_graphicsDisplay->createRealignTextures(name);
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

    // d_imageTransform is the transformation that takes points from
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
    } else if (d_constrainToTopography){
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
        d_newResampleImageName = (const char *) "";
        nmr_Util::createResampledImage((*im_2D), (*im_3D),
                                       d_imageTransform, (*new_image));
    } else {
     
        // here we figure out what resolution is required to preserve
        // the entire 2D projection image while keeping the resolution of
        // height field constant
        int min_i, min_j, max_i, max_j;
        nmr_Util::computeResampleExtents((*im_3D), (*im_2D), d_imageTransform,
                                         min_i, min_j, max_i, max_j);
        int res_x = max_i - min_i;
        int res_y = max_j - min_j;
        printf("got extents: %d x %d\n", res_x, res_y);

        new_image = new nmb_ImageGrid(
                (const char *)(d_newResampleImageName.string()),
                (const char *)(im_2D->unitsValue()),
                res_x, res_y);
        printf("allocated image\n");

        printf("%d, %d, %d, %d\n", min_i, min_j, max_i, max_j);
        nmr_Util::setRegionRelative((*im_3D), (*new_image),
                 min_i, min_j, max_i, max_j);
        nmr_Util::createResampledImage((*im_2D), d_imageTransform,
                 (*new_image));

        // HACK:
        // an extra step: combine the two datasets in a somewhat arbitrary
        // way so that you can see features from both in a single image
        // it would be nice to create a color image from these two instead:
        nmr_Util::addImage((*im_3D), (*new_image), (double)d_resampleRatio,
                  1.0-(double)d_resampleRatio);

        printf("resampled image\n");
    }
    // now make it available elsewhere:
    d_imageList->addImage(new_image);
}

