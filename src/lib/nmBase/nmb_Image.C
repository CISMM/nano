#include "Topo.h" // for TopoFile
#include "nmb_Image.h"
#include "nmb_ImageTransform.h"


#include <limits.h>
#ifdef _WIN32
#include <float.h> // for FLT_MAX
#endif

nmb_ImageBounds::nmb_ImageBounds() 
{
    nmb_ImageBounds(0.0, 0.0, 0.0, 0.0);
}

nmb_ImageBounds::nmb_ImageBounds(double x0,double y0,double x1,double y1){
    x[MIN_X_MIN_Y] = x0; x[MIN_X_MAX_Y] = x0;
    y[MIN_X_MIN_Y] = y0; y[MAX_X_MIN_Y] = y0;
    x[MAX_X_MIN_Y] = x1; x[MAX_X_MAX_Y] = x1;
    y[MIN_X_MAX_Y] = y1; y[MAX_X_MAX_Y] = y1;
}


// virtual
nmb_Image::~nmb_Image (void) {

}

float nmb_Image::getValueInterpolated(double i, double j) const {
	int i_ip, j_ip;
	double i_fp, j_fp;

	if (i <= 0.5) {
		i_ip = 0;
		i_fp = 0.0;
	} else if (i >= (double)width()-0.5) {
		i_ip = width() - 2;
		i_fp = 1.0;
	} else {
		i_ip = (int)floor(i-0.5);
		i_fp = (i - 0.5) - (double)i_ip;
	}

	if (j <= 0.5) {
		j_ip = 0;
		j_fp = 0.0;
	} else if (j >= (double)height()-0.5) {
		j_ip = height() - 2;
		j_fp = 1.0;
	} else {
    	j_ip = (int)floor(j-0.5);
		j_fp = (j - 0.5) - (double)j_ip;
	}

    double val;
    val = (1-i_fp)*(1-j_fp)*getValue(i_ip, j_ip);
    val += (1-i_fp)*j_fp*getValue(i_ip, j_ip+1);
    val += i_fp*(1-j_fp)*getValue(i_ip+1,j_ip);
    val += i_fp*j_fp*getValue(i_ip+1,j_ip+1);
    return val;
}

float nmb_Image::getValueInterpolatedNZ(double i, double j) const {
        int i_ip, j_ip;
        double i_fp, j_fp;

        if (i <= 0.5) {
                i_ip = 0;
                i_fp = 0.0;
        } else if (i >= (double)width()-0.5) {
                i_ip = width() - 2;
                i_fp = 1.0;
        } else {
                i_ip = (int)floor(i-0.5);
                i_fp = (i - 0.5) - (double)i_ip;
        }

        if (j <= 0.5) {
                j_ip = 0;
                j_fp = 0.0;
        } else if (j >= (double)height()-0.5) {
                j_ip = height() - 2;
                j_fp = 1.0;
        } else {
        j_ip = (int)floor(j-0.5);
                j_fp = (j - 0.5) - (double)j_ip;
        }

    double val, v00, v01, v10, v11;
    v00 = getValue(i_ip, j_ip);
    v01 = getValue(i_ip, j_ip+1);
    v10 = getValue(i_ip+1,j_ip);
    v11 = getValue(i_ip+1,j_ip+1);
    if (v00 == 0 || v01 == 0 || v10 == 0 || v11 == 0) {
        val = 0;
    } else {
        val = (1-i_fp)*(1-j_fp)*getValue(i_ip, j_ip);
        val += (1-i_fp)*j_fp*getValue(i_ip, j_ip+1);
        val += i_fp*(1-j_fp)*getValue(i_ip+1,j_ip);
        val += i_fp*j_fp*getValue(i_ip+1,j_ip+1);
    }
    return val;
}

double nmb_Image::widthWorld() const {
    double dx,dy;
    dx = boundX(nmb_ImageBounds::MAX_X_MIN_Y) -
                boundX(nmb_ImageBounds::MIN_X_MIN_Y);
    dy = boundY(nmb_ImageBounds::MAX_X_MIN_Y) -
                boundY(nmb_ImageBounds::MIN_X_MIN_Y);
    return sqrt(dx*dx + dy*dy);
}

double nmb_Image::heightWorld() const {
    double dx,dy;
    dx = boundX(nmb_ImageBounds::MIN_X_MAX_Y) -
                boundX(nmb_ImageBounds::MIN_X_MIN_Y);
    dy = boundY(nmb_ImageBounds::MIN_X_MAX_Y) -
                boundY(nmb_ImageBounds::MIN_X_MIN_Y);
    return sqrt(dx*dx + dy*dy);
}

/** This function assumes that (i,j) are coordinates for the basis vectors (u,v)
   where u = pnt_max_i_min_j - pnt_min_i_min_j
         v = pnt_min_i_max_j - pnt_min_i_min_j
   pnt_max_i_min_j, pnt_min_i_min_j, pnt_min_i_max_j are the world positions of
   three of the corners of the image
*/
void nmb_Image::pixelToWorld(const double i, const double j,
                  double &x, double &y) const {
        // just bilinear interpolation:
        double y_frac = j/((double)height());
        double x_frac = i/((double)width());
        double x_min = boundX(nmb_ImageBounds::MIN_X_MIN_Y)*(1.0-y_frac) +
             boundX(nmb_ImageBounds::MIN_X_MAX_Y)*(y_frac);
        double x_max = boundX(nmb_ImageBounds::MAX_X_MIN_Y)*(1.0-y_frac) +
             boundX(nmb_ImageBounds::MAX_X_MAX_Y)*(y_frac);
        x = x_min*(1.0-x_frac) + x_max*(x_frac);
        double y_min = boundY(nmb_ImageBounds::MIN_X_MIN_Y)*(1.0-y_frac) +
             boundY(nmb_ImageBounds::MIN_X_MAX_Y)*(y_frac);
        double y_max = boundY(nmb_ImageBounds::MAX_X_MIN_Y)*(1.0-y_frac) +
             boundY(nmb_ImageBounds::MAX_X_MAX_Y)*(y_frac);
        y = y_min*(1.0-x_frac) + y_max*(x_frac);
}

