/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#if !defined (_WIN32) || defined (__CYGWIN__)
#include <dirent.h>
#else
// Special file in the nmBase directory because VC++ doesn't have it's own. 
#include <vc_dirent.h>
#endif

#include "nmui_ColorMap.h"
#include <tk.h>
#include <tcl_tk.h> // for get_the_interpreter(). 

#include "nmui_Component.h"

#include <error_display.h>

/** @file nmui_ColorMap.C
    Encapsulates the interface to a list of colormaps, and the
    C interface to the Tcl widgets which choose a colormap and
    its parameters. 
 */

Tclvar_list_of_strings* nmui_ColorMap::s_colorMapNames = NULL;

const int nmui_ColorMap::s_menu_cmap_width = 24, nmui_ColorMap::s_menu_cmap_height = 128;

/** Update our own state if the colormap name changes */
void nmui_ColorMap::handle_colormap_change (const char *, void * userdata) {
    nmui_ColorMap * me = (nmui_ColorMap *)userdata;
    me->currentColorMap();
    me->tcl_colormapRedraw();
}

//  void handle_color_dataset_change(const char *, void * userdata)
//  {
//  }

/** Update our own state if the color/data min/max changes */
void nmui_ColorMap::handle_color_minmax_change (vrpn_float64, void * userdata) {
    nmui_ColorMap * me = (nmui_ColorMap *)userdata;
    me->tcl_colormapRedraw();
}

void nmui_ColorMap::handle_surface_color_change (vrpn_int32, void * userdata) {
    char name[100];
    nmui_ColorMap * me = (nmui_ColorMap *)userdata;
    if ( *(me->surface_color_changed) == 1 ) {
        *(me->surface_color_changed) = 0;

        // Re-do the "none" colormap based on new surface color.
        me->d_defaultColorMap->setGradient(0,0,0,255,int(*me->surface_r), 
                              int(*me->surface_g), int(*me->surface_b), 255);
        sprintf(name, "%s(cm_image_none)", me->d_tcl_array_name);
        me->makeColorMapImage(me->d_defaultColorMap, name, 
                      s_menu_cmap_width, s_menu_cmap_height);
        *me->colormap_name = "none";
        // Triggered already by setting cmap name. ???
        //me->tcl_colormapRedraw();
    }
}

/** The tclnet params can be null, as long as swapTclStrings is called 
    later */
nmui_ColorMap::nmui_ColorMap (const char * tclname_prefix,
                              TclNet_string * cimage_name,
                              Tclvar_list_of_strings * cimage_name_list,
                              TclNet_string * cmap_name) :
    d_tcl_array_name(NULL),
    d_curColorMap(NULL),
    d_defaultColorMap(NULL),
    color_image_name(cimage_name),
    color_image_name_list(cimage_name_list),
    colormap_name(cmap_name)
{
    char name [200];
    d_tcl_array_name = new char[strlen(tclname_prefix) + 1];
    strcpy(d_tcl_array_name, tclname_prefix);
    sprintf(name, "%s(color_limit_min)", tclname_prefix);
    color_min_limit = new Tclvar_float(name,0);
    sprintf(name, "%s(color_limit_max)", tclname_prefix);
    color_max_limit = new Tclvar_float(name,1);
    sprintf(name, "%s(color_min)", tclname_prefix);
    color_min = new TclNet_float(name,0,handle_color_minmax_change,this);
    sprintf(name, "%s(color_max)", tclname_prefix);
    color_max = new TclNet_float(name,1,handle_color_minmax_change,this);
    sprintf(name, "%s(data_min)", tclname_prefix);
    data_min = new TclNet_float(name,0,handle_color_minmax_change,this);
    sprintf(name, "%s(data_max)", tclname_prefix);
    data_max = new TclNet_float(name,1,handle_color_minmax_change,this);
    sprintf(name, "%s(surface_r)", tclname_prefix);
    surface_r = new TclNet_int(name,192);
    sprintf(name, "%s(surface_g)", tclname_prefix);
    surface_g = new TclNet_int(name,192);
    sprintf(name, "%s(surface_b)", tclname_prefix);
    surface_b = new TclNet_int(name,192);
    sprintf(name, "%s(surface_color_changed)", tclname_prefix);
    surface_color_changed = new TclNet_int (name, 0, 
                                            handle_surface_color_change,this);
    
    d_defaultColorMap = new nmb_ColorMap(0,0,0,255,int(*surface_r), 
                                     int(*surface_g), int(*surface_b),255);

    sprintf(name, "%s(cm_image_none)", tclname_prefix);
    makeColorMapImage(d_defaultColorMap, name, 
                      s_menu_cmap_width, s_menu_cmap_height);
    tcl_colormapRedraw();
    if (colormap_name) {
        colormap_name->addCallback(handle_colormap_change, this);
    }
}

