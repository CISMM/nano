#ifndef EXPOSUREPATTERN_H
#define EXPOSUREPATTERN_H

#include "list.h"

class PatternPoint {
  public:
   PatternPoint(double x=0, double y=0 ): d_x(x), d_y(y) {};
   int operator== (const PatternPoint &ppt) {
        return (d_x == ppt.d_x && d_y == ppt.d_y);}
   void translate(double x, double y)
   { d_x += x; d_y += y;}

   double d_x, d_y;
};

typedef enum {PS_POLYLINE, PS_POLYGON} ShapeType;

class PatternShape {
  public:
    PatternShape(double lw = 0.0, double exp = 1.0, ShapeType type=PS_POLYLINE);
    PatternShape(const PatternShape &sh);
    int operator== (const PatternShape &sh) {return (d_ID == sh.d_ID);}
    void addPoint(double x, double y);
    void removePoint();
    void translate(double x, double y)
    { d_trans_x += x; d_trans_y += y; }
    void draw(double units_per_pixel_x, double units_per_pixel_y);
    void drawThinPolyline(double units_per_pixel_x, double units_per_pixel_y);
    void drawThickPolyline(double units_per_pixel_x, double units_per_pixel_y);
    void drawPolygon(double units_per_pixel_x, double units_per_pixel_y);
    list<PatternPoint>::iterator pointListBegin();
    list<PatternPoint>::iterator pointListEnd();
    double minY();
    double maxY();

    int d_ID;
    static int s_nextID;

    double d_lineWidth_nm;
    double d_exposure_uCoulombs_per_square_cm;
    list<PatternPoint> d_points;
    ShapeType d_type;
    double d_trans_x, d_trans_y;
};

#endif
