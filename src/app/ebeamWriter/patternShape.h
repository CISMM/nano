#ifndef PATTERNSHAPE_H
#define PATTERNSHAPE_H

#include "nmb_TransformMatrix44.h"
#include <list>
#include "vrpn_Types.h"

using namespace std;

/* note on goToPoint usage:

     in an ideal world where all the other code runs infinitely fast we would
     expect this function to return after dwellTime_nsec nanoseconds but
     in reality what we do is initialize the SEM before starting to draw in
     order to compensate for the time required to execute code in between
     calls to this function - the overhead delay is computed by performing
     the draw operation with the SEM in a special mode in which it doesn't
     expose the surface and always assumes a 0 nsec dwell time
     The actual time for executing this function should be about
     the dwell time minus the overhead delay per point

*/

class nmm_Microscope_SEM_Remote;
class nmm_Microscope_SEM_EDAX;

class PatternPoint {
  public:
   PatternPoint(double x=0, double y=0 ): d_x(x), d_y(y) {};
   int operator== (const PatternPoint &ppt) {
        return (d_x == ppt.d_x && d_y == ppt.d_y);}

   double d_x, d_y;
};

typedef enum {PS_COLOR_BY_EXPOSURE} ColorMode;

class PatternShapeColorMap {
  public:
    PatternShapeColorMap();
    void setExposureLevels(list<double> &lineExposureLevels,
                           list<double> &areaExposureLevels)
       { d_lineExposureLevels = lineExposureLevels;
         d_areaExposureLevels = areaExposureLevels;
         d_lineExposureLevels.sort(); d_lineExposureLevels.unique();
         d_areaExposureLevels.sort(); d_areaExposureLevels.unique();
         d_numLineExposureLevels = d_lineExposureLevels.size();
         d_numAreaExposureLevels = d_areaExposureLevels.size();}
    void setMinLineExposureColor(double r, double g, double b)
       { d_minLineExposureColor[0] = r;
         d_minLineExposureColor[1] = g;
         d_minLineExposureColor[2] = b;
       }
    void setMinAreaExposureColor(double r, double g, double b)
       { d_minAreaExposureColor[0] = r;
         d_minAreaExposureColor[1] = g;
         d_minAreaExposureColor[2] = b;
       }
    void setMaxLineExposureColor(double r, double g, double b)
       { d_maxLineExposureColor[0] = r;
         d_maxLineExposureColor[1] = g;
         d_maxLineExposureColor[2] = b;
       }
    void setMaxAreaExposureColor(double r, double g, double b)
       { d_maxAreaExposureColor[0] = r;
         d_maxAreaExposureColor[1] = g;
         d_maxAreaExposureColor[2] = b;
       }
    void linearExposureColor(double exposure, double *color);
    void areaExposureColor(double exposure, double *color);

    ColorMode getColorMode() {return d_colorMode;}
    void draw(double x, double y,
              double units_per_pixel_x, double units_per_pixel_y);

  protected:
    static void exposureColor(double exposure, double *color,
                       list<double> &levels, int numLevels,
                       double *minColor, double *maxColor);
    list<double> d_lineExposureLevels;
    list<double> d_areaExposureLevels;
    double d_minLineExposureColor[3];
    double d_maxLineExposureColor[3];
    double d_minAreaExposureColor[3];
    double d_maxAreaExposureColor[3];
    int d_numLineExposureLevels;
    int d_numAreaExposureLevels;
    ColorMode d_colorMode;
};

typedef enum {PS_POLYLINE = 0, PS_POLYGON = 1, 
              PS_COMPOSITE = 2, PS_DUMP = 3, PS_DEFAULT = 4} ShapeType;
#define PS_NUMSHAPETYPES (5)

// base class
class PatternShape {
  friend class PatternFile;
  friend class PatternShapeListElement;
  public:
    PatternShape(ShapeType type);
    virtual ~PatternShape() {}
    virtual int operator== (const PatternShape &ps) const 
    { return d_ID == ps.d_ID;}

    virtual PatternShape *duplicate() {return NULL;}

    /// draw using openGL so the user can see what it looks like
    virtual void drawToDisplay(double /*units_per_pixel_x*/, 
                               double /*units_per_pixel_y*/,
                               PatternShapeColorMap &/*color*/) {}

    virtual void drawToDisplay(double /*units_per_pixel_x*/, 
                               double /*units_per_pixel_y*/,
                               double /*r*/, double /*g*/, double /*b*/) {}