/** This function assumes that (i,j) are coordinates for the basis vectors (u,v)
   where u = pnt_max_i_min_j - pnt_min_i_min_j
         v = pnt_min_i_max_j - pnt_min_i_min_j
   pnt_max_i_min_j, pnt_min_i_min_j, pnt_min_i_max_j are the world positions of
   three of the corners of the image
*/
void nmb_Image::worldToPixel(const double x, const double y,
                  double &i, double &j) const {
        // project x,y onto i,j coordinate axes
        double x_diff = boundX(nmb_ImageBounds::MAX_X_MIN_Y) -
                        boundX(nmb_ImageBounds::MIN_X_MIN_Y);
        double y_diff = boundY(nmb_ImageBounds::MAX_X_MIN_Y) -
                        boundY(nmb_ImageBounds::MIN_X_MIN_Y);
        double temp = (x - boundX(nmb_ImageBounds::MIN_X_MIN_Y))*x_diff +
                      (y - boundY(nmb_ImageBounds::MIN_X_MIN_Y))*y_diff;

        double x_frac = temp/(x_diff*x_diff + y_diff*y_diff);
        i = x_frac*(double)width();

        x_diff = boundX(nmb_ImageBounds::MIN_X_MAX_Y) -
                 boundX(nmb_ImageBounds::MIN_X_MIN_Y);
        y_diff = boundY(nmb_ImageBounds::MIN_X_MAX_Y) -
                 boundY(nmb_ImageBounds::MIN_X_MIN_Y);
        temp = (x - boundX(nmb_ImageBounds::MIN_X_MIN_Y))*x_diff +
               (y - boundY(nmb_ImageBounds::MIN_X_MIN_Y))*y_diff;
        double y_frac = temp/(x_diff*x_diff + y_diff*y_diff);
        j = y_frac*(double)height();
}

/**
 returns transformation matrix that takes points in world to points in
 normalized image coordinates with x and y in range 0..1

 The matrix is in column major order so one may use this for the texture matrix
 in openGL
*/

void nmb_Image::getWorldToImageTransform(double *matrix44)
{
  /* What this function is doing:
      If the worldToImage matrix has been set then we just return that but
      otherwise we construct an orthographic projection matrix from our 
      knowledge of the positions of three of the corners in the world.

      Start with the system:
        (0,0) = M*(x00, y00, 1)
        (1,0) = M*(x10, y10, 1)
        (0,1) = M*(x01, y01, 1)

        where M is a matrix | a b c |
                            | d e f |
        In each equation:
        the left side represents the position in normalized image coordinates
        and the vector to the right of M represents the corresponding points
        in world coordinates.

        Given x00, y00, x10, y10, x01, y01, 
        solve for a,b,c,d,e,f and stuff these into a 4x4 matrix in the 
        order col0row0-3, col1row0-3, col2row0-3, col3row0-3
  */

  if (d_worldToImageMatrixSet) {
    int i; 
    for (i = 0; i < 16; i++){
      matrix44[i] = d_worldToImageMatrix[i];
    }
  } else {

    double a, b, c, d, e, f;
    double x00, y00, x10, y10, x01, y01;
    x00 = boundX(nmb_ImageBounds::MIN_X_MIN_Y);
    y00 = boundY(nmb_ImageBounds::MIN_X_MIN_Y);
    x10 = boundX(nmb_ImageBounds::MAX_X_MIN_Y);
    y10 = boundY(nmb_ImageBounds::MAX_X_MIN_Y);
    x01 = boundX(nmb_ImageBounds::MIN_X_MAX_Y);
    y01 = boundY(nmb_ImageBounds::MIN_X_MAX_Y);

    double det;
    det = (x10-x00)*(y01-y00) - (y10-y00)*(x01-x00);

    a = (y01-y00)/det;
    b = (x00-x01)/det;
    c = -a*x00 - b*y00;

    d = (y00 - y10)/det;
    e = (x10 - x00)/det;   
    f = -d*x00 - e*y00;

    // first row:
    matrix44[0] = a;
    matrix44[4] = b;
    matrix44[8] = 0.0;
    matrix44[12] = c;
    // second row:
    matrix44[1] = d;
    matrix44[5] = e;
    matrix44[9] = 0.0;
    matrix44[13] = f;

    matrix44[2] = 0.0;
    matrix44[6] = 0.0;
    matrix44[10] = 1.0;
    matrix44[14] = 0.0;

    matrix44[3] = 0.0;
    matrix44[7] = 0.0;
    matrix44[11] = 0.0;
    matrix44[15] = 1.0;
  }
}

