
/* Program:  corescribe

   Takes an image and a list of points in the image, and draws a
   line on the image linking the points in order.

   Inputs:   An image stream containing a single image.
             A text file containing one or more lists of points (as
	     space-separated triples, the third value being the
	     characteristic radius of the core at the given point, one
	     point per line).

   Outputs:  A LONG image. 

   Author:   Mark Foskey

   Date:     November 18, 1998

   Revisions:

   Usage:    corescribe [options] instream outstream

   Options:  None.

   1.0 Outputs in LONG type.

*/

#define D_STDLIB
#define D_IOSTREAM
#define D_FSTREAM
#define D_Option
#define D_LongImage
#define D_ImageStream
#define D_IntensRange

#define D_CoreList
#define D_cores

#include <imprelud.h>
#include "draw_core_list.h"

#define DEFAULT_TICK_SPACING 10.0

void input_cores(char* file, CoreList* core_list_p);

status_t 
main (int argc, char *argv[]) 
{

    //--------------------------------------------------------------
    // Parse options.  See Options(3I) for details, or the GPFLib tutorial.

    double tick_spacing = DEFAULT_TICK_SPACING;

    // A new option can be added simply by adding a a line like the
    // first constructor call below.
    Option options[] = {

	// Spacing of tick marks.
	Option(&tick_spacing, 		// Address of variable to be set
	       DEFAULT_TICK_SPACING,    // Default value
	       "ticks",			// option name
	       "Spacing in pixels between tick marks."),
					// Description for help messages

	// usr/Image has standard options.  The following lines set
	// the text strings to respond to those options.
	Option(Option::eAuthor, "Mark Foskey"),
	Option(Option::eUsage, "corescribe [options] cores-file "
	       "input-image output_image"),

	// Terminate list of options
	Option()
    };

    ParseOptions(argc, argv, options);
    if (argc < 4) {
	OptionUsage("There are missing arguments.");
    }

    //--------------------------------------------------------------
    // Input cores.

    CoreList core_list;
    input_cores(argv[1], &core_list);

    //--------------------------------------------------------------
    // Input main image.  Also go ahead and open the output stream so
    // we don't waste time processing if that should fail.

    ImageStream in_stream(argv[2]);
    ImageStream out_stream(argv[3], CREATE);

    LongImage main_image;

    if ((in_stream(0) >> image >> main_image) == INVALID) {
	cout << "Error reading image.\n";
	exit(1);
    }

    //--------------------------------------------------------------
    // Draw the cores on the image.

    IntensRange mi_range(main_image);

    draw_core_list(core_list, main_image, tick_spacing, mi_range);

    //--------------------------------------------------------------
    // Output the image.

    if (out_stream.set_range(mi_range) == INVALID) {
	cout << "Error writing intensity range.\n";
	exit(1);
    }

    out_stream(0) << image << release(bTRUE) << main_image;
    if (out_stream.status() == INVALID) {
	cout << "Error writing image.\n";
	exit(1);
    }

}

// input_cores()
// Reads from the file 'file' into *core_list_p.  The file is an ascii
// file with a simple structure: The first line gives the number of
// cores.  The next line gives the number n of core points in the
// first core, and the following n lines are the points themselves.
// After the first core, the other cores are listed in the same
// format, without any blank lines.  Each core point is a
// space-delimited triple of decimal reals, one triple per line,
// representing x, y, and the scale s, respectively.
//
// Only the x and y values are stored into *core_list_p.
void
input_cores(char* file, CoreList* core_list_p)
{
    ifstream    cores(file);
    if (!cores) {
	cerr << "Cores file " << file << " did not open.\n";
	exit(1);
    }

    int num_cores;
    int num_points;

    float x;
    float y;
    float s;

    cores >> num_cores;

    for (; num_cores > 0; num_cores--) {
	int which_core = core_list_p->append_core();
	cores >> num_points;
	
	for (; num_points > 0; num_points--) {
	    cores >> x >> y >> s;
	    core_list_p->append_point(which_core, x, y, s);
	}
    }
}