    /// send high level shape description over the network
    virtual void drawToSEM(nmm_Microscope_SEM_Remote * /*sem*/) {};

    /// send individual point dwell commands to the SEM
    virtual void drawToSEM(nmm_Microscope_SEM_EDAX * /*sem*/,
           double /*current*/, double /*dotSpacing*/, double /*lineSpacing*/,
           int &/*numPoints*/, double &/*expTime*/) {};

    /// count total number of dwell points and total nominal exposure time
    virtual void computeExposureStatistics(int &numPoints, double &expTime,
           double /*current*/, double /*dotSpacing*/, double /*lineSpacing*/)
      {numPoints = 0; expTime = 0;}

    /// extents of this shape in the world
    virtual double minX() {return 0.0;}
    virtual double minY() {return 0.0;}
    virtual double maxX() {return 0.0;}
    virtual double maxY() {return 0.0;}

    /// get the type of shape
    virtual ShapeType type() {return d_shapeType;}

    /// get minimum required linear exposure and return false if
    /// no linear exposure is required
    virtual vrpn_bool minLinearExposure(double &/*exposure_pCoul_per_cm*/)
      { return vrpn_FALSE;}

    /// get minimum required area exposure and return false if
    /// no area exposure is required
    virtual vrpn_bool minAreaExposure(double &/*exposure_uCoul_per_sq_cm*/) 
      { return vrpn_FALSE;}

    virtual void setExposure(double /*linearExposure*/, 
                             double /*areaExposure*/) {}

    /// get all exposure levels in the shape
    virtual void getExposureLevels(list<double> &/*linearLevels*/,
                                   list<double> &/*areaLevels*/) {}

	/// set parent (for shapes composing composite shapes)
	virtual void setParent(PatternShape *parent) {d_parent = parent;}

	/// set world from object transform
	virtual void setParentFromObject(double *transform)
	{int i; for (i = 0; i < 16; i++)
		d_parentFromObject[i] = transform[i];
	}
	virtual void getParentFromObject(double *transform)
	{int i; for (i = 0; i < 16; i++) 
		transform[i] = d_parentFromObject[i];
	}
	virtual void getWorldFromObject(double *transform)
	{
		if (!d_parent) {
			getParentFromObject(transform);
		} else {
			double worldFromParent[16];
			d_parent->getWorldFromObject(worldFromParent);
			nmb_TransformMatrix44 worldFromObject44;
			worldFromObject44.setMatrix(worldFromParent);
			worldFromObject44.compose(d_parentFromObject);
			worldFromObject44.getMatrix(transform);
		}
	}
	virtual void getObjectFromWorld(double *transform)
	{
		double worldFromObjM[16];
		getWorldFromObject(worldFromObjM);
		nmb_TransformMatrix44 objFromWorld;
		objFromWorld.setMatrix(worldFromObjM);
		objFromWorld.invert();
		objFromWorld.getMatrix(transform);
	}

	virtual void handleWorldFromObjectChange() {}

  protected:
	inline void transform(double *transform, double x_src, double y_src, 
		double &x_dest, double &y_dest);
	inline void transform(double *transform, float x_src, float y_src, 
		float &x_dest, float &y_dest);
	inline void transformVect(double *transform, double x_src, double y_src, 
		double &x_dest, double &y_dest);
	inline void transformVect(double *transform, float x_src, float y_src, 
		float &x_dest, float &y_dest);
	PatternShape *d_parent;
	double d_parentFromObject[16];
    int d_ID;
    ShapeType d_shapeType;
    static int s_nextID;
    int d_numReferences;
};

void PatternShape::transform(double *transform, double x_src, double y_src, 
		double &x_dest, double &y_dest)
{
	x_dest = transform[0]*x_src +
		transform[4]*y_src + transform[12];
	y_dest = transform[1]*x_src +
		transform[5]*y_src + transform[13];
}

void PatternShape::transform(double *transform, float x_src, float y_src, 
		float &x_dest, float &y_dest)
{
	x_dest = transform[0]*x_src +
		transform[4]*y_src + transform[12];
	y_dest = transform[1]*x_src +
		transform[5]*y_src + transform[13];
}

void PatternShape::transformVect(double *transform, double x_src, double y_src, 
		double &x_dest, double &y_dest)
{
	x_dest = transform[0]*x_src + transform[4]*y_src;
	y_dest = transform[1]*x_src + transform[5]*y_src;
}

