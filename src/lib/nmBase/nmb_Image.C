#include "Topo.h" // for TopoFile
#include "nmb_Image.h"

#ifndef _WIN32
#include <limits.h> // for FLT_MAX
#else
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
        double y_frac = j/((double)height() - 1);
        double x_frac = i/((double)width() - 1);
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

*/

void nmb_Image::getWorldToImageTransform(double *matrix44)
{
    /* What this function is doing:
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
    min_x_set = MAXSHORT; min_y_set = MAXSHORT;
    max_x_set = -MAXSHORT; max_y_set = -MAXSHORT;
  
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
    min_x_set = MAXSHORT; min_y_set = MAXSHORT;
     max_x_set = -MAXSHORT; max_y_set = -MAXSHORT;

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
}

void nmb_ImageGrid::setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y)
{
    // WARNING: this might not do what you think because
    // BCGrid has a less general notion of the image extents
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
}

void nmb_ImageGrid::getBounds(nmb_ImageBounds &ib)  const
{
    ib = nmb_ImageBounds(plane->minX(), plane->minY(),
                        plane->maxX(), plane->maxY());
}

void nmb_ImageGrid::setBounds(const nmb_ImageBounds &ib)
{
    // WARNING: this might not do what you think because
    // BCGrid has a less general notion of the image extents
    if (grid == NULL) {
        fprintf(stderr,
                "Warning: nmb_ImageGrid::setBounds failed\n");
    }
    grid->setMinX(ib.getX(nmb_ImageBounds::MIN_X_MIN_Y));
    grid->setMinY(ib.getY(nmb_ImageBounds::MIN_X_MIN_Y));
    grid->setMaxX(ib.getX(nmb_ImageBounds::MAX_X_MAX_Y));
    grid->setMaxY(ib.getY(nmb_ImageBounds::MAX_X_MAX_Y));
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

template <class PixelType>
const int nmb_ImageArray<PixelType>::num_export_formats = 0;

template <class PixelType>
const char *nmb_ImageArray<PixelType>::export_formats_list[] = {"none"};

template <class PixelType>
const nmb_ImageArray<PixelType>::FileExportingFunction
        nmb_ImageArray<PixelType>::file_exporting_function[] = {NULL};

nmb_ImageArray<vrpn_uint8>::nmb_ImageArray(const char *name,
                                          const char * /*units*/,
                                          short x, short y):
        nmb_Image(), num_x(x), num_y(y),
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
        d_imagePosition(0.0, 0.0, 1.0, 1.0)
{
    min_x_set = MAXSHORT; min_y_set = MAXSHORT;
    max_x_set = -MAXSHORT; max_y_set = -MAXSHORT;
    data = new vrpn_uint8[x*y];
    if (!data) {
        fprintf(stderr, "nmb_ImageArray::nmb_ImageArray:"
                        " Error, out of memory\n");
    }
    for (int i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }
}

nmb_ImageArray<vrpn_uint16>::nmb_ImageArray(const char *name,
                                          const char * /*units*/,
                                          short x, short y):
        nmb_Image(), num_x(x), num_y(y),
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
        d_imagePosition(0.0, 0.0, 1.0, 1.0)
{
    min_x_set = MAXSHORT; min_y_set = MAXSHORT;
    max_x_set = -MAXSHORT; max_y_set = -MAXSHORT;
    data = new vrpn_uint16[x*y];
    if (!data) {
        fprintf(stderr, "nmb_ImageArray::nmb_ImageArray:"
                        " Error, out of memory\n");
    }
    for (int i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }
}

nmb_ImageArray<vrpn_float32>::nmb_ImageArray(const char *name,
                                          const char * /*units*/,
                                          short x, short y):
        nmb_Image(), num_x(x), num_y(y),
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
        d_imagePosition(0.0, 0.0, 1.0, 1.0)
{
    min_x_set = MAXSHORT; min_y_set = MAXSHORT;
    max_x_set = -MAXSHORT; max_y_set = -MAXSHORT;
    data = new vrpn_float32[x*y];
    if (!data) {
        fprintf(stderr, "nmb_ImageArray::nmb_ImageArray:"
                        " Error, out of memory\n");
    }
    for (int i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }
}