void nmb_Image::setWorldToImageTransform(double *matrix44)
{
    d_worldToImageMatrixSet = VRPN_TRUE;
    int i;
    for (i = 0; i < 16; i++){
        d_worldToImageMatrix[i] = matrix44[i];
    }

    // now we need to set the boundary of the image to keep any
    // boundary state consistent with the worldToImage transformation

    nmb_ImageTransformAffine worldToImage(4,4);
    worldToImage.setMatrix(matrix44);

    if (worldToImage.hasInverse()) {
      double imagePnt[4] = {0.0, 0.0, 0.0, 0.0}, worldPnt[4];
      // (0,0) -> ?
      worldToImage.invTransform(imagePnt, worldPnt);
      setBoundX(nmb_ImageBounds::MIN_X_MIN_Y, worldPnt[0]);
      setBoundY(nmb_ImageBounds::MIN_X_MIN_Y, worldPnt[1]);
      imagePnt[0] = 1.0;
      // (1,0) -> ?
      worldToImage.invTransform(imagePnt, worldPnt);
      setBoundX(nmb_ImageBounds::MAX_X_MIN_Y, worldPnt[0]);
      setBoundY(nmb_ImageBounds::MAX_X_MIN_Y, worldPnt[1]);
      imagePnt[0] = 0.0;
      imagePnt[1] = 1.0;
      // (0,1) -> ?
      worldToImage.invTransform(imagePnt, worldPnt);
      setBoundX(nmb_ImageBounds::MIN_X_MAX_Y, worldPnt[0]);
      setBoundY(nmb_ImageBounds::MIN_X_MAX_Y, worldPnt[1]);
      imagePnt[0] = 1.0;
      // (1,1) -> ?
      worldToImage.invTransform(imagePnt, worldPnt);
      setBoundX(nmb_ImageBounds::MAX_X_MAX_Y, worldPnt[0]);
      setBoundY(nmb_ImageBounds::MAX_X_MAX_Y, worldPnt[1]);
    } else {
      fprintf(stderr, "nmb_Image::setWorldToImageTransform:"
            " Error: non-invertible transformation\n");
    }
}

const int nmb_ImageGrid::num_export_formats = 5;
const char *nmb_ImageGrid::export_formats_list[] = {	"ThermoMicroscopes",
                                 		"Text(MathCAD)",
                                 		"PPM Image",
                                 		"SPIP",
                                 		"UNCA Image" };
const nmb_ImageGrid::FileExportingFunction 
	nmb_ImageGrid::file_exporting_function[] = 
				{nmb_ImageGrid::writeTopoFile,
                                 nmb_ImageGrid::writeTextFile,
                                 nmb_ImageGrid::writePPMFile,
                                 nmb_ImageGrid::writeSPIPFile,
                                 nmb_ImageGrid::writeUNCAFile};


nmb_ImageGrid::nmb_ImageGrid(const char *name, const char *units, 
	short x, short y):
            nmb_Image(),
            units_x("nm"), units_y("nm")
{
    BCString name_str(name), units_str(units);
    grid = new BCGrid(x, y, 0.0, 1.0, 0.0, 1.0);
    plane = grid->addNewPlane(name_str, units_str, 0);
    min_x_set = SHRT_MAX; min_y_set = SHRT_MAX;
    max_x_set = -SHRT_MAX; max_y_set = -SHRT_MAX;
    for (int i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }
  
    if (strcmp(units, "nm") == 0) {
        is_height_field = vrpn_TRUE;
    } else {
        is_height_field = vrpn_FALSE;
    }

}

nmb_ImageGrid::nmb_ImageGrid(BCPlane *p):nmb_Image(),
    units_x("nm"), units_y("nm")
{
    // WARNING: assumes (non-zero value <==> value was set) as
    // did BCPlane::findValidDataRange()

    plane = p;
    grid = NULL;
    min_x_set = SHRT_MAX; min_y_set = SHRT_MAX;
     max_x_set = -SHRT_MAX; max_y_set = -SHRT_MAX;

    int i,j;
    for (i = 0; i < plane->numX(); i++){
        for (j = 0; j < plane->numY(); j++){
            if (plane->value(i,j) != 0.0){
                min_x_set = MIN(min_x_set, i);
                max_x_set = MAX(max_x_set, i);
                min_y_set = MIN(min_y_set, j);
                max_y_set = MAX(max_y_set, j);
            }
        }
    }
    for (i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }
    if (strcmp(unitsValue()->Characters(), "nm") == 0) {
        is_height_field = vrpn_TRUE;
    } else {
        is_height_field = vrpn_FALSE;
    }
}

nmb_ImageGrid::nmb_ImageGrid(nmb_Image *im)
{
  int i,j;
  grid = new BCGrid(im->width(), im->height(), 0.0, 1.0, 0.0, 1.0);
  plane = grid->addNewPlane(*(im->name()), *(im->unitsValue()), 0);
  min_x_set = MAXSHORT; min_y_set = MAXSHORT;
  max_x_set = -MAXSHORT; max_y_set = -MAXSHORT;
  for (i = 0; i < numExportFormats(); i++){
      BCString name = exportFormatType(i);
      formatNames.addEntry(name);
  }
  if (strcmp(im->unitsValue()->Characters(), "nm") == 0) {
      is_height_field = vrpn_TRUE;
  } else {
      is_height_field = vrpn_FALSE;
  }

  units_x = *(im->unitsX());
  units_y = *(im->unitsY());

  for (i = 0; i < width(); i++) {
    for (j = 0; j < height(); j++) {
      setValue(i,j, im->getValue(i,j));
    }
  }
  im->validDataRange(&max_y_set, &min_x_set, &min_y_set, &max_x_set);
  im->getTopoFileInfo(d_topoFileDefaults);
}

