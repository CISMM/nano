/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
/** @class ColorMap
	This file defines the interface for the ColorMap object.  This
 object knows how to read itself from a named file, write itself to
 a named file, and provide color values in the range 0..1.
	It is also possible to specify the type of interpolation to be
 performed between the colors.  The default is linear.
*/

#ifndef	COLORMAP_H
#define	COLORMAP_H

typedef enum {
		COLORMAP_LINEAR
	} Colormap_Interpolation;

typedef	struct {
		float	value;
		float	r,g,b,a;
	} Colormap_Table_Entry;

class ColorMap {

    public:
        /// Two color map, color 1 at zero, color 2 at 1.0
        ColorMap (int r1, int g1, int b1, int a1, int r2, int g2, int b2, int a2);
        /// Color map obtained from a file. 
	ColorMap (const char * filename = NULL, const char * dir = ".");
	~ColorMap();

        /// Change to Two color map, color 1 at zero, color 2 at 1.0
        int setGradient(int r1, int g1, int b1, int a1,
                        int r2, int g2, int b2, int a2);

        /// Change to const "color map" - always the same color.
        int setConst(int r1, int g1, int b1, int a1);

	/// Load colormap from a file
	int	load_from_file (const char * filename, const char * dir = ".");
        /// Store to a file
	int	store_to_file (const char * filename, const char * dir = ".");

	/// Set the style of interpolation between points.  Default is linear.
	void	set_interpolation (Colormap_Interpolation);

        /// Lookup the color at a value.  The float version returns values
        /// from 0-1.  The integer version returns values from 0-255.
        /// The 'value' param is clamped to the range of values in the table.
	void	lookup (float value,
                        float * r, float * g, float * b, float * a) const;
	void	lookup (float value,
                        int * r, int * g, int * b, int * a) const;

    protected:
	Colormap_Table_Entry	*table;	///< ColorMap table
	int	num_entries;		///< # of entries in the color table
	int	num_allocated;		///< # of allocated slots in the table

	Colormap_Interpolation	interp;	///< Type of interpolation to be done

	int	get_full_name (const char *, const char *, char *, int);
};

#endif
