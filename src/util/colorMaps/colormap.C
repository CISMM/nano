#include	<stdlib.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<math.h>
#include	"colormap.h"

ColorMap::ColorMap (const char * filename, const char * dir) {
	table = NULL;
	num_entries = 0;
	num_allocated = 0;
	interp = COLORMAP_LINEAR;

	// If the filename is NULL, we're done
	// Load the file into the table if not
	if (filename) {
	    if (load_from_file(filename, dir)) {
		fprintf(stderr,"ColorMap::ColorMap(): Can't read file\n");
		return;
	    }
	} else {
	    return;
	}
}

ColorMap::~ColorMap()
{
	if (table) {
		delete [] table;
	}
}

int	ColorMap::get_full_name (const char * filename, const char * dir,
	char *full_name, int maxlen)
{
	// If the directory is NULL, we use ".".  Otherwise, fill in
	if (dir) {
		strncpy(full_name, dir, maxlen);
	} else {
		strncpy(full_name, ".", maxlen);
	}

	// Form the complete file name
	if ( (strlen(full_name) + strlen(filename) + 2) > maxlen) {
		fprintf(stderr,"ColorMap::get_full_name(): Path too long\n");
		return -1;
	}
	strcat(full_name, "/");
	strcat(full_name, filename);

	return 0;
}

int	ColorMap::load_from_file (const char * filename, const char * dir)
{
	char	full_name[5000];
	FILE	*infile;
	char	line[200];

	// Get the full file name
	if (get_full_name(filename, dir, full_name, sizeof(full_name))) {
		fprintf(stderr,"ColorMap::load_from_file(): Can't make name\n");
		return -1;
	}

	// Open the file for reading
	if ( (infile = fopen(full_name, "r")) == NULL) {
		perror("ColorMap::load_from_file(): Can't open file");
		fprintf(stderr,"   (File %s)\n",full_name);
		return -1;
	}

	// Free up the old table entries, if any
	num_entries = 0;
	num_allocated = 0;
	if (table != NULL) {
		delete [] table;
		table = NULL;
	}

	// Allocate some space for a new table (may need more later)
	table = new Colormap_Table_Entry[255];
	if (table == NULL) {
		fprintf(stderr,"ColorMap::load_from_file(): new() failed!\n");
		return -1;
	} else {
		num_allocated = 255;
	}

	// Look for the word 'ColorMap' at the start of the file
	fgets(line, sizeof(line), infile);
	if (strncmp(line,"Colormap",strlen("Colormap"))) {
		fprintf(stderr,
		  "ColorMap::load_from_file(): Expected 'Colormap' in file\n");
		fprintf(stderr,
		  "   (got %s)\n",line);
		fprintf(stderr,"   (File %s)\n",full_name);
		return -1;
	}
	// Look for the line of -----------'s separating header from body
	while (strncmp(line,"--------",8)) {
	    if (fgets(line, sizeof(line),infile) == NULL) {
		fprintf(stderr,"ColorMap::load_from_file(): Didn't find ---------- separator line\n");
		fprintf(stderr,"   (File %s)\n",full_name);
	    }
	}

	// Read in the file body.  Each line is float value, int r,g,b,a
	while (fgets(line,sizeof(line),infile)) {
		int ret;
		float	value;
		int	r,g,b,a;

		// Read the line and parse it
		ret = sscanf(line,"%g%d%d%d%d", &value, &r, &g, &b, &a);
		if (ret != 5) continue;	// XXX Skip lines we don't grok

		// Make sure it has a larger value than the last one
		if ((num_entries > 0) && (value <= table[num_entries-1].value)){
			fprintf(stderr,"ColorMap::load_from_file(): Conscutive values must always increase\n");
			return -1;
		}

		// XXX Ensure we have enough room for the entry
		if (num_entries >= num_allocated) {
			fprintf(stderr,"ColorMap::load_from_file(): Too many entries (ignoring the rest)\n");
			continue;
		}

		// Fill in the new entry
		table[num_entries].value = value;
		table[num_entries].r = r/255.0;
		table[num_entries].g = g/255.0;
		table[num_entries].b = b/255.0;
		table[num_entries].a = a/255.0;
		num_entries++;
	}

	// Close the file
	if (fclose(infile)) {
		perror("ColorMap::load_from_file(): Error closing file");
		return -1;
	}
	return 0;
}


int	ColorMap::store_to_file (const char * filename, const char * dir)
{
	int	i;
	char	full_name[5000];
	FILE	*outfile;

	// Get the full file name
	if (get_full_name(filename, dir, full_name, sizeof(full_name))) {
		fprintf(stderr,"ColorMap::store_to_file(): Can't write file\n");
		return -1;
	}

	// Open the file for writing
	if ( (outfile = fopen(full_name, "w")) == NULL) {
		perror("ColorMap::store_to_file(): Can't write file");
		fprintf(stderr,"   (File %s)\n",full_name);
		return -1;
	}

	// Put the file header
	fprintf(outfile,"Colormap\n");
	fprintf(outfile,"Value\tRed\tGreen\tBlue\tAlpha\n");
	fprintf(outfile,"------------------------------------------------\n");

	// Write the file body.  Each line is float value, int r,g,b,a
	for (i = 0; i < num_entries; i++) {
		if (fprintf(outfile,"%g\t%d\t%d\t%d\t%d\n",
			table[i].value,
			(int)(table[i].r*255),
			(int)(table[i].g*255),
			(int)(table[i].b*255),
			(int)(table[i].a*255)) != 5) {
		    perror("ColorMap::store_to_file(): Can't write line");
		    return -1;
		}
	}

	// Close the file
	if (fclose(outfile)) {
		perror("ColorMap::store_to_file(): Error closing file");
		return -1;
	}
	return 0;
}

void	ColorMap::set_interpolation(Colormap_Interpolation interpolation)
{
	interp = interpolation;
}

void	ColorMap::lookup (float value,
                          float * r, float * g, float * b, float * a) const
{
	int	i;
	float	scale;

	// If no table, return 0
	if (num_entries == 0) {
		*r = *g = *b = *a = 0;
		return;

	// If only one, return it
	} else if (num_entries == 1) {
		*r = table[0].r;
		*g = table[0].g;
		*b = table[0].b;
		*a = table[0].a;
		return;
	}

	// Clamp the value to the min/max ones in the table
	if (value < table[0].value)
		value = table[0].value;
	if (value > table[num_entries - 1].value)
		value = table[num_entries - 1].value;

	// Find an entry in the table that bounds the value from above
	// and is suitable to allow interpolation by the value between it
	// and the next-lowest entry.  Basically, start with the second
	// one and go up until the value in the table is above.
	i = 1; while (value > table[i].value) i++;

	// Interpolate the parameters between this and next-lower entry
	scale = (value - table[i - 1].value) /
		(table[i].value - table[i - 1].value);
	*r = table[i-1].r * (1 - scale) + table[i].r * scale;
	*g = table[i-1].g * (1 - scale) + table[i].g * scale;
	*b = table[i-1].b * (1 - scale) + table[i].b * scale;
	*a = table[i-1].a * (1 - scale) + table[i].a * scale;
}

void	ColorMap::lookup (float value,
                          int * r, int * g, int * b, int * a) const
{
	float	fr, fg, fb, fa;

	// Find the float values
	lookup(value, &fr, &fg, &fb, &fa);

	// Scale to 0..255
	*r = (int) (fr * 255);
	*g = (int) (fg * 255);
	*b = (int) (fb * 255);
	*a = (int) (fa * 255);
}