/** Allows the external tclvars to be changed, like when nmb_Dataset is
    re-created. */
void nmui_ColorMap::swapTclStrings(TclNet_string * cimage_name,
                                   Tclvar_list_of_strings * cimage_name_list,
                                   TclNet_string * cmap_name) {
    color_image_name = cimage_name;
    color_image_name_list = cimage_name_list;
    colormap_name = cmap_name;
    if (colormap_name) {
        colormap_name->addCallback(handle_colormap_change, this);
    }
}


void nmui_ColorMap::setColorMinMaxLimit(double min, double max) {
    *color_min_limit = min;
    *color_max_limit = max;
}

nmb_ColorMap * nmui_ColorMap::currentColorMap() { 
    if (strcmp(getColorMapName(), "none") == 0) {
        d_curColorMap = d_defaultColorMap;
    } else {
        d_curColorMap = colorMaps[s_colorMapNames->
                                 getIndex(getColorMapName())];
    }
    return d_curColorMap; 
}

const char * nmui_ColorMap::getColorImageName() { 
    return color_image_name->string(); 
}

const char * nmui_ColorMap::getColorMapName() {
    return colormap_name->string(); 
}

void nmui_ColorMap::getDataColorMinMax(double * dmin, double * dmax, 
                                       double * cmin, double * cmax) {
    *dmin = *data_min;
    *dmax = *data_max;
    *cmin = *color_min;
    *cmax = *color_max;
}

void nmui_ColorMap::getSurfaceColor(int * red, int * green, int * blue)
{
    *red = *surface_r;
    *green = *surface_g;
    *blue = *surface_b;
}

void nmui_ColorMap::setSurfaceColor(int red, int green, int blue)
{
    *surface_r = red;
    *surface_g = green;
    *surface_b = blue;
    // Triggers callbacks, including our own to redraw colormap.
    *surface_color_changed = 1;
}

/**
loadColorMaps
	This routine looks at the color maps in the specified directory, 
        loads them, and creates images in Tcl so they can be displayed. 
        The tcl images have the same name as the colormap with "cm_image_" 
        on the front. 
*/
int nmui_ColorMap::loadColorMapNames(char * colorMapDir)
{
    char cm_name[100];

    DIR	*directory;
    struct	dirent *entry;
    nmb_ListOfStrings temp_dir_list;
    
    //colorMaps[0] = new ColorMap(0,0,0,255,int(surface_r), int(surface_g), int(surface_b),255);
    // This is probably unnecessary, since each ColorMap object will
    // use it's own "none" colormap.
    colorMaps[0] = new nmb_ColorMap(0,0,0,255,255,255,255,255);
    makeColorMapImage(colorMaps[0], "cm_image_none", 
                      s_menu_cmap_width, s_menu_cmap_height);

    // Set the default entry to use the custom color controls
    temp_dir_list.addEntry("none");

    // Get the list of files that are in that directory
    // Put the name of each file in that directory into the list
    if ( (directory = opendir(colorMapDir)) == NULL) {
        display_error_dialog("Couldn't load colormaps from\n"
                             "directory named: %s\nDirectory not available.",colorMapDir);
        return -1;
    }
    int k;
    while ( (entry = readdir(directory)) != NULL) {
        if (entry->d_name[0] != '.') {
            k = temp_dir_list.numEntries();
            if (k==(nmb_ListOfStrings::NUM_ENTRIES)) {
                display_error_dialog("Couldn't finish loading colormaps from\n"
                                     "directory named: %s\n"
                                     "More than %d colormaps.",
                                     colorMapDir,
                                     nmb_ListOfStrings::NUM_ENTRIES-1);
                break;
            }
            // Load the colormap
            colorMaps[k] = new nmb_ColorMap(entry->d_name, colorMapDir);

            sprintf (cm_name, "cm_image_%s", entry->d_name); 
            makeColorMapImage(colorMaps[k], cm_name, 
                              s_menu_cmap_width, s_menu_cmap_height);

            // Remember the name
            temp_dir_list.addEntry(entry->d_name);
        }
    }
    // Don't set the real list of names until after the images have
    // been created. 
    baseColorMapNames = s_colorMapNames = new Tclvar_list_of_strings("colorMapNames");
    s_colorMapNames->copyList(&temp_dir_list);
    closedir(directory);
    
    return 0;
}

