#ifndef PNMImage_Class
#define PNMImage_Class

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

class PNMImage : public AbstractImage
{
   private:
      bool readInt
      (
         ifstream &pnm,
         int      &value
      );

      bool readUChar
      (
         ifstream      &pnm,
         unsigned char &value
      );

      int readHeader(ifstream &pnm);

      bool readASCII(ifstream &pnm);

      bool readRAW(ifstream &pnm);

      bool readPNM(ifstream &pnm);

      bool writeRAW(ofstream &pnm);


   public:
      virtual ~PNMImage (void);

      PNMImage(void) : AbstractImage() { }

      PNMImage
      (
         const int            y,
         const int            x,
         const short          c,
         const unsigned char *p = NULL,
         const bool           /*a */= false,
         const bool           r = false
      ) :
         AbstractImage(y, x, c, p, r)
      { }

      PNMImage(const PixelBuffer &p) : AbstractImage(p) { }

      PNMImage(const AbstractImage &ai) : AbstractImage(ai) { }

      bool Read(const char *filename);

      bool Read(FILE *file);

      bool Write(const char *filename);

      bool Write(FILE *file);

};


#endif
