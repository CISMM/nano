/* The nanoManipulator and its source code have been released under the
 * Boost software license when nanoManipulator, Inc. ceased operations on
 * January 1, 2014.  At this point, the message below from 3rdTech (who
 * sublicensed from nanoManipulator, Inc.) was superceded.
 * Since that time, the code can be used according to the following
 * license.  Support for this system is now through the NIH/NIBIB
 * National Research Resource at cismm.org.

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef AbstractImage_Class
#define AbstractImage_Class


#include <string.h>
#include <stdlib.h>
#include <stdio.h> // for FILE

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

      //virtual bool Read(FILE *file) = 0;

      virtual bool Write(const char *filename) = 0;

      //virtual bool Write(FILE *file) = 0;
};


#endif
