// For use with /usr/Image/5.0
#define D_MATH
#define D_IOSTREAM
#define D_maxim
#define D_usrimage
#define D_ImagFile
#define D_STDIO
#define D_DebugBuffer
#define D_String
#define D_header
#define D_ImgStream
#define D_Image
#define D_sitedict
#define D_cores
#define D_CORPRELUD
#define D_Spline
#define D_Spline3
#define D_Eigen
#define D_GrayImage
#define D_RealImage
#define D_ByteImage
#define D_IntensRange
#define D_DimReduce
#define D_GraphUtility
#define D_main

#include <imprelud.h>

#include <BCGrid.h>
#include <BCPlane.h>

RealImage ReadNanoFile(char *image_file_name)
{
   int i, j;

   // Read the Grid and pack its first frame into a RealImage.
   char *filenames[1];
   filenames[0] = image_file_name;

   BCGrid *grid = new BCGrid(512,512, 0,0, 1,1, 1, READ_FILE, filenames,1);
   BCPlane *plane = grid->head();

   printf("Using plane %s\n",plane->name()->c_str());
   RealImage tempim (grid->numX(), grid->numY());

   for (i = 0; i < grid->numX(); i++)
   {
      for (j = 0; j < grid->numY(); j++)
      {
         tempim(i,j) = plane->value(i,j);
      }
   }

   return tempim;
}

//**********************************************************************

main (int argc, char** argv)
{
   if (argc != 3)
   {
      cerr << "Usage: micro2Im inputfilename outputfilename" << endl;
      exit (1);
   }
   else
   {
      // output image.
      RealImage im;

      // Load the input image.
      im = ReadNanoFile (argv[1]);

      ImgStream rstr(argv[2], CREATE);
      IntensRange ir(im);
      rstr.set_range(ir);
      rstr.write_image(im);
   }
}

