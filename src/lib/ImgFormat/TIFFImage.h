#ifndef TIFFImage_Class
#define TIFFImage_Class


// make the SGI compile without tons of warnings
#ifdef sgi
#pragma set woff 1110,1424,3201
#endif

#include <iostream>
#include <fstream>
using namespace std;

// and reset the warnings
#ifdef sgi
#pragma reset woff 1110,1424,3201
#endif

#include "AbstractImage.h"

class TIFFImage : public AbstractImage
{
   private:
      bool writeHeader(ofstream &tiff);


   public:
      virtual ~TIFFImage (void);

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

      bool Read(FILE *file);

      bool Write(const char *filename);
  
	  // Not available using standard iostream library.
      //bool Write(FILE *file);
};


#endif
