/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <magick/api.h>

#include "BCGrid.h"
#include "BCPlane.h"
#include "nmb_Image.h"
#include "nmb_ImgMagick.h"

/** Initialize ImageMagick library. Pass in argv[0] */
void nmb_ImgMagick::initMagick(char * argv) {
    MagickIncarnate(argv);
}

/**
   Read an arbitrary image file using the ImageMagick library, and put the
   image data into a new BCPlane in the BCGrid provided.
@param filename full path to the file
@param name desired name for BCPlane we add to grid
@param grid BCGrid to add BCPlane to. 
@return -1 on error, 0 on success. 
*/
int nmb_ImgMagick::readFileMagick(const char * filename, const char * name, BCGrid * grid) {
    ExceptionInfo
        exception;

      Image
          *image;

      ImageInfo
        *image_info;

      PixelPacket * pixels;
      ViewInfo * vinfo;

      //Initialize the image info structure and read an image.
      GetExceptionInfo(&exception);
      image_info=CloneImageInfo((ImageInfo *) NULL);
      (void) strcpy(image_info->filename,filename);
      image=ReadImage(image_info,&exception);
      if (image == (Image *) NULL) {
          // print out something to let us know we are missing the 
          // delegates.mgk or whatever if that is the problem instead of just
          // saying the file can't be loaded later
          fprintf(stderr, "nmb_ImgMagic: %s: %s\n",
                 exception.reason,exception.description);
          //MagickError(exception.severity,exception.reason,exception.description);
          // Get here if we can't decipher the file, let caller handle it. 
          return -1;
      }

      // Set the grid size to accomodate this new image. 
      grid->setGridSize(image->columns,image->rows);

      // This is the method for reading pixels that compiles and works, 
      // as opposed to GetImagePixels or GetOnePixel, which wouldn't compile. 
      vinfo = OpenCacheView(image);
      pixels = GetCacheView(vinfo, 0,0,image->columns,image->rows);
      if(!pixels) {
          fprintf(stderr, "readFileMagick: unable to get pixel cache.\n"); 
          return(-1);
      }

      BCPlane * plane = grid->addNewPlane(name, "nm", NOT_TIMED);
      if (!plane) {
          fprintf(stderr, "readFileMagick: null plane, memory error.\n"); 
          return(-1);
      }
      for (unsigned int j=0; j<image->rows; j++) {
          for (unsigned int i=0; i < image->columns; i++) {
              //  data ranges 0 to 256
              // Swap data vertically, to match NM convention.
              plane->setValue(i,image->rows-1-j,pixels[i + image->columns*j].red);
          }
      }
      CloseCacheView(vinfo);
      //printf("%d %d %d\n", pixels[0].red, pixels[0].green, pixels[0].blue);
      DestroyImageInfo(image_info);
      DestroyImage(image);
      return(0);
}

int nmb_ImgMagick::writeFileMagick(const char * filename, 
				   const char * /*mgk_filetype*/, 
				   int cols, int rows, 
				   int /*bpp*/, unsigned char * pixels)
{
    ExceptionInfo
        exception;
    
    Image
        *image,
        *flip_image;

    ImageInfo
      *image_info;

    //Initialize the image with provided data. 
    GetExceptionInfo(&exception);
    image=ConstituteImage(cols, rows, "RGB", (StorageType)0, 
                          pixels, &exception);
    if (image == (Image *) NULL) {
        fprintf(stderr, "writeFileMagick: Can't create image.\n");
        // Get here if we can't create the image, let caller handle it. 
        return -1;
    }

    // Image is flipped vertically, with data coming from frame buffer. 
    flip_image = FlipImage(image, &exception);
    DestroyImage(image);
    if (!flip_image) {
        fprintf(stderr, 
            "writeFileMagick: Memory error, can't create image.\n");
        return -1;
    }
    image_info=CloneImageInfo((ImageInfo *) NULL);
    // Zip is the only one that works (well) for TIF files, but some programs
    // don't read it. 
    // LZW is not even available. 
    image_info->compression=NoCompression;
    //SetImageInfo(image_info,true,&exception);
    strcpy(flip_image->filename,filename);
    if(!WriteImage(image_info, flip_image)) {
        return -1;
    }
    DestroyImageInfo(image_info);
    DestroyImage(flip_image);
    return(0);
}