nmb_ImageGrid::~nmb_ImageGrid()
{   
    if (grid) {
        delete grid;
    }
}

int nmb_ImageGrid::width() const {return plane->numX();}

int nmb_ImageGrid::height() const {return plane->numY();}

float nmb_ImageGrid::getValue(int i, int j) const
             {return plane->value(i,j);}

float nmb_ImageGrid::maxValue() {return plane->maxValue();}

float nmb_ImageGrid::minValue() {return plane->minValue();}

int nmb_ImageGrid::normalize()
{
  int i,j;
  float min = minValue();
  float max = maxValue();
  float range = max-min;
  if (range == 0) return -1;
  float inv_range = 1.0/range;

  for (i = 0; i < width(); i++) {
    for (j = 0; j < height(); j++) {
      setValue(i,j, inv_range*(getValue(i,j)-min));
    }
  }
  return 0;
}

float nmb_ImageGrid::maxValidValue() {
    short top, left, bottom, right;
    if (validDataRange(&top, &left, &bottom, &right)) {
        return 0;
    }
    if ((top == height()-1) && (bottom == 0) && 
        (right == width()-1) && (left == 0)) {
        return maxValue();
    }

    fprintf(stderr, "nmb_ImageGrid::maxValidValue:: Warning, "
           "this function should be implemented more efficiently\n");

    int i,j;
    float result = getValue(left, bottom);
    float val;
    for (i = left; i <= right; i++) {
        for (j = bottom; j <= top; j++) {
            val = getValue(i,j);
            if (val > result) result = val;
        }
    }
    return result;
}

float nmb_ImageGrid::minValidValue() {
    short top, left, bottom, right;
    if (validDataRange(&top, &left, &bottom, &right)) {
        return 0;
    }
    if ((top == height()-1) && (bottom == 0) && 
        (right == width()-1) && (left == 0)) {
        return minValue();
    }

    fprintf(stderr, "nmb_ImageGrid::minValidValue:: Warning, "
           "this function should be implemented more efficiently\n");

    int i,j;
    float result = getValue(left, bottom);
    float val;
    for (i = left; i <= right; i++) {
        for (j = bottom; j <= top; j++) {
            val = getValue(i,j);
            if (val < result) result = val;
        }
    }
    return result;
}

float nmb_ImageGrid::maxNonZeroValue() {
    return maxValidValue();
}

float nmb_ImageGrid::minNonZeroValue() {
    return plane->minNonZeroValue();
}

void nmb_ImageGrid::setValue(int i, int j, float val)
{
     plane->setValue(i,j,val);
     min_x_set = MIN(min_x_set, i);
     max_x_set = MAX(max_x_set, i);
     min_y_set = MIN(min_y_set, j);
     max_y_set = MAX(max_y_set, j);
}

int nmb_ImageGrid::validDataRange(short* o_top, short* o_left,
                                   short* o_bottom, short*o_right){
     // if we are not the allocator then assume someone else is setting
     // values directly in the plane object without going through us and
     // so take the valid region from what the plane says
     if (!grid) {
         return plane->findValidDataRange(o_top, o_left, o_bottom, o_right);
     }
     // if no valid data:
     if (min_y_set > max_y_set || min_x_set > max_x_set)
        return -1;
     // otherwise at least one valid data point:
     *o_bottom = min_y_set; *o_top = max_y_set;
     *o_left = min_x_set; *o_right = max_x_set;
     return 0;
}

float nmb_ImageGrid::minAttainableValue() const {
           return plane->minAttainableValue();}

float nmb_ImageGrid::maxAttainableValue() const {
           return plane->maxAttainableValue();}

double nmb_ImageGrid::boundX(nmb_ImageBounds::ImageBoundPoint ibp) const
{
    if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
        ibp == nmb_ImageBounds::MIN_X_MAX_Y){
          return plane->minX();
    } else {
         return plane->maxX();
    }
}

double nmb_ImageGrid::boundY(nmb_ImageBounds::ImageBoundPoint ibp) const
{
    if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
        ibp == nmb_ImageBounds::MAX_X_MIN_Y) {
        return plane->minY();
    } else {
        return plane->maxY();
    }
}

void nmb_ImageGrid::setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x)
{
    // WARNING: this might not do what you think because
    // BCGrid has a less general notion of the image extents
/*
    if (grid == NULL) {
        fprintf(stderr,
                "Warning: nmb_ImageGrid::setBoundX failed\n");
            return;
    }

    if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
        ibp == nmb_ImageBounds::MIN_X_MAX_Y) {
        grid->setMinX(x);
    } else {
        grid->setMaxX(x);
    }
*/
    if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
        ibp == nmb_ImageBounds::MIN_X_MAX_Y) {
        plane->_grid->setMinX(x);
    } else {
        plane->_grid->setMaxX(x);
    }
}

void nmb_ImageGrid::setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y)
{
    // WARNING: this might not do what you think because
    // BCGrid has a less general notion of the image extents
/*
    if (grid == NULL) {
        fprintf(stderr,
                "Warning: nmb_ImageGrid::setBoundY failed\n");
        return;
    }
    if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
        ibp == nmb_ImageBounds::MAX_X_MIN_Y) {
        grid->setMinY(y);
    } else {
        grid->setMaxY(y);
    }
*/
    if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
        ibp == nmb_ImageBounds::MAX_X_MIN_Y) {
        plane->_grid->setMinY(y);
    } else {
        plane->_grid->setMaxY(y);
    }
}

