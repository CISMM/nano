#ifndef PixelBuffer_Class
#define PixelBuffer_Class


#include <stdlib.h>
#include <string.h>


#ifndef bool
#define bool   char
#define true   1
#define false  0
#endif


class PixelBuffer
{
   protected:
      unsigned char *pixels;

      unsigned int nRows, nColumns;

      unsigned short nColors;

      bool valid;


      inline void Die(void)
      {
         if (pixels)
            delete [] pixels;

         nRows = nColumns = nColors = 0;

         pixels = NULL;

         valid = false;
      }

      inline void Copy(const PixelBuffer &p)
      {
         if ((valid = p.valid))
         {
            nRows = p.nRows;
            nColumns = p.nColumns;
            nColors = p.nColors;

            pixels = new unsigned char[nRows * nColumns * nColors];
            memcpy(pixels, p.pixels, nRows * nColumns * nColors);
         }
      }


   public:
      ~PixelBuffer(void) { Die(); }

      PixelBuffer(void) :
         pixels(NULL),
         nRows(0),
         nColumns(0),
         nColors(0),
         valid(false)
      { }

      PixelBuffer
      (
         const int            y,                // Image height
         const int            x,                // Image width
         const short          c,                // Image colors
         const unsigned char *p = NULL,         // Array of pixels to copy
         const bool           reverse = false   // Rows are in reverse order
         // Note: If you are grabbing the GL frame buffer, you'll need to use
         // align = true and reverse = true when you make a this call
      ) :
         nRows(y),
         nColumns(x),
         nColors(c),
         valid(true)
      {
         pixels = new unsigned char[y*x*c];

         if (p)
         {
            if (!reverse)
               memcpy(pixels, p, y*x*c);
            else
               for (unsigned int i = 0; i < nRows; i++)
                  memcpy(pixels + (y-1-i)*x*c, p + i*x*c, x*c);
         }
         else
            memset(pixels, 0, nRows * nColumns * nColors);
      }

      PixelBuffer(const PixelBuffer &p) { Copy(p); }

      inline unsigned int Rows(void) { return nRows; }

      inline unsigned int Columns(void) { return nColumns; }

      inline unsigned short Colors(void) { return nColors; }

      inline bool Valid(void) { return valid; }

      // This gives direct access to the data in the pixel array,
      // that's bad. But doing so makes this class fast, that's good.
      inline unsigned char &Pixel
      (
         const int   y = 0,
         const int   x = 0,
         const short c = 0
      )
      {
         return pixels[c + x*nColors + y*nColors*nColumns];
      }

      // This gives direct access to the data in the pixel array,
      // that's bad. But doing so makes this class fast, that's good.
      inline const unsigned char &Pixel
      (
         const int   y = 0,
         const int   x = 0,
         const short c = 0
      ) const
      {
         return pixels[c + x*nColors + y*nColors*nColumns];
      }

      // This gives a bilinearly interpolated value from the pixel array,
      // input coordinates should be in the range [0..1) but x,y values will
      // be clamped from above
      inline unsigned char interpPixel
      (
         const double   y = 0.0,
         const double   x = 0.0,
         const short c = 0
      )
      {
	 unsigned char c00,c01,c10,c11;
	 unsigned char result;
	 double y_r = y*nRows, x_r = x*nColumns; // scale by image size
	 unsigned int x0_i = (unsigned int)x_r, y0_i = (unsigned int)y_r;	// truncate
	 unsigned int x1_i = x0_i+1, y1_i = y0_i+1;
	 double a,b;
	 if (x0_i > nColumns-1 || x1_i > nColumns-1) {
	     x0_i = nColumns-1;
	     x1_i = nColumns-1;
	     a = 1.0;
	 }
	 else a = (double)x1_i - x_r;
	 if (y0_i > nRows-1 || y1_i > nRows-1) {
	     y0_i = nRows-1;
	     y1_i = nRows-1;
	     b = 1.0;
	 }
	 else b = (double)y1_i - y_r;

	 c00 = pixels[c + x0_i*nColors + y0_i*nColors*nColumns];
	 c01 = pixels[c + x0_i*nColors + y1_i*nColors*nColumns];
	 c10 = pixels[c + x1_i*nColors + y0_i*nColors*nColumns];
	 c11 = pixels[c + x1_i*nColors + y1_i*nColors*nColumns];
	 result = (unsigned char)(c00*a*b + c01*a*(1-b) + 
			c10*(1-a)*b + c11*(1-a)*(1-b));
	 return result;
      }


      inline PixelBuffer &operator=(const PixelBuffer &p)
      {
         if (this != &p)
         {
            Die();
            Copy(p);
         }

         return *this;
      }
};


#endif
