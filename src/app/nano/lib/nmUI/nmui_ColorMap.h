#ifndef NMUI_COLORMAP_H
#define NMUI_COLORMAP_H
/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include <nmb_ColorMap.h>

class nmui_Component;

class nmui_ColorMap {
public:
    nmui_ColorMap(const char * tclname_prefix,
                  TclNet_string * cimage_name,
                  TclNet_string * cmap_name);
    ~nmui_ColorMap() { };
    void swapTclStrings(TclNet_string * cimage_name,
                  TclNet_string * cmap_name);

    /// Set limits on color slider. 
    void setColorMinMaxLimit(double min, double max);
    nmb_ColorMap * currentColorMap();
    const char * getColorImageName();
    const char * getColorMapName();

    /// Gets current values from tcl UI.
    void getDataColorMinMax(double * dmin, double * dmax, 
                            double * cmin, double * cmax);

    /// Gets current values from tcl UI.
    void getSurfaceColor(int * red, int * green, int * blue);
    /// Sets current values, export to tcl UI.
    void setSurfaceColor(int red, int green, int blue);

    /// Get the color maps from a directory. 
    static int loadColorMapNames(char * colorMapDir);

    /// Make a colormap image for tcl UI. 
    static int makeColorMapImage(nmb_ColorMap * cmap, char * name, 
                                 int width, int height, 
                                 float c_min = 0, float c_max = 1); 
    /// Redraw the widget color map bar for tcl UI. 
    void tcl_colormapRedraw();

    void addMinMaxCallback (Linkvar_Floatcall callback, void * userdata);
    void removeMinMaxCallback (Linkvar_Floatcall callback, void * userdata);
    void addColorImageNameCallback (Linkvar_Stringcall callback, void * userdata);
    void removeColorImageNameCallback (Linkvar_Stringcall callback, void * userdata);
    void addColorMapNameCallback (Linkvar_Stringcall callback, void * userdata);
    void removeColorMapNameCallback (Linkvar_Stringcall callback, void * userdata);
    void addSurfaceColorCallback (Linkvar_Intcall callback, void * userdata);
    void removeSurfaceColorCallback (Linkvar_Intcall callback, void * userdata);

    /// Collaboration. 
    void setupSynchronization(nmui_Component * c);
    /// Collaboration. 
    void shutdownConnections();

private:
    static void handle_colormap_change (const char *, void * userdata);
    static void handle_color_minmax_change (vrpn_float64, void * userdata);
    static void handle_surface_color_change (vrpn_int32, void * userdata);

    /// width and height of colormap image used in the colormap choice popup menu
    static const int s_menu_cmap_width, s_menu_cmap_height;
    char * d_tcl_array_name;
    nmb_ColorMap * d_curColorMap;
    nmb_ColorMap * d_defaultColorMap;
    /** This is the list of color map files from which mappings can be made.
        It should be loaded with the files in the colormap directory, and
        "none". It is connected with an nmb_ListOfStrings baseColorMapNames in
        colormap.h */
    static Tclvar_list_of_strings* s_colorMapNames;

    TclNet_string * color_image_name;
    TclNet_string * colormap_name;

    /// The limits on the Tk slider where min and max value are selected
    Tclvar_float *color_min_limit;
    Tclvar_float *color_max_limit;
// NANOX
    /// The positions of the min and max values within the Tk slider
    TclNet_float *color_min;
    TclNet_float *color_max;
    TclNet_float *data_min;
    TclNet_float *data_max;
    // Surface color is displayed when the colormap is not applied. 
    TclNet_int *surface_r;
    TclNet_int *surface_g;
    TclNet_int *surface_b;
    TclNet_int *surface_color_changed;
};




#endif