void nmb_ImageGrid::getBounds(nmb_ImageBounds &ib)  const
{
    ib = nmb_ImageBounds(plane->minX(), plane->minY(),
                        plane->maxX(), plane->maxY());
}

void nmb_ImageGrid::setBounds(const nmb_ImageBounds &ib)
{
/*
    // WARNING: this might not do what you think because
    // BCGrid has a less general notion of the image extents
    if (grid == NULL) {
        fprintf(stderr,
                "Warning: nmb_ImageGrid::setBounds failed\n");
        return;
    }
    grid->setMinX(ib.getX(nmb_ImageBounds::MIN_X_MIN_Y));
    grid->setMinY(ib.getY(nmb_ImageBounds::MIN_X_MIN_Y));
    grid->setMaxX(ib.getX(nmb_ImageBounds::MAX_X_MAX_Y));
    grid->setMaxY(ib.getY(nmb_ImageBounds::MAX_X_MAX_Y));
*/
    plane->_grid->setMinX(ib.getX(nmb_ImageBounds::MIN_X_MIN_Y));
    plane->_grid->setMinY(ib.getY(nmb_ImageBounds::MIN_X_MIN_Y));
    plane->_grid->setMaxX(ib.getX(nmb_ImageBounds::MAX_X_MAX_Y));
    plane->_grid->setMaxY(ib.getY(nmb_ImageBounds::MAX_X_MAX_Y));
}

BCString *nmb_ImageGrid::name() {return plane->name();}
BCString *nmb_ImageGrid::unitsValue() {return plane->units();}
BCString *nmb_ImageGrid::unitsX() {return &units_x;}
BCString *nmb_ImageGrid::unitsY() {return &units_y;}

void nmb_ImageGrid::setTopoFileInfo(TopoFile &tf)
{
    d_topoFileDefaults = tf;
}

void nmb_ImageGrid::getTopoFileInfo(TopoFile &tf)
{
    tf = d_topoFileDefaults;
}


void *nmb_ImageGrid::pixelData() { return (void *)(plane->flatValueArray());}

int nmb_ImageGrid::border() { return plane->_border;}


int nmb_ImageGrid::arrayLength() 
{ 
   return ((2*border()+width())*(2*border()+height()));
}


nmb_PixelType nmb_ImageGrid::pixelType() {return NMB_FLOAT32;}

int nmb_ImageGrid::numExportFormats() {return num_export_formats;}

nmb_ListOfStrings *nmb_ImageGrid::exportFormatNames()
                {return &formatNames;}

const char *nmb_ImageGrid::exportFormatType(int type)
            {return (const char *)(export_formats_list[type]);}

int nmb_ImageGrid::exportToFile(FILE *f, const char *export_type){

    int my_export_type;
    for (my_export_type = 0; my_export_type < numExportFormats(); 
         my_export_type++){
	if (strcmp(export_type, exportFormatType(my_export_type)) == 0)
	    break;
    }
    // if didn't find a match to export_type
    if (my_export_type == numExportFormats()) {
	fprintf(stderr, "nmb_ImageGrid::Error, unknown file type: %s\n",
	    export_type);
	return -1;
    }
    else {  // we have a function for exporting this type
	if (file_exporting_function[my_export_type](f, this)) {
	    fprintf(stderr, "nmb_ImageGrid::Error writing file of type %s\n",
		export_type);
	    return -1;
	}
	return 0;
    }
}

//static 
int nmb_ImageGrid::writeTopoFile(FILE *file, nmb_ImageGrid *im)
{
 //what about microscope->d_topoFile? - should somehow be using this info here
    TopoFile tf = im->d_topoFileDefaults;
    tf.imageToTopoData((nmb_Image *)im);
    if (tf.writeTopoFile(file) < 0) {
	//error occured
	return -1;
    }
    return 0;
}

//static
int nmb_ImageGrid::writeTextFile(FILE *file, nmb_ImageGrid *im)
{
    if (im->plane->_grid->writeTextFile(file, im->plane)) {
	return -1;
    }
    return 0;
}

//static 
int nmb_ImageGrid::writePPMFile(FILE *file, nmb_ImageGrid *im)
{
    if (im->plane->_grid->writePPMFile(file, im->plane)) {
	return -1;
    }
    return 0;
}

//static 
int nmb_ImageGrid::writeSPIPFile(FILE *file, nmb_ImageGrid *im)
{
    if (im->plane->_grid->writeSPIPFile(file, im->plane)) {
	return -1;
    }
    return 0;
}

//static 
int nmb_ImageGrid::writeUNCAFile(FILE *file, nmb_ImageGrid *im)
{
    if (im->plane->_grid->writeUNCAFile(file, im->plane)) {
	return -1;
    }
    return 0;
}

const int nmb_ImageArray::num_export_formats = 0;

const char *nmb_ImageArray::export_formats_list[] = {"none"};

const nmb_ImageArray::FileExportingFunction
        nmb_ImageArray::file_exporting_function[] = {NULL};

