/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMB_IMAGE_H
#define NMB_IMAGE_H

#include <vrpn_Shared.h>
#include <string>
#include "BCPlane.h"
#include "BCGrid.h"
#include "nmb_String.h"
#include "nmb_TransformMatrix44.h"
#include <math.h>
#include "Topo.h"
#include <assert.h>

#if !(defined(__CYGWIN__) || defined(_WIN32))
#include <values.h>	// for MAXSHORT - probably different on windows
#else
// MAXSHORT taken care of by vrpn_Shared.h above - it's in a windows header.
//#include <limits.h>
//#ifndef MAXSHORT
//#define MAXSHORT SHRT_MAX
#endif

class nmb_Dataset;

// min and max also in Topo.h
#ifdef MAX
#undef MAX
#endif
#define MAX(x,y)        (((x)>(y))?(x):(y))

#ifdef MIN
#undef MIN
#endif
#define MIN(x,y)        (((x)<=(y))?(x):(y))

typedef enum {NMB_FLOAT32, NMB_UINT8, NMB_UINT16}  nmb_PixelType;

#define NMB_HEIGHT_UNITS_STR "nm"

class nmb_ImageBounds {
  public:
	nmb_ImageBounds();
	nmb_ImageBounds(double x0,double y0,double x1,double y1);
    /// the MIN/MAX distinction is based on how the pixels in the image
    /// are indexed (e.g.,the corner at the pixel with index (0,0) is
    /// referenced by MIN_X_MIN_Y; particular values are assigned so that
    /// the enum values can be used as indexes for the x,y arrays
	enum ImageBoundPoint {MIN_X_MIN_Y = 0, MIN_X_MAX_Y = 1, 
				MAX_X_MIN_Y = 2, MAX_X_MAX_Y = 3};

	void setX(ImageBoundPoint ibp, double xb) {x[ibp] = xb;}
	void setY(ImageBoundPoint ibp, double yb) {y[ibp] = yb;}
	double getX(ImageBoundPoint ibp) const { return x[ibp]; }
	double getY(ImageBoundPoint ibp) const { return y[ibp]; }
        double minX() const {return min(min(x[0], x[1]), min(x[2], x[3]));}
        double minY() const {return min(min(y[0], y[1]), min(y[2], y[3]));}
        double maxX() const {return max(max(x[0], x[1]), max(x[2], x[3]));}
        double maxY() const {return max(max(y[0], y[1]), max(y[2], y[3]));}
        double area() const {
           double a1 = (x[MAX_X_MIN_Y] - x[MIN_X_MIN_Y])*
                       (y[MIN_X_MAX_Y] - y[MIN_X_MIN_Y]) -
                       (x[MIN_X_MAX_Y] - x[MIN_X_MIN_Y])*
                       (y[MAX_X_MIN_Y] - y[MIN_X_MIN_Y]);
           double a2 = (x[MAX_X_MIN_Y] - x[MAX_X_MAX_Y])*
                       (y[MIN_X_MAX_Y] - y[MAX_X_MAX_Y]) -
                       (x[MIN_X_MAX_Y] - x[MAX_X_MAX_Y])*
                       (y[MAX_X_MIN_Y] - y[MAX_X_MAX_Y]);
           return 0.5*(fabs(a1) + fabs(a2));

        }

  private:
    double x[4], y[4];
};

class nmb_ImageList;
/**
	nmb_ImageList provides a way to unite images from different BCGrids
	into a single list. An nmb_Image base class and container class for
	BCPlane were added to allow code to work with data structures other
	than BCPlane more easily
*/
class nmb_Image {
        friend class nmb_ImageList;
  public:
	nmb_Image():num_referencing_lists(0),
                d_worldToImageMatrixSet(VRPN_FALSE),
                d_imagePosition(0.0, 0.0, 1.0, 1.0),
                d_units_scale(1.0), d_units_offset(0.0),
                d_DAC_scale(1.0), d_DAC_offset(0),
                d_dimensionUnknown(VRPN_TRUE)
        {
           for (int i = 0; i < 4; i++){
              for (int j = 0; j < 4; j++) {
                 d_worldToImageMatrix[i*4+j] = (i==j?1:0);
              }
           }
        };
        nmb_Image(nmb_Image *) {};
        static int deleteImage(nmb_Image *im);
	virtual int width() const = 0;
	virtual int height() const = 0;
	virtual float getValue(int i, int j) const = 0;
	virtual void setValue(int i, int j, float val) = 0;
        /// min and max of values actually in the image:
	virtual float maxValue() = 0;
	virtual float minValue() = 0;
        virtual int normalize() = 0;
        // min and max of values that have been set
        virtual float maxValidValue() = 0;
        virtual float minValidValue() = 0;
        virtual float maxNonZeroValue() = 0;
        virtual float minNonZeroValue() = 0;
        /// min and max representable values:
	virtual float minAttainableValue() const = 0;
        virtual float maxAttainableValue() const = 0;
	virtual int validDataRange(short* o_minX, short* o_maxX, 
                           short* o_minY, short* o_maxY) = 0;

