#include "nmr_SurfaceModel.h"

nmr_SurfaceModelHeightField::nmr_SurfaceModelHeightField(
	nmb_Image *heightValues)
{
	if (heightValues) {
		double sideLength = heightValues->widthWorld()/
			(double)(heightValues->width()-1);
		d_heightField = new nmg_HeightField(heightValues, 
		sideLength, 1.0);
	} else {
		d_heightField = NULL;
	}
}

nmr_SurfaceModelHeightField::~nmr_SurfaceModelHeightField()
{
	if (d_heightField) {
		delete d_heightField;
	}
}
	
//virtual 
vrpn_bool nmr_SurfaceModelHeightField::surfaceRayIntersection(
								double x, double y, double z,
								double vx, double vy, double vz, 
								double &t)
{
	if (!d_heightField) return vrpn_FALSE;

	nmg_Point_3d start(x, y, z);
	nmg_Vector_3d dir(vx, vy, vz);
	nmg_Ray ray(start, dir);
	double minT;
	nmg_Vector_3d normal;
	bool startingInside = vrpn_FALSE;
	if (d_heightField->contains(start)) {
		startingInside = vrpn_TRUE;
	}
	bool foundIntersection = 
		d_heightField->intersectsRay(ray, startingInside, minT, normal);
	if (!foundIntersection) {
		ray = nmg_Ray(start, -dir);
		foundIntersection = 
			d_heightField->intersectsRay(ray, startingInside, minT, normal);
		minT = -minT;
	}

	t = minT;
	return foundIntersection;
}