nmb_ImageArray::nmb_ImageArray(const char *name,
                                          const char * /*units*/,
                                          short x, short y,
                                          nmb_PixelType pixType):
        nmb_Image(), 
        fData(NULL),ucData(NULL), usData(NULL), data(NULL),
        num_x(x), num_y(y), d_border(1),
        units_x("none"), units_y("none"), units("ADC"),
        my_name(name),
        d_minNonZeroValueComputed(VRPN_FALSE),
        d_minNonZeroValue(0),
        d_minValueComputed(VRPN_FALSE),
        d_minValue(0),
        d_maxValueComputed(VRPN_FALSE),
        d_maxValue(0),
        d_minValidValueComputed(VRPN_FALSE),
        d_minValidValue(0),
        d_maxValidValueComputed(VRPN_FALSE),
        d_maxValidValue(0),
        d_imagePosition(0.0, 0.0, 1.0, 1.0),
        d_pixelType(pixType)
{
    min_x_set = MAXSHORT; min_y_set = MAXSHORT;
    max_x_set = -MAXSHORT; max_y_set = -MAXSHORT;
    int array_size = arrayLength();
    switch (d_pixelType) {
      case NMB_FLOAT32:
        data = new vrpn_float32[array_size];
        break;
      case NMB_UINT8:
        data = new vrpn_uint8[array_size];
        break;
      case NMB_UINT16:
        data = new vrpn_uint16[array_size];
        break;
      default:
        fprintf(stderr, "nmb_ImageArray::nmb_ImageArray:"
           " Error, unknown type\n");
        break;
    }
    if (!data) {
        fprintf(stderr, "nmb_ImageArray::nmb_ImageArray:"
                        " Error, out of memory\n");
        return;
    }

    fData = (vrpn_float32 *)data;
    ucData = (vrpn_uint8 *)data;
    usData = (vrpn_uint16 *)data;

    int j;
    switch (d_pixelType) {
      case NMB_FLOAT32:
        for (j = 0; j < array_size; j++) {
           fData[j] = 0;
        }
        break;
      case NMB_UINT8:
        for (j = 0; j < array_size; j++) {
           ucData[j] = 0;
        }
        break;
      case NMB_UINT16:
        for (j = 0; j < array_size; j++) {
           usData[j] = 0;
        }
        break;
      default:
        fprintf(stderr, "nmb_ImageArray::nmb_ImageArray:"
           " Error, unknown type\n");
        break;
    }

    for (int i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }
}

nmb_ImageArray::nmb_ImageArray(nmb_Image *im)
{
  nmb_ImageArray(im->name()->Characters(),
                  im->unitsValue()->Characters(),
                  im->width(), im->height(), im->pixelType());
  units_x = *(im->unitsX());
  units_y = *(im->unitsY());
  int i,j;
  for (i = 0; i < width(); i++) {
    for (j = 0; j < height(); j++) {
      setValue(i,j, im->getValue(i,j));
    }
  }
  im->validDataRange(&max_y_set, &min_x_set, &min_y_set, &max_x_set);
}

nmb_ImageArray::~nmb_ImageArray() {
  if (data) {
    delete [] data;
    data = NULL;
    fData = NULL;
    ucData = NULL;
    usData = NULL;
  }
}

int nmb_ImageArray::width() const {return num_x;}

int nmb_ImageArray::height() const {return num_y;}

int nmb_ImageArray::border() {return d_border;}

int nmb_ImageArray::arrayLength() 
{
   return ((2*d_border+num_x)*(2*d_border+num_y));
}

nmb_PixelType nmb_ImageArray::pixelType() {return d_pixelType;}

void *nmb_ImageArray::pixelData() {return data;}

float nmb_ImageArray::getValue(int i, int j) const
{
  int index = arrIndex(i,j);
  switch (d_pixelType) {
    case NMB_FLOAT32:
      return (float)fData[index];
    case NMB_UINT8:
      return (float)ucData[index];
    case NMB_UINT16:
      return (float)usData[index];
    default:
      fprintf(stderr, "nmb_ImageArray::getValue:"
           " Error, unknown type\n");
      break;
  }
  return 0.0;
}

void nmb_ImageArray::setValue(int i, int j, float val)
{
  float clampedValue;
  if (val > maxAttainableValue()) {
      clampedValue = maxAttainableValue(); 
  } else if (val < minAttainableValue()) {
      clampedValue = minAttainableValue();
  } else {
      clampedValue = val;
  }
  switch (d_pixelType) {
    case NMB_FLOAT32:
      fData[arrIndex(i,j)] = (float)clampedValue;
      break;
    case NMB_UINT8:
      ucData[arrIndex(i,j)] = (vrpn_uint8)clampedValue;
      break;
    case NMB_UINT16:
      usData[arrIndex(i,j)] = (vrpn_uint16)clampedValue;
      break;
    default:
      fprintf(stderr, "nmb_ImageArray::setValue:"
           " Error, unknown type\n");
      break;
  }

  d_minValueComputed = VRPN_FALSE;
  d_maxValueComputed = VRPN_FALSE;
  d_minNonZeroValueComputed = VRPN_FALSE;
  d_minValidValueComputed = VRPN_FALSE;
  d_maxValidValueComputed = VRPN_FALSE;
} 

void nmb_ImageArray::setLine(int line, void *line_data) {
  switch(d_pixelType) {
    case NMB_FLOAT32:
      memcpy(&(fData[(line+d_border)*(num_x+2*d_border)+d_border]),
               line_data, num_x*sizeof(vrpn_float32));
      break;
    case NMB_UINT8:
      memcpy(&(ucData[(line+d_border)*(num_x+2*d_border)+d_border]),
               line_data, num_x*sizeof(vrpn_uint8));
      break;
    case NMB_UINT16:
      memcpy(&(usData[(line+d_border)*(num_x+2*d_border)+d_border]),
               line_data, num_x*sizeof(vrpn_uint16));
      break;
    default:
      fprintf(stderr, "nmb_ImageArray::getLine:"
           " Error, unknown type\n");
      break;
  }
}