	float getValueInterpolated(double i, double j) const;
	float getValueInterpolatedNZ(double i, double j) const;

        void getGradient(int i, int j, double &grad_x, double &grad_y);
        void getGradient(double i, double j, double &grad_x, double &grad_y);

	/// bounds of image in whatever units the image is in 
	/// (note: this is more general than the current BCGrid interface since
	///  it allows for rotation of the image coordinate system (in which we
	///  have image positions in terms of pixel (i,j) coordinates) relative
	///  to the world coordinate system)
	virtual double boundX(nmb_ImageBounds::ImageBoundPoint ibp) const = 0;
	virtual double boundY(nmb_ImageBounds::ImageBoundPoint ibp) const = 0;
	virtual void setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x) 
             = 0;
	virtual void setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y) 
             = 0;
	virtual void getBounds(nmb_ImageBounds &ib) const = 0;
	virtual void setBounds(const nmb_ImageBounds &ib) = 0;

	double widthWorld() const;
	double heightWorld() const;

        //void setWidthWorld(double width, double originX = 0.5);
        //void setHeightWorld(double height, double originY = 0.5);

        void setAcquisitionDimensions(double distX, double distY);
        void getAcquisitionDimensions(double &distX, double &distY) const;

	/// convert a position in an image given as a pixel location into a
	/// position in the world coordinate system for the image
	void pixelToWorld(const double i, const double j, 
 		  double &x, double &y) const;

	/// WARNING: assumes image axes are orthogonal in the world
	void worldToPixel(const double x, const double y,
			double &i, double &j) const;

        /**
         We have four different coordinate systems for an image:
         1) world coordinates - an external coordinate system used for 
              alignment between images
         2) image coordinates - a coordinate system with axes aligned to the
              image axes such that the accessible part of the image spans 
              exactly the unit square in x and y
         3) scaled image coordinates - like image coordinates but with the
              x and y axes scaled up to some measure of the real world distance
              spanned in x and y by the image - these distances are independent
              of the distances spanned by the image in "world" coordinates
              because they are independent of any alignment procedure
              This represents our knowledge of magnification and pixel
              aspect ratios which we have independent of any alignment
              procedure (based on get/setAcquisitionDimensions).
         4) texture coordinates - a coordinate system very similar to 
              image coordinates but with an additional scale and translation
              such that the full array used to store the image data
              spans exactly the unit square in x and y (the array used to
              store the image data actually has a non-accessible border
              on all four sides of the image to make the array 
              dimensions powers of 2)
        */

        void getWorldToImageTransform(double *matrix44) const;
        void getWorldToImageTransform(nmb_TransformMatrix44 &xform) const;
        void getImageToTextureTransform(double *matrix44) const;
        void getImageToTextureTransform(nmb_TransformMatrix44 &xform) const;
        void setWorldToImageTransform(double *matrix44); 
        void setWorldToImageTransform(nmb_TransformMatrix44 &xform);

        void getWorldToScaledImageTransform(double *matrix44) const;
        void getWorldToScaledImageTransform(nmb_TransformMatrix44 &xform) const;

        void getScaledImageToImageTransform(double *matrix44) const;
        void getScaledImageToImageTransform(nmb_TransformMatrix44 &xform) const;
        void getImageToScaledImageTransform(double *matrix44) const;
        void getImageToScaledImageTransform(nmb_TransformMatrix44 &xform) const;

        double areaInWorld() const;

        virtual void setTopoFileInfo(TopoFile &tf) = 0;
        virtual void getTopoFileInfo(TopoFile &tf) = 0;

	virtual string *name() = 0;
	virtual string *unitsX() = 0;
	virtual string *unitsY() = 0;
	virtual string *unitsValue() = 0;

        /// tells you how to convert values returned by getValue() into
        /// the units returned by unitsValue()
        virtual double valueOffset() {return d_units_offset;}
        virtual double valueScale() {return d_units_scale;}

        /// tells you how to convert the values returned by getValue() into
        /// DAC units
        virtual double valueOffsetDAC() {return d_DAC_offset;}
        virtual double valueScaleDAC() {return d_DAC_scale;}

	/// gives address of an array of pixels in the order
        /// row0, row1, row2, ... row<height-1>
	virtual void *pixelData() = 0;

        /// gives the border widths for data returned by pixelData
        virtual int borderXMin() const = 0;
        virtual int borderXMax() const = 0;
        virtual int borderYMin() const = 0;
        virtual int borderYMax() const = 0;

        /// tells you the size of the array returned by pixelData()
        virtual int arrayLength() = 0;

        /// tells you what type of data is returned by pixelData
        virtual nmb_PixelType pixelType() = 0;

	virtual int numExportFormats() = 0;
	virtual const char *exportFormatType(int type) = 0;
	virtual nmb_ListOfStrings *exportFormatNames() = 0;
	virtual int exportToFile(FILE *f, const char *export_type,
                                 const char * filename) = 0;

        /// this is to give us some clue about whether or not we know anything
        /// about the dimensions of the image in the world
        virtual vrpn_bool dimensionUnknown();

  protected:
        virtual ~nmb_Image (void);
        int num_referencing_lists;

        /// has d_worldToImageMatrix been set?
        vrpn_bool d_worldToImageMatrixSet;
        /// the transformation matrix returned by getWorldToImageTransform
        /// if its been set through setWorldToImageTransform
        double d_worldToImageMatrix[16];
        /// position of the corners of the image in the world
        nmb_ImageBounds d_imagePosition;

        /// dimensions of the image in the coordinate system in which it was 
        /// acquired regardless of what transformation or world position has
        /// been set (the main reason this was introduced was so that rotations 
        /// would be performed in a reasonably-scaled space)
        /// ----
        /// At the least, these distances should be proportional to the 
        /// actual distances spanned in x and y (so if the aspect ratio for
        /// pixels is 1:1 then they may be equal to the resolution of the image)
        /// but in the future we may want to assume they are in nanometers or
        /// some real units like that
        double d_acquisitionDistX;
        double d_acquisitionDistY;
        
        /// (value in specified units) = 
        ///               (array value)*d_units_scale+d_units_offset
        double d_units_scale;
        double d_units_offset;

        /// (value in DAC units) = (array value)/d_DAC_scale - d_DAC_offset
        double d_DAC_scale;  // should have same meaning as BCPlane::tm_scale
        double d_DAC_offset; // should have same meaning as BCPlane::tm_offset 
        
        /// this is to give us some clue about whether or not we know anything
        /// about the dimensions of the image in the world
        vrpn_bool d_dimensionUnknown;
};

