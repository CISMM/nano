#ifndef NMB_IMG_MAGICK_H
#define NMB_IMG_MAGICK_H
/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
class BCGrid;
class nmb_Image;

class nmb_ImgMagick {
public:
    static void initMagick(char * argv);

    /// Read an image file using the ImageMagick library's routines. 
    static int readFileMagick(const char * filename, const char * name, 
                              BCGrid * grid);
    /// Write an image file using the ImageMagick library's routines. 
    static int writeFileMagick(const char * filename, 
                               const char * mgk_filetype, 
                               int cols, int rows, 
                               int bpp, unsigned char * pixels);
    /// Write an image file using the ImageMagick library's routines, from /
    /// data in a BCPlane object.
    static int writeFileMagick(const char * filename, 
                               const char * mgk_filetype, 
                               BCPlane * plane);

    /// Write an image file using the ImageMagick library's routines, from /
    /// data in an nmb_Image object
    static int writeFileMagick(const char * filename,
                               const char * mgk_filetype,
                               nmb_Image * image);
private:
    ///Static object, should never be constructed
    nmb_ImgMagick ();
};

#endif