void nmb_ImageArray::setImage(void *newdata) {
  int i;
  switch(d_pixelType) {
    case NMB_FLOAT32:
      for (i = 0; i < num_y; i++){
        setLine(i, &(((vrpn_float32*)newdata)[num_x*i]));
      }
      break;
    case NMB_UINT8:
      for (i = 0; i < num_y; i++){
        setLine(i, &(((vrpn_uint8*)newdata)[num_x*i]));
      }
      break;
    case NMB_UINT16:
      for (i = 0; i < num_y; i++){
        setLine(i, &(((vrpn_uint16*)newdata)[num_x*i]));
      }
      break;
    default:
      fprintf(stderr, "nmb_ImageArray::setImage:"
           " Error, unknown type\n");
      break;
  }
}

int nmb_ImageArray::validDataRange(short* o_top, short* o_left,
                                   short* o_bottom, short*o_right){
     // if no valid data:
     if (min_y_set > max_y_set || min_x_set > max_x_set)
        return -1;
     // otherwise at least one valid data point:
     *o_bottom = min_y_set; *o_top = max_y_set;
     *o_left = min_x_set; *o_right = max_x_set;
     return 0;
}

float nmb_ImageArray::maxAttainableValue() const 
{
  switch(d_pixelType) {
    case NMB_FLOAT32:
      return FLT_MAX;
    case NMB_UINT8:
      return 255.0f;
    case NMB_UINT16:
      return 65535.0f;
    default:
      fprintf(stderr, "nmb_ImageArray::maxAttainableValue:"
           " Error, unknown type\n");
      return 0.0;
  }
}

float nmb_ImageArray::minAttainableValue() const
{
  switch(d_pixelType) {
    case NMB_FLOAT32:
      return -FLT_MAX;
    case NMB_UINT8:
      return 0.0f;
    case NMB_UINT16:
      return 0.0f;
    default:
      fprintf(stderr, "nmb_ImageArray::minAttainableValue:"
           " Error, unknown type\n");
      return 0.0;
  }
}

float nmb_ImageArray::maxValue()
{
    if (!d_maxValueComputed) {
        d_maxValue = getValue(0,0);
        for (int i = 0; i < num_x; i++){
            for (int j = 0; j < num_y; j++){
                float val = getValue(i,j);
                if (val > d_maxValue) {
		    d_maxValue = val;
                }
            }
        }
        d_maxValueComputed = VRPN_TRUE;
    }
    return d_maxValue;
}

float nmb_ImageArray::minValue() 
{
    if (!d_minValueComputed) {
        d_minValue = getValue(0,0);
        for (int i = 0; i < num_x; i++){
            for (int j = 0; j < num_y; j++){
                float val = getValue(i,j);
                if (val < d_minValue) {
                    d_minValue = val;
                }
            }
        }
        d_minValueComputed = VRPN_TRUE;
    }
    return d_minValue;
}

int nmb_ImageArray::normalize() {
  float min = minValue();
  float range = (maxValue() - min);
  float inv_range;
  if (range == 0) {
     inv_range = 0;
  } else {
     inv_range = 1.0/range;
  }

  // special case for floats - normalize to the range 0..1
  if (d_pixelType == NMB_FLOAT32) {
    for (int i = 0; i < num_x; i++) {
      for (int j = 0; j < num_y; j++) {
        setValue(i,j, (inv_range*(getValue(i,j)-min)));
      }
    }
  } else {
    assert(minAttainableValue() == 0.0);
    for (int i = 0; i < num_x; i++) {
      for (int j = 0; j < num_y; j++) {
        setValue(i,j,
            (inv_range * (getValue(i,j) - min) * maxAttainableValue()));
      }
    }
  }

  return 0;
}

float nmb_ImageArray::maxValidValue() {
    short top, left, bottom, right;
    if (validDataRange(&top, &left, &bottom, &right)) {
        return 0;
    }
    if ((top == height()-1) && (bottom == 0) && 
        (right == width()-1) && (left == 0)) {
        return maxValue();
    } else if (!d_maxValidValueComputed) {
        int i,j;
        d_maxValidValue = getValue(left, bottom);
        float val;
        for (i = left; i <= right; i++) {
            for (j = bottom; j <= top; j++) {
                val = getValue(i,j);
                if (val > d_maxValidValue) d_maxValidValue = val;
            }
        }
        d_maxValidValueComputed = VRPN_TRUE;
    }
    return d_maxValidValue;
}

float nmb_ImageArray::minValidValue() {
    short top, left, bottom, right;
    if (validDataRange(&top, &left, &bottom, &right)) {
        return 0;
    }
    if ((top == height()-1) && (bottom == 0) &&
        (right == width()-1) && (left == 0)) {
        return minValue();
    } else if (!d_maxValidValueComputed) {
        int i,j;
        d_minValidValue = getValue(left, bottom);
        float val;
        for (i = left; i <= right; i++) {
            for (j = bottom; j <= top; j++) {
                val = getValue(i,j);
                if (val < d_minValidValue) d_minValidValue = val;
            }
        }
        d_minValidValueComputed = VRPN_TRUE;
    }
    return d_minValidValue;
}

