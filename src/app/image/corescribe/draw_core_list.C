// draw_core_list.C
// Part of corescribe program.
// Actually draws the cores on the image.
// 
// Bugs/Design flaws: Does a range check every time it plots a point.
// This conscious tradeoff of speed for reliability/programming ease
// may be a bad one.

//--------------------------------------------------------------
// Includes and usr/Image #defines

#define D_STDLIB
#define D_MATH
#define D_LongImage
#define D_IntensRange

#define D_GraphUtility
#define D_maxima

#define D_CoreList
#define D_cores

#include <imprelud.h>
#include "draw_core_list.h"

//--------------------------------------------------------------
// Global definitions and declarations.

#define DOT_RADIUS 2

typedef scale_point_2_t Point;

// The MAXIMA line drawing function requires callback functions to
// tell it what to do with the coordinates its line algorithm
// computes.  The callback functions have to already know where to
// plot the points, and the easiest way to do this is to make the
// image a global object, and hard code it into the callback functions
// that they plot to that image.

static struct {
    const LongImage* im_p;
    long min;
    long max;
} image_g;

//--------------------------------------------------------------
// Protototypes of functions not in draw_core_list.h (which only
// contains draw_core_list() and draw_core()).

void internal_draw_core(const Core2* core_p, double tick_spacing);
void draw_core_curve(const Core2* core, void (*process)(int x, int y));
void draw_tickmarks(const Core2* core, double tick_spacing);
void draw_boundary(const Core2* core_p, double tick_spacing);
inline void small_dark_spot(int x, int y);
inline void big_light_spot(int x, int y);
inline void mark_beginning(int x, int y);
inline void make_tickmark(int x, int y);
void make_boundary_points(double x, double y, double s, double prevx, double prevy);
void big_spot(long intensity, int x, int y);
inline double distance(Point a, Point b);
inline long round(double x);

//--------------------------------------------------------------
// Procedures to draw cores and tick marks. 

// draw_core_list()
// Draws the cores in core_list on the image im.
void
draw_core_list(const CoreList& core_list,
	       const LongImage& im,
	       double tick_spacing,
	       const IntensRange& range)
{
    image_g.im_p = &im;

    image_g.min = range.range_min();
    image_g.max = range.range_max();

    for( unsigned i = 0; i < core_list.size(); i++) {
	internal_draw_core(core_list.core_addr(i), tick_spacing);
    }
}

// draw_core()
// Draws a single core, as opposed to a core_list.
void
draw_core(const Core2& core, 
	  const LongImage& im, 
	  double tick_spacing,
	  const IntensRange& range)
{
    image_g.im_p = &im;

    image_g.min = range.range_min();
    image_g.max = range.range_max();

    internal_draw_core(&core, tick_spacing);
}

// internal_draw_core()
// Draws the core *core_p on the image in image_g.im_p as a thin
// dark line inside a thick white line with tick marks at given
// intervals.
void 
internal_draw_core(const Core2* core_p, double tick_spacing)
{
    draw_core_curve(core_p, big_light_spot);
    draw_core_curve(core_p, small_dark_spot);
    draw_tickmarks(core_p, tick_spacing);
    draw_boundary(core_p, tick_spacing);
}

// draw_core_curve()
// Takes the core 'core_p' and draws it on image_g.im_p by applying
// the function (*process)() at each point.  Called twice with
// different values of (*process)() to draw the thin dark line on the
// thick white line.
void
draw_core_curve(const Core2* core_p, void (*process)(int x, int y))
{
    int prevx = 0;
    int prevy = 0;
    int x = 0;
    int y = 0;

    GraphUtility gu;
    gu.process2D = process;
    
    int i = 0;
    int iend = core_p->size();
    prevx = round(core_p->x(i));
    prevy = round(core_p->y(i));
    for (i++; i < iend; i++) {
	x = round(core_p->x(i));
	y = round(core_p->y(i));
	gu.line2D(prevx, prevy, x, y);
	prevx = x;
	prevy = y;
    }
}