void PatternShape::transformVect(double *transform, float x_src, float y_src, 
		float &x_dest, float &y_dest)
{
	x_dest = transform[0]*x_src + transform[4]*y_src;
	y_dest = transform[1]*x_src + transform[5]*y_src;
}

/* this is basically a wrapper for a PatternShape pointer */
class PatternShapeListElement {
  friend class PatternFile;
  friend class CompositePatternShape;
  public:
    PatternShapeListElement(PatternShape *ps);
    PatternShapeListElement(const PatternShapeListElement &psle);
    ~PatternShapeListElement();
    int operator== (const PatternShapeListElement &psle) const;

    void drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
                               PatternShapeColorMap &color) 
    {
      d_shape->drawToDisplay(units_per_pixel_x, units_per_pixel_y, color);
    }
	void drawToDisplay(double units_per_pixel_x, 
                               double units_per_pixel_y,
                               double r, double g, double b)
	{
		d_shape->drawToDisplay(units_per_pixel_x, units_per_pixel_y, r,g,b);
	}

    /// send high level shape description over the network
    void drawToSEM(nmm_Microscope_SEM_Remote *sem) {d_shape->drawToSEM(sem);}

    /// send individual point dwell commands to the SEM
    void drawToSEM(nmm_Microscope_SEM_EDAX *sem,
          double current, double dotSpacing, double lineSpacing,
          int &numPoints, double &expTime) 
              {d_shape->drawToSEM(sem, current, dotSpacing, lineSpacing,
                                  numPoints, expTime);}

    /// count total number of dwell points and total nominal exposure time
    virtual void computeExposureStatistics(int &numPoints, double &expTime,
                  double current, double dotSpacing, double lineSpacing)
              {d_shape->computeExposureStatistics(numPoints, expTime,
                      current, dotSpacing, lineSpacing);}

    double minX() {return d_shape->minX();}
    double minY() {return d_shape->minY();}
    double maxX() {return d_shape->maxX();}
    double maxY() {return d_shape->maxY();}

    ShapeType shapeType() {return d_shape->type();}

    vrpn_bool minLinearExposure(double &exposure_pCoul_per_cm)
                  { return d_shape->minLinearExposure(exposure_pCoul_per_cm);}
    vrpn_bool minAreaExposure(double &exposure_uCoul_per_sq_cm)
                  { return d_shape->minAreaExposure(exposure_uCoul_per_sq_cm);}
    void setExposure(double linearExposure, double areaExposure)
                  { d_shape->setExposure(linearExposure, areaExposure);}
    void getExposureLevels(list<double> &linearLevels,
                                   list<double> &areaLevels)
    { d_shape->getExposureLevels(linearLevels, areaLevels);}

  protected:
    PatternShape *d_shape;
};

class PolylinePatternShape : public PatternShape {
  friend class PatternFile;
  public:
    PolylinePatternShape(double lineWidth = 0.0, 
                         double line_dose = 0.0, double area_dose = 0.0);
    PolylinePatternShape(const PolylinePatternShape &pps);
    ~PolylinePatternShape() 
    { if (d_leftSidePoints) delete [] d_leftSidePoints;
      if (d_rightSidePoints) delete [] d_rightSidePoints;
    }
   
    virtual PatternShape *duplicate()
        {return (PatternShape *)new PolylinePatternShape(*this);}