float nmb_ImageArray::maxNonZeroValue() {
    return maxValidValue();
}

float nmb_ImageArray::minNonZeroValue() {
    if (!d_minNonZeroValueComputed) {
      int i,j;
      float val = 0, d_minNonZeroValue = 0;
      for (i = 0; i < width(); i++) {
        for (j = 0; j < height(); j++) {
          val = getValue(i,j);
          if (val != 0 && val < d_minNonZeroValue) {
            d_minNonZeroValue = val;
          }
        }
      }
      d_minNonZeroValueComputed = VRPN_TRUE;
    }
    return d_minNonZeroValue;
}

double nmb_ImageArray::boundX(
                    nmb_ImageBounds::ImageBoundPoint ibp) const
{
    return d_imagePosition.getX(ibp);
}

double nmb_ImageArray::boundY(
                   nmb_ImageBounds::ImageBoundPoint ibp) const
{
    return d_imagePosition.getY(ibp);
}

void nmb_ImageArray::setBoundX(
    nmb_ImageBounds::ImageBoundPoint ibp,
    double x)
{
    d_imagePosition.setX(ibp, x);
}

void nmb_ImageArray::setBoundY(
    nmb_ImageBounds::ImageBoundPoint ibp,
    double y)
{
    d_imagePosition.setY(ibp, y);
}

void nmb_ImageArray::setBounds(const nmb_ImageBounds & ib) 
{
    d_imagePosition = ib;
}

void nmb_ImageArray::getBounds(nmb_ImageBounds &ib)  const
{
    ib = d_imagePosition;
}

BCString *nmb_ImageArray::name() {return &my_name;}
BCString *nmb_ImageArray::unitsValue() {return &units;}
BCString *nmb_ImageArray::unitsX() {return &units_x;}
BCString *nmb_ImageArray::unitsY() {return &units_y;}

int nmb_ImageArray::numExportFormats() {return 0;}
nmb_ListOfStrings *nmb_ImageArray::exportFormatNames() 
{return NULL;}
const char *nmb_ImageArray::exportFormatType(int) {return NULL;}

int nmb_ImageArray::exportToFile(FILE *f, const char *export_type){

    int my_export_type;
    for (my_export_type = 0; my_export_type < numExportFormats();
         my_export_type++){
        if (strcmp(export_type, exportFormatType(my_export_type)) == 0)
            break;
    }
    // if didn't find a match to export_type
    if (my_export_type == numExportFormats()) {
        fprintf(stderr, "nmb_ImageArray::Error, unknown file type: %s\n",
            export_type);
        return -1;
    }
    else {  // we have a function for exporting this type
        if (file_exporting_function[my_export_type](f, this)) {
            fprintf(stderr, "nmb_ImageArray::Error writing file of type %s\n",
                export_type);
            return -1;
        }
        return 0;
    }
}

nmb_ImageList::nmb_ImageList(nmb_ListOfStrings *namelist) :
       num_images(0),
       imageNames(namelist)
{

}

nmb_ImageList::nmb_ImageList(nmb_ListOfStrings *namelist,
                             const char **file_names, int num_files,
                             TopoFile &topoFile) :
       num_images(0),
       imageNames(namelist)
{
    //printf("nmb_ImageList::nmb_ImageList building list\n");

    for (int i = 0; i < num_files; i++) {
//          printf("nmb_ImageList::nmb_ImageList - creating grid for file %s\n",
//  		file_names[i]);
	BCGrid *g = new BCGrid(0, 0, 0, 1, 0, 1,
			READ_FILE, file_names[i], topoFile);
	nmb_Image *im;
	BCPlane *p;
//          printf("file contained: \n");
	for (p = g->head(); p != NULL; p = p->next()){
	    im = new nmb_ImageGrid(p);
            im->setTopoFileInfo(topoFile);
	    addImage(im);
//              printf("  %s\n", (const char *)(*(im->name())));
//  	    printf("nmb_Image min,max=%f,%f\n", im->minValue(),im->maxValue());
	}
    }
}

nmb_ImageList::~nmb_ImageList()
{
    for (int i = 0; i < num_images; i++) {
        images[i]->num_referencing_lists--;
        if (images[i]->num_referencing_lists == 0) {
	    delete images[i];
        }
    }
}

int nmb_ImageList::addImage(nmb_Image *im)
{
    // don't add this if name is not unique
    if (getImageByName(*(im->name())))
	return -2;
    // don't add if list is full
    BCString name = (*(im->name()));
    if (num_images == NMB_MAX_IMAGELIST_LENGTH ||
		(imageNames->addEntry(name))) return -1;
    images[num_images] = im;
    num_images++;
    im->num_referencing_lists++;
    return 0;
}

nmb_Image *nmb_ImageList::removeImageByName(BCString name) {
    int i;
    nmb_Image *im = getImageByName(name, i);
    if (im == NULL) return NULL;
    // getImageByName() succeeds ==> num_images >= 1
    images[i] = images[num_images-1];
    imageNames->deleteEntry((const char *)(*(im->name())));
    num_images--;
    im->num_referencing_lists--;
    return im;
}

nmb_Image *nmb_ImageList::getImageByName(BCString name, int &index) {
    for (int i = 0; i < num_images; i++) {
        if (*(images[i]->name()) == name){
             index = i;
             return images[i];
        }
    }
    return NULL;
}
