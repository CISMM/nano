/*----------------------------------------------------------------------------
 CoreViewImage.h
 11/09/98
 Mark Foskey
 Image class for CoreView program.
----------------------------------------------------------------------------*/

#ifndef COREVIEWIMAGE_H_HWRAP_UNC_1998
#define COREVIEWIMAGE_H_HWRAP_UNC_1998

#define D_RealImage

#include <imprelud.h>

#include "cimage.h"

typedef const RealImage& RealImageRef;


// Class CoreViewImage.
class CoreViewImage {
public:
    
    CoreViewImage();
    CoreViewImage(RealImage);

    int     read_from_file(String file);
    void    invert();
    void    blur(float gaussian_sigma);
    void    zoomed_copy_of(const CoreViewImage& cvim, float zoom_factor);

    void    to_cimage(cimage cim) const;
    void    to_pgm(unsigned char* pgmim) const;

    unsigned   xdim() const { return _im.shape()[0]; }
    unsigned   ydim() const { return _im.shape()[1]; }

    const RealImage& im() const { return _im; }
    double  min_intensity() const { return _min_intensity; }
    double  max_intensity() const { return _max_intensity; }
    double  x_nm_perpixel() const { return _x_nm_perpixel; }
    double  y_nm_perpixel() const { return _y_nm_perpixel; }

    // For debugging purposes.  Feel free to remove.
    void    display_yourself() const;

private:
    RealImage _im;
    double _min_intensity;
    double _max_intensity;
    double _x_nm_perpixel; // x and y scales in nanometers per pixel
    double _y_nm_perpixel;

    void minmax();
};

#endif // COREVIEWIMAGE_H_HWRAP_UNC_1998
