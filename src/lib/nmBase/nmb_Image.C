#include "Topo.h" // for TopoFile
#include "nmb_Image.h"

extern TopoFile GTF;

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

int nmb_ImageGrid::exportToFile(FILE *f, const char *export_type){

    int my_export_type;
    for (my_export_type = 0; my_export_type < numExportFormats(); my_export_type++){
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