template <class PixelType>
nmb_ImageArray<PixelType>::nmb_ImageArray(nmb_Image *im)
{
  nmb_ImageArray<PixelType>(im->name()->Characters(),
                  im->unitsValue()->Characters(),
                  im->width(), im->height());
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

template <class PixelType>
nmb_ImageArray<PixelType>::~nmb_ImageArray() {if (data) delete [] data;}

template <class PixelType>
int nmb_ImageArray<PixelType>::width() const {return num_x;}

template <class PixelType>
int nmb_ImageArray<PixelType>::height() const {return num_y;}

// I don't think its possible to write a default function to return the
// right type for pixelType() so I disabled this feature by default and
// made some special ones for the types we will use
template <class PixelType>
void *nmb_ImageArray<PixelType>::pixelData() {return NULL;}

template <class PixelType>
nmb_PixelType nmb_ImageArray<PixelType>::pixelType() {return NMB_NONE;}

void *nmb_ImageArray<vrpn_uint8>::pixelData() {return data;}

nmb_PixelType nmb_ImageArray<vrpn_uint8>::pixelType() 
	{return NMB_UINT8;}

void *nmb_ImageArray<vrpn_uint16>::pixelData() {return data;}

nmb_PixelType nmb_ImageArray<vrpn_uint16>::pixelType() 
        {return NMB_UINT16;}

void *nmb_ImageArray<vrpn_float32>::pixelData() {return data;}

nmb_PixelType nmb_ImageArray<vrpn_float32>::pixelType() 
        {return NMB_FLOAT32;}

template <class PixelType>
float nmb_ImageArray<PixelType>::getValue(int i, int j) const
        {return (float)(data[i+j*num_x]);}

template <class PixelType>
void nmb_ImageArray<PixelType>::setValue(int i, int j, float val)
{
    if (val > maxAttainableValue()) {
        data[i+j*num_x] = maxAttainableValue();
    } else if (val < minAttainableValue()) {
        data[i+j*num_x] = minAttainableValue();
    } else {
        data[i+j*num_x] = (PixelType)val;
    }
    d_minValueComputed = VRPN_FALSE;
    d_maxValueComputed = VRPN_FALSE;
    d_minNonZeroValueComputed = VRPN_FALSE;
    d_minValidValueComputed = VRPN_FALSE;
    d_maxValidValueComputed = VRPN_FALSE;
} 

template <class PixelType>
void nmb_ImageArray<PixelType>::setLine(int line, void *line_data) {
    memcpy(&(data[line*num_x]), line_data, num_x*sizeof(PixelType));
}

template <class PixelType>
void nmb_ImageArray<PixelType>::setImage(void *newdata) {
    memcpy(data, newdata, num_x*num_y*sizeof(PixelType));
}

template <class PixelType>
int nmb_ImageArray<PixelType>::validDataRange(short* o_top, short* o_left,
                                   short* o_bottom, short*o_right){
     // if no valid data:
     if (min_y_set > max_y_set || min_x_set > max_x_set)
        return -1;
     // otherwise at least one valid data point:
     *o_bottom = min_y_set; *o_top = max_y_set;
     *o_left = min_x_set; *o_right = max_x_set;
     return 0;
}

template <class PixelType>
float nmb_ImageArray<PixelType>::maxAttainableValue() const {return 0.0;}
template <class PixelType>
float nmb_ImageArray<PixelType>::minAttainableValue() const {return 0.0;}

float nmb_ImageArray<vrpn_uint8>::maxAttainableValue() const {return 255.0f;}
float nmb_ImageArray<vrpn_uint8>::minAttainableValue() const {return 0.0f;}

float nmb_ImageArray<vrpn_uint16>::maxAttainableValue() const {return 65535.0f;}
float nmb_ImageArray<vrpn_uint16>::minAttainableValue() const {return 0.0f;}

float nmb_ImageArray<vrpn_float32>::maxAttainableValue() const {return FLT_MAX;}
float nmb_ImageArray<vrpn_float32>::minAttainableValue() const {return -FLT_MAX;}

template <class PixelType>
float nmb_ImageArray<PixelType>::maxValue()
{
    int i;
    if (!d_maxValueComputed) {
        d_maxValue = (float)data[0];
        for (int i = 1; i < num_x*num_y; i++){
            if (data[i] > d_maxValue) {
		d_maxValue = data[i];
            }
        }
        d_maxValueComputed = VRPN_TRUE;
    }
    return d_maxValue;
}

template <class PixelType>
float nmb_ImageArray<PixelType>::minValue() 
{
    if (!d_minValueComputed) {
        d_minValue = (float)data[0];
        for (int i = 1; i < num_x*num_y; i++){
            if (data[i] < d_maxValue) {
                d_minValue = data[i];
            }
        }
        d_minValueComputed = VRPN_TRUE;
    }
    return d_minValue;
}

template <class PixelType>
float nmb_ImageArray<PixelType>::maxValidValue() {
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

template <class PixelType>
float nmb_ImageArray<PixelType>::minValidValue() {
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

template <class PixelType>
float nmb_ImageArray<PixelType>::maxNonZeroValue() {
    return maxValidValue();
}

template <class PixelType>
float nmb_ImageArray<PixelType>::minNonZeroValue() {
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

template <class PixelType>
double nmb_ImageArray<PixelType>::boundX(
                    nmb_ImageBounds::ImageBoundPoint ibp) const
{
    return d_imagePosition.getX(ibp);
}

template <class PixelType>
double nmb_ImageArray<PixelType>::boundY(
                   nmb_ImageBounds::ImageBoundPoint ibp) const
{
    return d_imagePosition.getY(ibp);
}

template <class PixelType>
void nmb_ImageArray<PixelType>::setBoundX(
    nmb_ImageBounds::ImageBoundPoint ibp,
    double x)
{
    d_imagePosition.setX(ibp, x);
}

template <class PixelType>
void nmb_ImageArray<PixelType>::setBoundY(
    nmb_ImageBounds::ImageBoundPoint ibp,
    double y)
{
    d_imagePosition.setY(ibp, y);
}

template <class PixelType>
void nmb_ImageArray<PixelType>::setBounds(const nmb_ImageBounds & ib) 
{
    d_imagePosition = ib;
}

template <class PixelType>
void nmb_ImageArray<PixelType>::getBounds(nmb_ImageBounds &ib)  const
{
    ib = d_imagePosition;
}

template <class PixelType>
BCString *nmb_ImageArray<PixelType>::name() {return &my_name;}
template <class PixelType>
BCString *nmb_ImageArray<PixelType>::unitsValue() {return &units;}
template <class PixelType>
BCString *nmb_ImageArray<PixelType>::unitsX() {return &units_x;}
template <class PixelType>
BCString *nmb_ImageArray<PixelType>::unitsY() {return &units_y;}

int nmb_ImageArray<vrpn_uint8>::numExportFormats() {return 0;}
nmb_ListOfStrings *nmb_ImageArray<vrpn_uint8>::exportFormatNames() 
{return NULL;}
const char *nmb_ImageArray<vrpn_uint8>::exportFormatType(int) {return NULL;}

int nmb_ImageArray<vrpn_uint16>::numExportFormats() {return 0;}
nmb_ListOfStrings *nmb_ImageArray<vrpn_uint16>::exportFormatNames() 
{return NULL;}
const char *nmb_ImageArray<vrpn_uint16>::exportFormatType(int) {return NULL;}

int nmb_ImageArray<vrpn_float32>::numExportFormats() {return 0;}
nmb_ListOfStrings *nmb_ImageArray<vrpn_float32>::exportFormatNames() 
{return NULL;}
const char *nmb_ImageArray<vrpn_float32>::exportFormatType(int) {return NULL;}

template <class PixelType>
int nmb_ImageArray<PixelType>::exportToFile(FILE *f, const char *export_type){

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
