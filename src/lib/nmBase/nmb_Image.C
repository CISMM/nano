#include "Topo.h" // for TopoFile
#include "nmb_Image.h"

extern TopoFile GTF;

nmb_ImageBounds::nmb_ImageBounds() 
{
    for (int i = 0; i < 4; i++){
        x[i] = 0.0; y[i] = 0.0;
    }
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

    double val = (1-i_fp)*(1-j_fp)*getValue(i_ip, j_ip);
    val += (1-i_fp)*j_fp*getValue(i_ip, j_ip+1);
    val += i_fp*(1-j_fp)*getValue(i_ip+1,j_ip);
    val += i_fp*j_fp*getValue(i_ip+1,j_ip+1);
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

/* This function assumes that (i,j) are coordinates for the basis vectors (u,v)
   where u = pnt_max_i_min_j - pnt_min_i_min_j
         v = pnt_min_i_max_j - pnt_min_i_min_j
   pnt_max_i_min_j, pnt_min_i_min_j, pnt_min_i_max_j are the world positions of
   three of the corners of the image
*/
void nmb_Image::pixelToWorld(const double i, const double j,
                  double &x, double &y) const {
        // just bilinear interpolation:
        double y_frac = j/(double)height();
        double x_frac = i/(double)width();
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

/* This function assumes that (i,j) are coordinates for the basis vectors (u,v)
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

const int nmb_ImageGrid::num_export_formats = 5;
const char *nmb_ImageGrid::export_formats_list[] = {	"Topometrix",
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
}

nmb_ImageGrid::nmb_ImageGrid(BCPlane *p):nmb_Image()
{
    // WARNING: assumes (non-zero value <==> value was set) as
    // did BCPlane::findValidDataRange()

    plane = p;
    grid = NULL;
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

float nmb_ImageGrid::maxValue() const {return plane->maxValue();}

float nmb_ImageGrid::minValue() const {return plane->minValue();}

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

vrpn_uint8 *nmb_ImageGrid::rawDataUnsignedByte() { return NULL;}

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
    GTF.imageToTopoData((nmb_Image *)im);
    if (GTF.writeTopoFile(file) < 0) {
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


const int nmb_Image8bit::num_export_formats = 0;
const char *nmb_Image8bit::export_formats_list[] = {"none"};
const nmb_Image8bit::FileExportingFunction
        nmb_Image8bit::file_exporting_function[] = {NULL};


nmb_Image8bit::nmb_Image8bit(const char *name, const char *units, 
	short x, short y):
        nmb_Image(), num_x(x), num_y(y),
        units_x("none"), units_y("none"), units("ADC"),
        my_name(name)
{
    min_x_set = MAXSHORT; min_y_set = MAXSHORT;
    max_x_set = -MAXSHORT; max_y_set = -MAXSHORT;
    data = new vrpn_uint8[x*y];
    if (!data) {
	fprintf(stderr, "nmb_Image8bit::nmb_Image8bit: Error, out of memory\n");
    }
    for (int i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }
}

nmb_Image8bit::~nmb_Image8bit() {if (data) delete [] data;}

int nmb_Image8bit::width() const {return num_x;}

int nmb_Image8bit::height() const {return num_y;}

vrpn_uint8 *nmb_Image8bit::rawDataUnsignedByte() {return data;}

float nmb_Image8bit::getValue(int i, int j) const
        {return (float)data[i+j*num_x];}

void nmb_Image8bit::setValue(int i, int j, float val)
{
    if (val > 255)
        data[i+j*num_x] = 255;
    else if (val < 0)
        data[i+j*num_x] = 0;
    else
        data[i+j*num_x] = (vrpn_uint8)val;
}

void nmb_Image8bit::setLine(int line, vrpn_uint8 *line_data) {
    memcpy(&(data[line*num_x]), line_data, num_x);
}

void nmb_Image8bit::setImage(vrpn_uint8 *newdata) {
    memcpy(data, newdata, num_x*num_y);
}

int nmb_Image8bit::validDataRange(short* o_top, short* o_left,
                                   short* o_bottom, short*o_right){
     // if no valid data:
     if (min_y_set > max_y_set || min_x_set > max_x_set)
        return -1;
     // otherwise at least one valid data point:
     *o_bottom = min_y_set; *o_top = max_y_set;
     *o_left = min_x_set; *o_right = max_x_set;
     return 0;
}

float nmb_Image8bit::maxValue() const {return 255.0;}

float nmb_Image8bit::minValue() const {return 0.0;}

float nmb_Image8bit::maxAttainableValue() const {return 255.0;}

float nmb_Image8bit::minAttainableValue() const {return 0.0;}

double nmb_Image8bit::boundX(nmb_ImageBounds::ImageBoundPoint ibp) const
{
    if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
        ibp == nmb_ImageBounds::MIN_X_MAX_Y){
          return 0.0;
    } else {
          return (double)num_x;
    }
}

double nmb_Image8bit::boundY(nmb_ImageBounds::ImageBoundPoint ibp) const
{
    if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
        ibp == nmb_ImageBounds::MAX_X_MIN_Y) {
          return 0.0;
    } else {
          return (double)num_y;
    }
}

void nmb_Image8bit::setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x){}
void nmb_Image8bit::setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y){}

void nmb_Image8bit::setBounds(const nmb_ImageBounds &ib) {}

void nmb_Image8bit::getBounds(nmb_ImageBounds &ib)  const
{
    ib = nmb_ImageBounds(0.0, 0.0, (double)num_x, (double)num_y);
}

BCString *nmb_Image8bit::name() {return &my_name;}
BCString *nmb_Image8bit::unitsValue() {return &units;}
BCString *nmb_Image8bit::unitsX() {return &units_x;}
BCString *nmb_Image8bit::unitsY() {return &units_y;}

int nmb_Image8bit::numExportFormats() {return 0;}
nmb_ListOfStrings *nmb_Image8bit::exportFormatNames() {return NULL;}
const char *nmb_Image8bit::exportFormatType(int type)
         {return NULL;}

int nmb_Image8bit::exportToFile(FILE *f, const char *export_type){

    int my_export_type;
    for (my_export_type = 0; my_export_type < numExportFormats();
         my_export_type++){
        if (strcmp(export_type, exportFormatType(my_export_type)) == 0)
            break;
    }
    // if didn't find a match to export_type
    if (my_export_type == numExportFormats()) {
        fprintf(stderr, "nmb_Image8bit::Error, unknown file type: %s\n",
            export_type);
        return -1;
    }
    else {  // we have a function for exporting this type
        if (file_exporting_function[my_export_type](f, this)) {
            fprintf(stderr, "nmb_Image8bit::Error writing file of type %s\n",
                export_type);
            return -1;
        }
        return 0;
    }
}

nmb_ImageList::nmb_ImageList(const char **file_names, int num_files)
{
    printf("nmb_ImageList::nmb_ImageList building list\n");
    num_images = 0;

    for (int i = 0; i < num_files; i++) {
        printf("nmb_ImageList::nmb_ImageList - creating grid for file %s\n",
		file_names[i]);
	BCGrid *g = new BCGrid(0, 0, 0, 1, 0, 1,
			READ_FILE, file_names[i]);
	nmb_Image *im;
	BCPlane *p;
        printf("file contained: \n");
	for (p = g->head(); p != NULL; p = p->next()){
	    im = new nmb_ImageGrid(p);
	    addImage(im);
            printf("  %s\n", (const char *)(*(im->name())));
	    printf("nmb_Image min,max=%f,%f\n", im->minValue(),im->maxValue());
	}
    }
}

nmb_ImageList::~nmb_ImageList()
{
    for (int i = 0; i < num_images; i++) {
	delete images[i];
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
		(imageNames.addEntry(name))) return -1;
	images[num_images] = im;
	num_images++;
	return 0;
}

nmb_Image *nmb_ImageList::removeImageByName(BCString name) {
    int i;
    nmb_Image *im = getImageByName(name, i);
    if (im == NULL) return NULL;
    // getImageByName() succeeds ==> num_images >= 1
    images[i] = images[num_images-1];
    imageNames.deleteEntry((const char *)(*(im->name())));
    num_images--;
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
