#include "correspondence.h"
#include "stdlib.h"
#include "nmb_Types.h"	// for vrpn_bool
#include "math.h"	// for fabs()

Correspondence::Correspondence()
{
    num_points = 0;
    max_points = 0;
    num_spaces = 0;
    pnts = NULL;
}

Correspondence::Correspondence(int num_im, int max_pnts)
{
    num_points = 0;
    max_points = max_pnts;
    num_spaces = num_im;

    pnts = new corr_point_t*[num_im];
    int i;
    for (i = 0; i < num_im; i++)
        pnts[i] = new corr_point_t[max_pnts];
}

/* This function will always change the number of images
   that the correspondence but will only change the maximum
   number of points if the specified value max_pnts is greater
   than the current value for the maximum number

   The reason for doing it this way is that we may want three points
   where two of the points are duplicates and so only two points are
   specified. When we copy the specified correspondence into the
   one used for registration we just have to duplicate the last one to
   get the required three points to use in finding the solution.
*/
void Correspondence::init(int num_im, int max_pnts)
{
    int i;
    if (pnts){
        for (i = 0; i < num_im; i++)
	    delete [] pnts[i];
        delete [] pnts;
    }

    num_points = 0;
    if (max_pnts > max_points) {
        max_points = max_pnts;
    }
    num_spaces = num_im;

    pnts = new corr_point_t*[num_im];
    for (i = 0; i < num_im; i++)
        pnts[i] = new corr_point_t[max_points];
}

Correspondence &Correspondence::operator = (const Correspondence &c) {
    init(c.numSpaces(), c.numPoints());
    int i,j;
    corr_point_t p;
    for (j = 0; j < c.numPoints(); j++){
        addPoint(p);
        for (i = 0; i < c.numSpaces(); i++){
        	c.getPoint(i, j, &p);
        	setPoint(i,j, p);
    	}
    }
	return (*this);
}

void Correspondence::print()
{
   int i, j;
   printf("%d points, %d spaces\n", num_points, num_spaces);
   for (i = 0; i < num_points; i++) {
       for (j = 0; j < num_spaces; j++){
           printf("(%g,%g,%g) ", pnts[j][i].x, pnts[j][i].y, pnts[j][i].z);
       }
       printf("\n");
   }
}

int Correspondence::addPoint(corr_point_t &p)
{
    if (num_points == max_points) return -1;
    unsigned int i;
    for (i = 0; i < num_spaces; i++){
        pnts[i][num_points] = p;
    }
    num_points++;
    return num_points-1;
}

int Correspondence::setPoint(int spaceIdx, int pntIdx, corr_point_t &p)
{
    if (pntIdx < 0 || (unsigned)pntIdx >= num_points) return -1;
    if (spaceIdx < 0 || (unsigned)spaceIdx >= num_spaces) return -1;
    pnts[spaceIdx][pntIdx] = p;
    return 0;
}

int Correspondence::deletePoint(int pntIdx)
{
    unsigned int i;
    num_points--;
    for (i = 0; i < num_spaces; i++)
        pnts[i][pntIdx] = pnts[i][num_points];
    return 0;
}

vrpn_bool Correspondence::findNearestPoint(int spaceIdx, double x, double y,
                double x_max, double y_max, int *pntIdx)
{
    if (spaceIdx < 0 || (unsigned)spaceIdx >= num_spaces) return VRPN_FALSE;
    unsigned int i;
    vrpn_bool found_point = VRPN_FALSE;
    float dx, dy, min_distance;

    for (i = 0; i < num_points; i++){
        dx = fabs(x-pnts[spaceIdx][i].x);
        dy = fabs(y-pnts[spaceIdx][i].y);
        if (dx <= x_max && dy <= y_max){
            if (!found_point){
                found_point = VRPN_TRUE;
                min_distance = dx + dy;
                *pntIdx = i;
            }
            else if (dx + dy < min_distance){
                min_distance = dx+dy;
                *pntIdx = i;
            }
        }
    }
    return found_point;
}

int Correspondence::getPoint(int spaceIdx, int pntIndex,
        corr_point_t *pnt) const
{
    if (pntIndex < 0 || (unsigned)pntIndex >= num_points) return -1;
    if (spaceIdx < 0 || (unsigned)spaceIdx >= num_spaces) return -1;
    *pnt = pnts[spaceIdx][pntIndex];
    return 0;
}

// this converts points from grid units and sets the z values to
// x,y,z in nano-meters or whatever units the world is measured in
int Correspondence::setValuesFromImage(int spaceIdx, nmb_Image *im)
{
    // note: points are assumed to be normalized image units with
    // x,y from 0..1
    if (spaceIdx < 0 || (unsigned)spaceIdx >= num_spaces) return -1;

    double x_normalized, y_normalized;
	double x_pixels, y_pixels;
	double x_world, y_world;
    unsigned int i;
    for (i = 0; i < num_points; i++) {
    	x_normalized = pnts[spaceIdx][i].x;	// x is 0..1
    	y_normalized = pnts[spaceIdx][i].y;	// y is 0..1
	x_pixels = x_normalized*(im->width());
	y_pixels = y_normalized*(im->height());
	im->pixelToWorld(x_pixels, y_pixels, x_world, y_world);
    	pnts[spaceIdx][i].x = x_world;
    	pnts[spaceIdx][i].y = y_world;
    	pnts[spaceIdx][i].z = im->getValueInterpolated(x_pixels, y_pixels);
    }
    return 0;
}

// this converts points from grid units and sets the z values to
// x,y,z in nM
int Correspondence::setValuesFromPlane(int spaceIdx, BCPlane *p)
{
    // note: points are assumed to be normalized image units with 
    // x,y from 0..1
    if (spaceIdx < 0 || (unsigned)spaceIdx >= num_spaces) return -1;

    double x, y, z;
    unsigned int i;
    for (i = 0; i < num_points; i++) {
	x = pnts[spaceIdx][i].x;
	y = pnts[spaceIdx][i].y;
	z = p->interpolatedValue(x*p->numX(), y*p->numY());
        pnts[spaceIdx][i].x = x*(p->maxX() - p->minX()) + p->minX();
	pnts[spaceIdx][i].y = y*(p->maxY() - p->minY()) + p->minY();
	pnts[spaceIdx][i].z = z;
    }
    return 0;
}