int nmb_ImgMagick::writeFileMagick(const char * filename, 
                    const char * mgk_filetype, BCPlane * plane)
{
    ExceptionInfo
        exception;
    
    Image
          *image;

    ImageInfo
        *image_info;

    int w = plane->numX(), h =plane->numY();
    unsigned char * pixels = new unsigned char[3*w*h];

    if (!pixels) {
        return -1;
    }
    double scale = 254.0 / (plane->maxNonZeroValue() - 
                            plane->minNonZeroValue());
      
    unsigned int val;
    int x, y;
      
    for(y = 0; y <h ; y++ ) {
        for(x = 0; x < w; x++ ) {
            // Flip access of data vertically, 
            // so data looks upright in external viewers. 
            if (plane->value(x,h-1-y) < plane->minNonZeroValue()) {
                val = 0;
            } else {
                val = 1 + 
                    (unsigned)((plane->value(x, h-1-y) - 
                                plane->minNonZeroValue()) * scale);
            }
            pixels[3*y*w + 3*x] = 
                pixels[3*y*w + 3*x + 1] = 
                pixels[3*y*w + 3*x + 2] = val;
        }
    }
     
    //Initialize the image with provided data. 
    GetExceptionInfo(&exception);
    image=ConstituteImage(w, h, "RGB", (StorageType)0, pixels, &exception);
    if (image == (Image *) NULL) {
        fprintf(stderr, "writeFileMagick: Can't create image.\n");
        // Get here if we can't create the image, let caller handle it. 
        return -1;
    }

    image_info=CloneImageInfo((ImageInfo *) NULL);
    if ((mgk_filetype == NULL) || (mgk_filetype[0] == '\0')){
        strcpy(image->filename,filename);
    } else {
        sprintf(image->filename, "%s:%s", mgk_filetype, filename);
    }
    // Some experimental parameters. 
    image_info->colorspace=RGBColorspace;
    // 75 default for JPEG, PNG; increase for better quality. JPEG manual
    // says 100 is overkill, 95 max.
    image_info->quality=90;
    //Default is NoCompression. BZipCompression, JPEGCompression,
    // LosslessJPEGCompression, LZWCompression, RunlengthEncodedCompression
    // ZipCompression
    // Zip is the only one that works (well) for TIF files, but some programs
    // don't read it. 
    // LZW is not even available. 
    image_info->compression=NoCompression;

    if(!WriteImage(image_info, image)) {
        return -1;
    }
    DestroyImageInfo(image_info);
    DestroyImage(image);
    return(0);
}

int nmb_ImgMagick::writeFileMagick(const char * filename,
                    const char * mgk_filetype, nmb_Image *data)
{
    ExceptionInfo
        exception;

    Image
          *image;

    ImageInfo
        *image_info;

    int w = data->width(), h = data->height();
    unsigned char * pixels = new unsigned char[3*w*h];

    if (!pixels) {
        return -1;
    }
    double scale = 254.0 / (data->maxNonZeroValue() -
                            data->minNonZeroValue());

    unsigned int val;
    int x, y;

    for(y = 0; y <h ; y++ ) {
        for(x = 0; x < w; x++ ) {
            // Flip access of data vertically,
            // so data looks upright in external viewers.
            if (data->getValue(x,h-1-y) < data->minNonZeroValue()) {
                val = 0;
            } else {
                val = 1 +
                    (unsigned)((data->getValue(x, h-1-y) -
                                data->minNonZeroValue()) * scale);
            }
            pixels[3*y*w + 3*x] =
                pixels[3*y*w + 3*x + 1] =
                pixels[3*y*w + 3*x + 2] = val;
        }
    }

    //Initialize the image with provided data.
    GetExceptionInfo(&exception);
    image=ConstituteImage(w, h, "RGB", (StorageType)0, pixels, &exception);
    if (image == (Image *) NULL) {
        fprintf(stderr, "writeFileMagick: Can't create image.\n");
        // Get here if we can't create the image, let caller handle it.
        return -1;
    }

    image_info=CloneImageInfo((ImageInfo *) NULL);
    if ((mgk_filetype == NULL) || (mgk_filetype[0] == '\0')){
        strcpy(image->filename,filename);
    } else {
        sprintf(image->filename, "%s:%s", mgk_filetype, filename);
    }
    // Some experimental parameters.
    image_info->colorspace=RGBColorspace;
    // 75 default for JPEG, PNG; increase for better quality. JPEG manual
    // says 100 is overkill, 95 max.
    image_info->quality=90;
    //Default is NoCompression. BZipCompression, JPEGCompression,
    // LosslessJPEGCompression, LZWCompression, RunlengthEncodedCompression
    // ZipCompression
    // Zip is the only one that works (well) for TIF files, but some programs
    // don't read it. 
    // LZW is not even available. 
    image_info->compression=NoCompression;

    if(!WriteImage(image_info, image)) {
        return -1;
    }
    DestroyImageInfo(image_info);
    DestroyImage(image);
    return(0);
}
