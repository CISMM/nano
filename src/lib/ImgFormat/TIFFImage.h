#ifndef TIFFImage_Class
#define TIFFImage_Class

#include <iostream.h>
#include <fstream.h>

#include "AbstractImage.h"


class TIFFImage : public AbstractImage
{
   private:
      bool writeHeader(ofstream &tiff);


   public:
      virtual ~TIFFImage(void) { Die(); }

      TIFFImage(void) : AbstractImage() { }

      TIFFImage
      (
         const int            y,
         const int            x,
         const int            c,
         const unsigned char *p = NULL,
         const bool           r = false
      ) :
         AbstractImage(y, x, c, p, r)
      { }

      TIFFImage(const PixelBuffer &p) : AbstractImage(p) { }

      TIFFImage(const AbstractImage &ai) : AbstractImage(ai) { }

      bool Read(const char *filename);

      bool Write(const char *filename);
};


#endif
