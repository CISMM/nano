#ifndef ImageMaker_Class
#define ImageMaker_Class


#include "TIFFImage.h"
#include "PNMImage.h"


enum ImageType { TIFFImageType, PNMImageType };

extern const int ImageType_count;

extern const char *ImageType_names[];


AbstractImage *ImageMaker(const ImageType type);

AbstractImage *ImageMaker
(
   const ImageType      type,
   const int            y,
   const int            x,
   const short          c,
   const unsigned char *p = NULL,
   const bool           r = false
);

AbstractImage *ImageMaker
(
   const ImageType    type,
   const PixelBuffer &pb
);

AbstractImage *ImageMaker
(
   const ImageType      type,
   const AbstractImage &ai
);


#endif