void nmui_ColorMap::addMinMaxCallback (Linkvar_Floatcall callback, void * userdata) {
    color_min->addCallback(callback, userdata);
    color_max->addCallback(callback, userdata);
    data_min->addCallback(callback, userdata);
    data_max->addCallback(callback, userdata);
}
void nmui_ColorMap::removeMinMaxCallback (Linkvar_Floatcall callback, void * userdata) {
    color_min->removeCallback(callback, userdata);
    color_max->removeCallback(callback, userdata);
    data_min->removeCallback(callback, userdata);
    data_max->removeCallback(callback, userdata);

}
void nmui_ColorMap::addColorImageNameCallback (Linkvar_Stringcall callback, void * userdata) {
    color_image_name->addCallback(callback, userdata);
}
void nmui_ColorMap::removeColorImageNameCallback (Linkvar_Stringcall callback, void * userdata) {
    color_image_name->removeCallback(callback, userdata);
}
void nmui_ColorMap::addColorMapNameCallback (Linkvar_Stringcall callback, void * userdata){
    colormap_name->addCallback(callback, userdata);
}
void nmui_ColorMap::removeColorMapNameCallback (Linkvar_Stringcall callback, void * userdata){
    colormap_name->removeCallback(callback, userdata);
}

void nmui_ColorMap::addSurfaceColorCallback (Linkvar_Intcall callback, void * userdata){
    surface_color_changed->addCallback(callback, userdata);
}
void nmui_ColorMap::removeSurfaceColorCallback (Linkvar_Intcall callback, void * userdata){
    surface_color_changed->removeCallback(callback, userdata);
}

/** Collaboration helper. */
void nmui_ColorMap::setupSynchronization(nmui_Component * c) {
  c->add(color_min);
  c->add(color_max);
  c->add(data_min);
  c->add(data_max);
}

/** Collaboration helper. */
void nmui_ColorMap::shutdownConnections() {
  color_min->bindConnection(NULL);
  color_max->bindConnection(NULL);
  data_min->bindConnection(NULL);
  data_max->bindConnection(NULL);
}

/** Take a colormap and makes an color-bar image in Tcl with specified name,
    width, height.  c_min and c_max are values between 0 and 1 which can
    squash the colormap range, making the whole colormap appear in a small
    window in the larger image. Defaults perform no squashing. 
 */
int nmui_ColorMap::makeColorMapImage(nmb_ColorMap * cmap, char * name, int width, int height, 
                      float c_min , float c_max ) 
{
    Tk_PhotoImageBlock colormap;
    unsigned char *colormap_pixels = new unsigned char[ height * width * 3];
    if (!colormap_pixels) return -1;
    Tk_PhotoHandle image;

    char command[200];
    // This code sets up the colormap bars displayed in the colormap
    // choice menu in tcl.
    colormap.pixelPtr = colormap_pixels;
    colormap.width = width;
    colormap.height = height;
    colormap.pixelSize = 3;
    colormap.pitch = width * 3;
    colormap.offset[0] = 0; colormap.offset[1] = 1; colormap.offset[2] = 2;

    // Make an image in Tcl based on the colormap.
    float ci;
    int r, g, b, a;
    for ( int i= 0; i < height; i++) {
        for ( int j = 0; j < width; j++ ) {
            ci = 1.0f - float(i)/height;
            if ( ci <  c_min ) ci = 0;
            else if ( ci > c_max ) ci = 1.0;
            else ci = (ci - c_min)/(c_max - c_min);
            
            cmap->lookup( ci, &r, &g, &b, &a);
            
            colormap_pixels[ i*width*3 + j*3 + 0] = (unsigned char)( r );
            colormap_pixels[ i*width*3 + j*3 + 1] = (unsigned char)( g );
            colormap_pixels[ i*width*3 + j*3 + 2] = (unsigned char)( b );
        }
    }
    // "image create" will replace any existing instance of the image. 
    sprintf (command, "image create photo %s", name);
    TCLEVALCHECK( get_the_interpreter(), command);
    image = Tk_FindPhoto( get_the_interpreter(), name );
    Tk_PhotoPutBlock( image, &colormap, 0, 0, width, height );

    return 0;
}

void nmui_ColorMap::tcl_colormapRedraw() {
    // Must match the colormap widget:
    const int colormap_width = 32, colormap_height = 192;

    char name [200];
    sprintf(name, "%s(colormap_image)", d_tcl_array_name);
    // Draw a colormap bar if either colorPlaneName or colorMapName 
    // aren't "none"
    if (( strcmp( getColorImageName(), "none") != 0 ) ||
        ( strcmp( getColorMapName(), "none") != 0) )  {
        // color_max and color_min "squash" the color map image based
        // on tcl controls. 
        makeColorMapImage(currentColorMap(), name, 
                          colormap_width, colormap_height, 
                          (float)(*color_min), (float)(*color_max));
    }
    else {
        //printf("nmui_ColorMap::tcl_colormapRedraw() else FIX ME\n");
          nmb_ColorMap constmap;
          constmap.setConst(*surface_r , *surface_g, *surface_b, 255);
          makeColorMapImage(&constmap, name, 
                            colormap_width, colormap_height);
    }  
}