    /// draw using openGL so the user can see what it looks like
    virtual void drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
                               PatternShapeColorMap &color);

	virtual void drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
							   double r, double g, double b);

    /// send high level shape description over the network
    virtual void drawToSEM(nmm_Microscope_SEM_Remote *sem);

    /// send individual point dwell commands to the SEM
    virtual void drawToSEM(nmm_Microscope_SEM_EDAX *sem,
                  double current, double dotSpacing, double lineSpacing,
                  int &numPoints, double &expTime);

    /// count total number of dwell points and total nominal exposure time
    virtual void computeExposureStatistics(int &numPoints, double &expTime,
                  double current, double dotSpacing, double lineSpacing);

    /// extents of this shape in the world
    virtual double minX();
    virtual double minY();
    virtual double maxX();
    virtual double maxY();

    /// get minimum required linear exposure and return false if
    /// no linear exposure is required
    virtual vrpn_bool minLinearExposure(double &exposure_pCoul_per_cm);

    /// get minimum required area exposure and return false if
    /// no area exposure is required
    virtual vrpn_bool minAreaExposure(double &exposure_uCoul_per_sq_cm);

    virtual void setExposure(double linearExposure, double areaExposure);

    /// get all exposure levels in the shape
    virtual void getExposureLevels(list<double> &linearLevels,
                                   list<double> &areaLevels);

	virtual void handleWorldFromObjectChange();

    void setPoints(list<PatternPoint> &points);
    void getPoint(int index, double &x, double &y);
	void getPointInWorld(int index, double &x, double &y);
    void setPoint(int index, double x, double y);
    void addPoint(double x, double y);
	int numPoints() {return d_numPoints;}
    void removePoint();
    void clearPoints();

    void setLineWidth(double width_nm);
    double getLineWidth();

  protected:
    // helper functions
    void computeSidePoints();
    // for the special case of 0 line width we use these functions
    void drawToDisplayZeroWidth(double units_per_pixel_x,
                               double units_per_pixel_y,
                               double r, double g, double b);
    
    void generateExposurePoints(nmm_Microscope_SEM_EDAX *sem,
         double current, double dotSpacing, double lineSpacing,
         int &numPoints, double &expTime);

    void generateExposurePointsZeroWidth(nmm_Microscope_SEM_EDAX *sem,
         double current, double dotSpacing, double lineSpacing,
         int &numPoints, double &expTime);

    virtual double minXZeroWidth();
    virtual double minYZeroWidth();
    virtual double maxXZeroWidth();
    virtual double maxYZeroWidth();

    list<PatternPoint> d_points;
    vrpn_bool d_sidePointsNeedUpdate;
    PatternPoint *d_leftSidePoints;
    PatternPoint *d_rightSidePoints;
    int d_numPoints;
    double d_lineWidth_nm;
    double d_exposure_uCoulombs_per_square_cm;
    double d_exposure_pCoulombs_per_cm;
};

class PolygonPatternShape : public PatternShape {
  friend class PatternFile;
  public:
    PolygonPatternShape(const PolygonPatternShape &pps);
    PolygonPatternShape(double area_dose = 0.0);
    ~PolygonPatternShape() {};

    virtual PatternShape *duplicate()
        {return (PatternShape *)new PolygonPatternShape(*this);}

    /// draw using openGL so the user can see what it looks like
    virtual void drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
                               PatternShapeColorMap &color);

	virtual void drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
							   double r, double g, double b);

    /// send high level shape description over the network
    virtual void drawToSEM(nmm_Microscope_SEM_Remote *sem);

    /// send individual point dwell commands to the SEM
    virtual void drawToSEM(nmm_Microscope_SEM_EDAX *sem,
           double current, double dotSpacing, double lineSpacing,
           int &numPoints, double &expTime);

    /// count total number of dwell points and total nominal exposure time
    virtual void computeExposureStatistics(int &numPoints, double &expTime,
                  double current, double dotSpacing, double lineSpacing);

    /// extents of this shape in the world
    virtual double minX();
    virtual double minY();
    virtual double maxX();
    virtual double maxY();

    /// get minimum required linear exposure and return false if
    /// no linear exposure is required
    virtual vrpn_bool minLinearExposure(double &exposure_pCoul_per_cm);

    /// get minimum required area exposure and return false if
    /// no area exposure is required
    virtual vrpn_bool minAreaExposure(double &exposure_uCoul_per_sq_cm);

    virtual void setExposure(double linearExposure, double areaExposure);

    /// get all exposure levels in the shape
    virtual void getExposureLevels(list<double> &linearLevels,
                                   list<double> &areaLevels);

    void setPoints(list<PatternPoint> &points);
    void getPoint(int index, double &x, double &y);
	void getPointInWorld(int index, double &x, double &y);
    void setPoint(int index, double x, double y);
    void addPoint(double x, double y);
	int numPoints() {return d_numPoints;}
    void removePoint();
    void clearPoints();

  protected:
    void generateExposurePoints(nmm_Microscope_SEM_EDAX *sem,
           double current, double dotSpacing, double lineSpacing,
           int &numPoints, double &expTime);

    list<PatternPoint> d_points;
    int d_numPoints;
    double d_exposure_uCoulombs_per_square_cm;
};

class CompositePatternShape : public PatternShape {
  friend class PatternFile;
  public:
    CompositePatternShape(const CompositePatternShape &cps);
    CompositePatternShape();
    ~CompositePatternShape() {};

	CompositePatternShape &operator = (const CompositePatternShape &cps);

