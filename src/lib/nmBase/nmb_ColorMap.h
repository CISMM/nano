/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef	COLORMAP_H
#define	COLORMAP_H

#include <stdio.h> // for FILE

class nmb_ListOfStrings;

typedef enum {
		COLORMAP_LINEAR
	} Colormap_Interpolation;

typedef	struct {
		float	value;
		float	r,g,b,a;
	} Colormap_Table_Entry;

/** @class nmb_ColorMap
	This file defines the interface for the nmb_ColorMap object.  This
 object knows how to read itself from a named file, write itself to
 a named file, and provide color values in the range 0..1.
	It is also possible to specify the type of interpolation to be
 performed between the colors.  The default is linear, the only choice so far.
*/
class nmb_ColorMap {

    public:
        /// Two color map, color 1 at zero, color 2 at 1.0
        nmb_ColorMap (int r1, int g1, int b1, int a1, int r2, int g2, int b2, int a2);
        /// Color map obtained from a file. 
	nmb_ColorMap (const char * filename = NULL, const char * dir = ".");
	nmb_ColorMap(const nmb_ColorMap &map);
	~nmb_ColorMap();

        /// Change to Two color map, color 1 at zero, color 2 at 1.0
        int setGradient(int r1, int g1, int b1, int a1,
                        int r2, int g2, int b2, int a2);

        /// Change to const "color map" - always the same color.
        int setConst(int r1, int g1, int b1, int a1);

	/// Load colormap from a file
	int	load_from_file (const char * filename, const char * dir = ".");
        /// Store to a file
	int	store_to_file (const char * filename, const char * dir = ".");

	/// Set the style of interpolation between points,  Default is linear.
	void	set_interpolation (Colormap_Interpolation);

        /// Lookup the color at a value,  The float version returns values
        /// from 0-1,
        /// The 'value' param is clamped to the range of values in the table.
	void	lookup (float value,
                        float * r, float * g, float * b, float * a) const;
        /// The integer version returns values from 0-255.
        void	lookup (float value,
                        int * r, int * g, int * b, int * a) const;
    /// Calculate a value based on colormap widget values.
        void	lookup (float value,
                        float min_data_value, float max_data_value,
                        float data_min, float data_max,
                        float color_min, float color_max,
                        int * r, int * g, int * b, int * a) const;
    /// Calculate a value based on colormap widget values.
        void	lookup (float value,
                        float min_data_value, float max_data_value,
                        float data_min, float data_max,
                        float color_min, float color_max,
                        float * r, float * g, float * b, float * a) const;

    protected:
	Colormap_Table_Entry	*table;	///< nmb_ColorMap table
	int	num_entries;		///< # of entries in the color table
	int	num_allocated;		///< # of allocated slots in the table

	Colormap_Interpolation	interp;	///< Type of interpolation to be done

	int	get_full_name (const char *, const char *, char *, int);
    /// Read a ThermoMicroscopes format color map, ".pal" file
        int read_PAL_from_file(char * line, int line_len, FILE * infile );
};

extern nmb_ColorMap * colorMaps[];		
  ///< Color maps currently loaded

/// This is the list of color map files from which mappings can be made;
/// it is connected with a Tclvar_list_of_strings in microscape.c 
extern nmb_ListOfStrings* baseColorMapNames;

#endif