/// container class for BCGrid/BCPlane-based images
class nmb_ImageGrid : public nmb_Image{
    friend class nmb_Dataset;
  public:
	nmb_ImageGrid(const char *name, const char *units, short x, short y);
        // makes a wrapper object which is responsible for deallocating 
        // the grid and plane
        nmb_ImageGrid(BCGrid *g);
        // makes a wrapper object which doesn't do any deallocation of the
        // plane or grid
	nmb_ImageGrid(BCPlane *p);
        nmb_ImageGrid(nmb_Image *);
	virtual int width() const;
	virtual int height() const;
	virtual float getValue(int i, int j) const;
	virtual void setValue(int i, int j, float val);
        virtual int validDataRange(short* o_minX, short* o_maxX, 
                           short* o_minY, short* o_maxY);
	virtual float maxValue();
	virtual float minValue();
        virtual int normalize();
        virtual float maxValidValue();
        virtual float minValidValue();
        virtual float maxNonZeroValue();
        virtual float minNonZeroValue();
        virtual float minAttainableValue() const;
        virtual float maxAttainableValue() const;
	virtual double boundX(nmb_ImageBounds::ImageBoundPoint ibp) const;
	virtual double boundY(nmb_ImageBounds::ImageBoundPoint ibp) const;
	virtual void setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x);
	virtual void setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y);
	virtual void getBounds(nmb_ImageBounds &ib)  const;
	virtual void setBounds(const nmb_ImageBounds &ib);
	virtual string *name();
	virtual string *unitsValue();
	virtual string *unitsX();
	virtual string *unitsY();
        /// Updates correctly if plane receives new thermo header
        /// during live or streamfile replay. 
        virtual double valueOffsetDAC() {return plane->tm_offset;}
        /// Updates correctly if plane receives new thermo header
        /// during live or streamfile replay. 
        virtual double valueScaleDAC() {return plane->tm_scale;}
        virtual void setTopoFileInfo(TopoFile &tf);
        virtual void getTopoFileInfo(TopoFile &tf);
        virtual void *pixelData();
        virtual int borderXMin() const;
        virtual int borderXMax() const;
        virtual int borderYMin() const;
        virtual int borderYMax() const;
        virtual int arrayLength();
	virtual nmb_PixelType pixelType();
        virtual int numExportFormats();
	virtual nmb_ListOfStrings *exportFormatNames();
        virtual const char *exportFormatType(int type); 
        virtual int exportToFile(FILE *f, const char *export_type, 
                                 const char * filename);
        BCPlane *getPlane();

	typedef int (*FileExportingFunction) (FILE *file, nmb_ImageGrid *im, const char * filename);

        /// functions for reading one or more images out of a file
        /// these are in this class so we can leverage the code in BCGrid
        /// for reading topometrix files
        static int openFile(const char *filename);
        static nmb_ImageGrid *getNextImage();

  protected:
        virtual ~nmb_ImageGrid();
	static const int     num_export_formats;
        static const char    *export_formats_list[];
	nmb_ListOfStrings formatNames;
	static const FileExportingFunction file_exporting_function[];
        static int writeTopoFile(FILE *file, nmb_ImageGrid *im, const char * filename);
	static int writeTextFile(FILE *file, nmb_ImageGrid *im, const char * filename);
        static int writePPMFile(FILE *file, nmb_ImageGrid *im, const char * filename);
        static int writeTIFFile(FILE *file, nmb_ImageGrid *im, const char * filename);
        static int writeOtherImageFile(FILE *file, nmb_ImageGrid *im, const char * filename);
        static int writeSPIPFile(FILE *file, nmb_ImageGrid *im, const char * filename);
	static int writeUNCAFile(FILE *file, nmb_ImageGrid *im, const char * filename);

	BCPlane *plane;
	BCGrid *grid;  ///< this is NULL if we are not the allocator of
			///< the grid (important for destructor)
	string units_x;
	string units_y;
        short min_x_set, min_y_set, max_x_set, max_y_set;

        /// the topo file information for this particular image
        TopoFile d_topoFileDefaults;

        /// the topo file information for the last opened file
        static TopoFile s_openFileTopoHeader;
        static BCPlane *s_openFilePlane;
        static BCGrid *s_openFileGrid;
        
        vrpn_bool d_imagePositionSet;
};

