#include "nmg_ImageDisplayProjectiveTexture.h"

nmg_ImageDisplayProjectiveTexture::
nmg_ImageDisplayProjectiveTexture( nmg_Graphics *g ):
  nmb_ImageDisplay(),
  d_graphicsDisplay(g), d_projectiveTexturesEnabled(vrpn_FALSE)
{

}

void nmg_ImageDisplayProjectiveTexture::
addImageToDisplay( nmb_Image* /*image*/ )
{
  if (!d_graphicsDisplay) return;
  d_graphicsDisplay->setTextureMode(nmg_Graphics::COLORMAP,
                                    nmg_Graphics::REGISTRATION_COORD);
  d_projectiveTexturesEnabled = vrpn_TRUE;
}

void nmg_ImageDisplayProjectiveTexture::
removeImageFromDisplay( nmb_Image* /*image*/ )
{
  if (!d_graphicsDisplay) return;
  if (!d_projectiveTexturesEnabled) return;
  
  // this check is very important because we don't want to disable textures
  // for some other mode
  if (d_graphicsDisplay->getTextureMode() != nmg_Graphics::COLORMAP) return;

  d_graphicsDisplay->setTextureMode(nmg_Graphics::NO_TEXTURES,
                                    nmg_Graphics::RULERGRID_COORD);
  d_projectiveTexturesEnabled = vrpn_FALSE;
}

void nmg_ImageDisplayProjectiveTexture::
updateDisplayTransform( nmb_Image * /*image*/, double *transform )
{
  if (!d_graphicsDisplay) return;

  d_graphicsDisplay->setTextureTransform(transform);
}

void nmg_ImageDisplayProjectiveTexture::
setDisplayColorMap( nmb_Image * /*image*/, const char *map, const char *mapdir )
{
  if (!d_graphicsDisplay) return;
  d_graphicsDisplay->setRealignTexturesConversionMap(map, mapdir);
}

void nmg_ImageDisplayProjectiveTexture::
setDisplayColorMapRange( nmb_Image * /*image*/,
			 float data_min, float data_max,
			 float color_min, float color_max)
{
  if (!d_graphicsDisplay) return;
  d_graphicsDisplay->setRealignTextureSliderRange(data_min, data_max,
                                                    color_min, color_max);
}

void nmg_ImageDisplayProjectiveTexture::updateImage(nmb_Image *image) 
{
  d_graphicsDisplay->createRealignTextures(image->name()->Characters());
}
