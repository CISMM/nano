#if ((defined _WIN32) && !(defined __CYGWIN__))
#include <windows.h>
#else
#include <netinet/in.h>
#endif

#include "TIFFImage.h"


const short TAG_NewSubfileType            = 0x00FE; // 0
const short TAG_ImageWidth                = 0x0100; // X
const short TAG_ImageLength               = 0x0101; // Y
const short TAG_BitsPerSample             = 0x0102; // 8
const short TAG_Compression               = 0x0103; // 1
const short TAG_PhotometricInterpretation = 0x0106; // 2
const short TAG_StripOffsets              = 0x0111; // ... just one
const short TAG_SamplesPerPixel           = 0x0115; // C
const short TAG_RowsPerStrip              = 0x0116; // Y
const short TAG_StripByteCounts           = 0x0117; // X * Y * C
const short TAG_XResolution               = 0x011A; // ptr to double (600.0)
const short TAG_YResolution               = 0x011B; // ptr to double (600.0)
const short TAG_PlanarConfiguration       = 0x011C; // 1
const short TAG_ResolutionUnit            = 0x0128; // 2

//const short FMT_BYTE     = 1;
//const short FMT_ASCII    = 1;
const short FMT_SHORT    = 3;
const short FMT_LONG     = 4;
const short FMT_RATIONAL = 5;

const long TIFF_Stamp   = 0x4D4D002A;
const long TIFF_IFD     = 0x08;

const short TIFF_Entries = 0x0E;
const short TIFF_Pad     = 0x00;


// virtual
TIFFImage::~TIFFImage (void) {
  // Don't need this - AbstractImage does it.
  // Die();
}

bool TIFFImage::writeHeader(ofstream &tiff)
{
   double r = 600.0;

   int  x  = image->Columns(),
        y  = image->Rows(),
        c  = image->Colors(),
        l  = x * y * c,
       *dh = (int *)&r,
       *dl = (int *)(&r + 4),
        i  = 0;

   unsigned short header[200];
   
   
   // The following is very ugly.  It does, however, make the (IMHO,
   // quite stupid and broken) HP compiler happy.
   
   header[i++] = (short)(TIFF_Stamp>>16);
   header[i++] = (short)(TIFF_Stamp&0xffff);

   header[i++] = (short)(TIFF_IFD>>16);
   header[i++] = (short)(TIFF_IFD&0xffff);

   header[i++] = TIFF_Entries;

   header[i++] = TAG_NewSubfileType;
   header[i++] = FMT_LONG;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = 0;
   header[i++] = 0;

   header[i++] = TAG_ImageWidth;
   header[i++] = FMT_LONG;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = (short)(x>>16);
   header[i++] = (short)(x&0xffff);

   header[i++] = TAG_ImageLength;
   header[i++] = FMT_LONG;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = (short)(y>>16);
   header[i++] = (short)(y&0xffff);

   header[i++] = TAG_BitsPerSample;
   header[i++] = FMT_SHORT;
   header[i++] = 0;
   header[i++] = 3;
   header[i++] = 0;
   header[i++] = 192;

   header[i++] = TAG_Compression;
   header[i++] = FMT_SHORT;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = 1;
   header[i++] = TIFF_Pad;

   header[i++] = TAG_PhotometricInterpretation;
   header[i++] = FMT_SHORT;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = 2;
   header[i++] = TIFF_Pad;

   header[i++] = TAG_StripOffsets;
   header[i++] = FMT_LONG;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = 0;
   header[i++] = 200;

   header[i++] = TAG_SamplesPerPixel;
   header[i++] = FMT_SHORT;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = 3;
   header[i++] = TIFF_Pad;

   header[i++] = TAG_RowsPerStrip;
   header[i++] = FMT_LONG;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = (short)(y>>16);
   header[i++] = (short)(y&0xffff);

   header[i++] = TAG_StripByteCounts;
   header[i++] = FMT_LONG;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = (short)(l>>16);
   header[i++] = (short)(l&0xffff);

   header[i++] = TAG_XResolution;
   header[i++] = FMT_RATIONAL;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = 0;
   header[i++] = 186;

   header[i++] = TAG_YResolution;
   header[i++] = FMT_RATIONAL;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = 0;
   header[i++] = 186;

   header[i++] = TAG_PlanarConfiguration;
   header[i++] = FMT_SHORT;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = 1;
   header[i++] = TIFF_Pad;

   header[i++] = TAG_ResolutionUnit;
   header[i++] = FMT_SHORT;
   header[i++] = 0;
   header[i++] = 1;
   header[i++] = 2;
   header[i++] = TIFF_Pad;

   header[i++] = 0;
   header[i++] = 0;

   header[i++] = TIFF_Pad;

   header[i++] = (short)((*dh)>>16);
   header[i++] = (short)((*dh)&0xffff);

   header[i++] = (short)((*dl)>>16);
   header[i++] = (short)((*dl)&0xffff);

   header[i++] = 8;
   header[i++] = 8;
   header[i++] = 8;
   header[i++] = TIFF_Pad;

   for (i = 0; i < 100; i++)
	   header[i] = htons(header[i]);

   tiff.write((char *)header, 200);

   return !tiff.fail();
}

bool TIFFImage::Read(const char * /*filename*/ )
{
   cerr << "TIFF reading is not implemented!" << endl;

   return false;
}

bool TIFFImage::Write(const char *filename)
{
   if (!image || !image->Valid())
   {
      cerr << "Invalid image contents!" << endl;
      return false;
   }

   if (3 != image->Colors() && 1 != image->Colors())
   {
      cerr << "Do not know how to write " << image->Colors()
           << "-color TIFF image." << endl;
      return false;
   }


#ifdef _WIN32
   // Marked BINARY to try to fix problem on PCs
   ofstream tiff (filename, ios::binary | ios::out);
#else
   ofstream tiff (filename);
#endif


   if (!tiff)
   {
      cerr << "Error opening '" << filename << "' for writing." << endl;
      return false;
   }

   if (!writeHeader(tiff))
   {
      cerr << "Error writing header for '" << filename << "'." << endl;
      return false;
   }

   tiff.write(&(image->Pixel()),image->Rows()*image->Columns()*image->Colors());

   if (tiff.fail())
   {
      tiff.close();

      cerr << "Error writing '" << filename << "'." << endl;
      return false;
   }

   tiff.close();

   return true;
}
