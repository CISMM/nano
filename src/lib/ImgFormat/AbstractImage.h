#ifndef AbstractImage_Class
#define AbstractImage_Class


#include <string.h>
#include <stdlib.h>

#include "PixelBuffer.h"

// Most of the calls here mirror the calls in PixelBuffer.
//
// To make a new image type, inherit this class, define the
// constructors and desctructors (have them call the ones in
// this class) and define Read() and Write().  Then add the
// new class to ImageMaker.


class AbstractImage
{
   protected:
      PixelBuffer *image;


      inline void Die(void)
      {
         if (image)
            delete image;

         image = NULL;
      }

      inline void Copy(const AbstractImage &ai)
      {
         image = new PixelBuffer(*(ai.image));
      }


   public:
      virtual ~AbstractImage (void);

      AbstractImage(void) :
         image(NULL)
      { }

      AbstractImage
      (
         const int            y,
         const int            x,
         const short          c,
         const unsigned char *p = NULL,
         const bool           r = false
      )
      {
         image = new PixelBuffer(y, x, c, p, r);
      }

      AbstractImage(const PixelBuffer &p) { image = new PixelBuffer(p); }

      AbstractImage(const AbstractImage &ai) { Copy(ai); }

      inline unsigned int Rows(void) { return image ? image->Rows() : 0; }

      inline unsigned int Columns(void) { return image ? image->Columns() : 0; }

      inline unsigned short Colors(void) { return image ? image->Colors() : 0; }

      inline bool Valid(void) { return image ? image->Valid() : false; }

      inline const PixelBuffer &Buffer(void) const { return *image; }

      inline void Clone(const AbstractImage &ai)
      {
         if (this != &ai)
         {
            Die();
            Copy(ai);
         }
      }

      inline void Clone(const PixelBuffer &pb)
      {
         Die();
         image = new PixelBuffer(pb);
      }

      inline AbstractImage &operator=(const AbstractImage &ai)
      {
         if (this != &ai)
         {
            Die();
            Copy(ai);
         }

         return *this;
      }

      inline unsigned char &Pixel
      (
         const int   y = 0,
         const int   x = 0,
         const short c = 0
      )
      {
         return image->Pixel(y, x, c);
      }

      inline const unsigned char &Pixel
      (
         const int   y = 0,
         const int   x = 0,
         const short c = 0
      ) const
      {
         return image->Pixel(y, x, c);
      }

      virtual bool Read(const char *filename) = 0;

      virtual bool Write(const char *filename) = 0;
};


#endif