    virtual PatternShape *duplicate()
        {return (PatternShape *)new CompositePatternShape(*this);}

    /// draw using openGL so the user can see what it looks like
    virtual void drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
                               PatternShapeColorMap &color);

	virtual void drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
							   double r, double g, double b);

    /// send high level shape description over the network
    virtual void drawToSEM(nmm_Microscope_SEM_Remote *sem);

    /// send individual point dwell commands to the SEM
    virtual void drawToSEM(nmm_Microscope_SEM_EDAX *sem,
                  double current, double dotSpacing, double lineSpacing,
                  int &numPoints, double &expTime);

    /// count total number of dwell points and total nominal exposure time
    virtual void computeExposureStatistics(int &numPoints, double &expTime,
                  double current, double dotSpacing, double lineSpacing);

    /// extents of this shape in the world
    virtual double minX();
    virtual double minY();
    virtual double maxX();
    virtual double maxY();

    /// get minimum required linear exposure and return false if
    /// no linear exposure is required
    virtual vrpn_bool minLinearExposure(double &exposure_pCoul_per_cm);

    /// get minimum required area exposure and return false if
    /// no area exposure is required
    virtual vrpn_bool minAreaExposure(double &exposure_uCoul_per_sq_cm);

    virtual void setExposure(double linearExposure, double areaExposure);

    /// get all exposure levels in the shape
    virtual void getExposureLevels(list<double> &linearLevels,
                                   list<double> &areaLevels);

    virtual void handleWorldFromObjectChange();

    void setSubShapes(list<PatternShapeListElement> &shapes)
       { d_subShapes = shapes;}
    list<PatternShapeListElement> &getSubShapes() {return d_subShapes;}

    void addSubShape(PatternShape *shape)
       {shape->setParent(this); 
		d_subShapes.push_back(PatternShapeListElement(shape));}
    void removeSubShape() {d_subShapes.pop_back();}
    list<PatternShapeListElement>::iterator shapeListBegin()
       {return d_subShapes.begin();}
    list<PatternShapeListElement>::iterator shapeListEnd()
       {return d_subShapes.end();}
    int empty() {return d_subShapes.empty();}
    void clear() { d_subShapes.clear();}


  protected:
    void generateExposurePoints(nmm_Microscope_SEM_EDAX *sem,
           double current, double dotSpacing, double lineSpacing,
           int &numPoints, double &expTime);


    list<PatternShapeListElement> d_subShapes;
};

class DumpPointPatternShape : public PatternShape {
  friend class PatternFile;
  public:
    DumpPointPatternShape(const DumpPointPatternShape &dpps);
    DumpPointPatternShape(double x_nm = 0.0, double y_nm = 0.0);

    virtual PatternShape *duplicate()
        {return (PatternShape *)new DumpPointPatternShape(*this);}

    /// draw using openGL so the user can see what it looks like
    virtual void drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
                               PatternShapeColorMap &color);

	virtual void drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
							   double r, double g, double b);

    /// send high level shape description over the network
    virtual void drawToSEM(nmm_Microscope_SEM_Remote *sem);

    /// send individual point dwell commands to the SEM
    virtual void drawToSEM(nmm_Microscope_SEM_EDAX *sem,
                  double current, double dotSpacing, double lineSpacing,
                  int &numPoints, double &expTime);

    /// count total number of dwell points and total nominal exposure time
    virtual void computeExposureStatistics(int &numPoints, double &expTime,
                  double current, double dotSpacing, double lineSpacing);

    /// extents of this shape in the world
    virtual double minX();
    virtual double minY();
    virtual double maxX();
    virtual double maxY();

    /// get minimum required linear exposure and return false if
    /// no linear exposure is required
    virtual vrpn_bool minLinearExposure(double &/*exposure_pCoul_per_cm*/)
            { return vrpn_FALSE;}

    /// get minimum required area exposure and return false if
    /// no area exposure is required
    virtual vrpn_bool minAreaExposure(double &/*exposure_uCoul_per_sq_cm*/)
            { return vrpn_FALSE;}

    virtual void setExposure(double /*linearExposure*/, 
                             double /*areaExposure*/) {}

    /// get all exposure levels in the shape
    virtual void getExposureLevels(list<double> &linearLevels,
                                   list<double> &areaLevels) 
            { linearLevels.clear(); areaLevels.clear(); }
    void setLocation(double x_nm, double y_nm);

  protected:
    PatternPoint d_location;
    double d_dwellTime_sec;
};

#endif
