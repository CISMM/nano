#ifndef PNMImage_Class
#define PNMImage_Class

#include <iostream.h>
#include <fstream.h>

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

      bool Write(const char *filename);

};


#endif
