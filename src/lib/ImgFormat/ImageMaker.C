#include "ImageMaker.h"


// This class allows images to be constructed at run-time in
// the format the user wants.
// To add a new type:
//    Increment ImageType_count
//    Add a string name to ImageType_names
//    Add a type enumeration to ImageType (in ImageMaker.h)
//    Add a case statement to each of the following functions



extern const int ImageType_count = 2;

extern const char *ImageType_names[] = { "TIFF", "PNM" };


AbstractImage *ImageMaker(const ImageType type)
{
   AbstractImage *ret;


   switch (type)
   {
      case TIFFImageType:
         ret = new TIFFImage;
         break;

      case PNMImageType:
         ret = new PNMImage;
         break;

      default:
         ret = NULL;
   }

   return ret;
}

AbstractImage *ImageMaker
(
   const ImageType      type,
   const int            y,
   const int            x,
   const short          c,
   const unsigned char *p,
   const bool           r
)
{
   AbstractImage *ret;


   switch (type)
   {
      case TIFFImageType:
         ret = new TIFFImage(y, x, c, p, r);
         break;

      case PNMImageType:
         ret = new PNMImage(y, x, c, p, r);
         break;

      default:
         ret = NULL;
   }

   return ret;
}

AbstractImage *ImageMaker
(
   const ImageType    type,
   const PixelBuffer &pb
)
{
   AbstractImage *ret;


   switch (type)
   {
      case TIFFImageType:
         ret = new TIFFImage(pb);
         break;

      case PNMImageType:
         ret = new PNMImage(pb);
         break;

      default:
         ret = NULL;
   }

   return ret;
}

AbstractImage *ImageMaker
(
   const ImageType      type,
   const AbstractImage &ai
)
{
   AbstractImage *ret;


   switch (type)
   {
      case TIFFImageType:
         ret = new TIFFImage(ai);
         break;

      case PNMImageType:
         ret = new PNMImage(ai);
         break;

      default:
         ret = NULL;
   }

   return ret;
}