// draw_tickmarks()
// Draws tick marks separated by tick_spacing.  Not interleaved with
// the drawing of the curves themselves since the curves must be drawn
// twice.  Always draws tickmarks at the end of the line segment that
// straddles the actual correct location, so the location is off a
// little.
void
draw_tickmarks(const Core2* core_p, double tick_spacing)
{
    Point p;
    Point prev;
    double arclength = 0;

    int i = 0;
    int iend = core_p->size();
    prev = core_p->point(i);
    mark_beginning(round(prev.x), round(prev.y));
    for (i++; i < iend; i++) {
	p = core_p->point(i);
	arclength += distance(prev, p);
	if (arclength >= tick_spacing) {
	    make_tickmark(round(p.x), round(p.y));
	    arclength -= tick_spacing;
	}
	prev = p;
    }
}

// draw_boundary()
// Draws points on the implied boundary, separated by tick_spacing.
void
draw_boundary(const Core2* core_p, double tick_spacing)
{
    Point p;
    Point prev;
    double arclength = 0;

    int i = 0;
    int iend = core_p->size();
    prev = core_p->point(i);
    for (i++; i < iend; i++) {
	p = core_p->point(i);
	arclength += distance(prev, p);
	if (arclength >= tick_spacing) {
	    make_boundary_points(p.x, p.y, p.s, prev.x, prev.y);
	    arclength -= tick_spacing;
	}
	prev = p;
    }
}

//--------------------------------------------------------------
// Functions that actually plot the points.

// big_light_spot(int x, int y)
inline void
big_light_spot(int x, int y)
{
    long white = image_g.max;
    big_spot(white, x, y);
}

// small_dark_spot(int x, int y)
// Makes a one-pixel black spot at the indicated coordinates.
inline void
small_dark_spot(int x, int y)
{
    int xmax = image_g.im_p->shape()[0];
    int ymax = image_g.im_p->shape()[1];
    long black = image_g.min;

    if (0 <= x && x < xmax && 0 <= y && y < ymax)
	(*image_g.im_p)(x, y) = black;
}

// mark_beginning(int x, int y)
// Makes a circular gray tickmark at the indicated coordinates.
inline void
mark_beginning(int x, int y)
{ 
    long gray = (image_g.min + image_g.max)/2;
    big_spot(gray, x, y);
}

// make_tickmark(int x, int y)
// Makes a circular black tickmark at the indicated coordinates.
inline void
make_tickmark(int x, int y)
{
    long black = image_g.min;
    big_spot(black, x, y);
}

// make_boundary_points(x, y, s, prevx, prevy)
// Makes circular black dots a distance s on either side of (x, y),
// measured perpendicular to the line from (prevx, prevy) to (x, y).
void
make_boundary_points(double x, double y, double s, double prevx, double prevy)
{
    double xperp = prevy - y;
    double yperp = x - prevx;
    double magperp = sqrt(xperp * xperp + yperp * yperp);
    long x1 = round(x + xperp / magperp * s);
    long x2 = round(x - xperp / magperp * s);
    long y1 = round(y + yperp / magperp * s);
    long y2 = round(y - yperp / magperp * s);
    big_light_spot(x1, y1);
    big_light_spot(x2, y2);
    small_dark_spot(x1, y1);
    small_dark_spot(x2, y2);
}

// big_spot(long intensity, int x, int y)
// Makes a roughly circular spot of the given intensity, five pixels
// across, centered at the indicated coordinates.
void
big_spot(long intensity, int x, int y)
{
    int xmax = image_g.im_p->shape()[0];
    int ymax = image_g.im_p->shape()[1];

    int i, j;
    for (i = x-1; i <= x+1; i++)
	for (j = y-2; j <= y+2; j++)
	    if (0 <= i && i < xmax && 0 <= j && j < ymax)
		(*image_g.im_p)(i, j) = intensity;
    for (i = x-2; i <= x+2; i += 4)
	for (j = y-1; j <= y+1; j++)
	    if (0 <= i && i < xmax && 0 <= j && j < ymax)
		(*image_g.im_p)(i, j) = intensity;
}

//--------------------------------------------------------------
// Utilities used by draw_core_curve() and draw_tickmarks().

inline double
distance(Point a, Point b)
{
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    return sqrt(dx * dx + dy * dy);
}

inline long
round(double x)
{
    return (long) (x + 0.5);
}