/* This was originally written as a template with variable type for the pixel
   data but something about this mixture of templates and inheritance seems
   to cause the sgi CC to do something really nasty. -- AAS
*/

class nmb_ImageArray : public nmb_Image {
  public:
    nmb_ImageArray(const char *name, const char *units, short x, short y,
        nmb_PixelType pixType = NMB_FLOAT32);
    nmb_ImageArray(nmb_Image *);
    virtual int width() const;
    virtual int height() const;

    virtual float getValue(int i, int j) const;
    virtual void setValue(int i, int j, float val);
    /// min and max of values actually in the image:
    virtual float maxValue();
    virtual float minValue();
    virtual int normalize();
    // min and max of values that have been set
    virtual float maxValidValue();
    virtual float minValidValue();
    virtual float maxNonZeroValue();
    virtual float minNonZeroValue();
    /// min and max representable values:
    virtual float minAttainableValue() const;
    virtual float maxAttainableValue() const;
    virtual int validDataRange(short* o_minX, short* o_maxX, 
                           short* o_minY, short* o_maxY);

    virtual double boundX(nmb_ImageBounds::ImageBoundPoint ibp) const;
    virtual double boundY(nmb_ImageBounds::ImageBoundPoint ibp) const;
    virtual void setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x);
    virtual void setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y);
    virtual void getBounds(nmb_ImageBounds &ib) const;
    virtual void setBounds(const nmb_ImageBounds &ib);

    virtual void setTopoFileInfo(TopoFile &) {
      fprintf(stderr,
          "Warning: nmb_ImageArray::setTopoFileInfo not implemented\n");
    }
    virtual void getTopoFileInfo(TopoFile &) {
      fprintf(stderr,
          "Warning: nmb_ImageArray::getTopoFileInfo not implemented\n");
    }

    virtual string *name();
    virtual string *unitsX();
    virtual string *unitsY();
    virtual string *unitsValue();

    /// gives address of an array of pixels in the order
    /// row0, row1, row2, ... row<height-1>
    virtual void *pixelData();

    /// gives the border width for data returned by pixelData
    virtual int borderXMin() const;
    virtual int borderXMax() const;
    virtual int borderYMin() const;
    virtual int borderYMax() const;

    /// tells you the size of the array returned by pixelData()
    virtual int arrayLength();

    /// tells you what type of data is returned by pixelData
    virtual nmb_PixelType pixelType();

    virtual int numExportFormats();
    virtual const char *exportFormatType(int type);
    virtual nmb_ListOfStrings *exportFormatNames();
    virtual int exportToFile(FILE *f, const char *export_type, 
                             const char * filename);

    virtual void setLine(int line, void *line_data);
    virtual void setImage(void *newdata);

    typedef int (*FileExportingFunction) (FILE *file, nmb_ImageArray *im, const char * filename);
  protected:
    virtual ~nmb_ImageArray();
    static int exportToTIFF(FILE *file, nmb_ImageArray *im, const char *);


    int arrIndex(int i, int j) const
      { return (i+d_borderXMin+(j+d_borderYMin)*
                               (num_x+d_borderXMin+d_borderXMax));}
    int pixelSize()
    {
      switch(d_pixelType) {
        case NMB_FLOAT32:
          return sizeof(vrpn_float32);
        case NMB_UINT8:
          return sizeof(vrpn_uint8);
        case NMB_UINT16:
          return sizeof(vrpn_uint16);
        default:
          return 0;
      }
    }

    vrpn_float32 *fData;
    vrpn_uint8 *ucData;
    vrpn_uint16 *usData;

    void *data;
    short num_x, num_y;
    short d_borderXMin, d_borderXMax, d_borderYMin, d_borderYMax;
    string units_x, units_y, units, my_name;
    short min_x_set, min_y_set, max_x_set, max_y_set;

    static const int     num_export_formats;
    static const char    *export_formats_list[];
    nmb_ListOfStrings formatNames;
    static const FileExportingFunction file_exporting_function[];

    vrpn_bool d_minNonZeroValueComputed;
    float d_minNonZeroValue;
    vrpn_bool d_minValueComputed;
    float d_minValue;
    vrpn_bool d_maxValueComputed;
    float d_maxValue;
    vrpn_bool d_maxValidValueComputed;
    float d_maxValidValue;
    vrpn_bool d_minValidValueComputed;
    float d_minValidValue;

    nmb_PixelType d_pixelType;
};

#define NMB_MAX_IMAGELIST_LENGTH 100

class nmb_ImageList {
  public:
	nmb_ImageList(nmb_ListOfStrings *namelist);
        ~nmb_ImageList();
        void addFileImages(const char **file_names, int num_files,
                             TopoFile &topoFile);
	int addImage(nmb_Image *im);
	nmb_ListOfStrings *imageNameList() {return imageNames;}
	nmb_Image *getImageByName(string name) {
		int i;
		return getImageByName(name, i);
	}
        nmb_Image* getImageByPlane( BCPlane* plane)
        {
          return getImageByName( *(plane->name( )) ); 
        }
	nmb_Image *removeImageByName(string name);
        int numImages() {
            return num_images;
        }
        nmb_Image *getImage(int index) {
            return images[index];
        }

  private:
	nmb_Image *getImageByName(string name, int &index);

	int num_images;
        nmb_Image *images[NMB_MAX_IMAGELIST_LENGTH];
	nmb_ListOfStrings *imageNames;
};


#endif
