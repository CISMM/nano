//imported_obj.h

//don't forget to remove the next two includes--for debugging only!
#include <stdio.h>
#include <fstream.h>

#include <vrpn_Types.h>

#include "Tcl_Linkvar.h"
#include "Tcl_Netvar.h"

#include "UTree.h"
#include "URPolygon.h"

const int ugraphics_scale = 20;

class imported_obj_list; //forward declaration

class imported_obj {
  //private:
    URPolygon *URPoly;  //A pointer to the objects ubergraphics object --
                        //which actually determines how/when the object
                        //is drawn, etc.
    int object_number;

    imported_obj* next;
    
    //store values for rotate methods so that we combine rotations
    float x_last_rotated;
    float y_last_rotated;
    float z_last_rotated;
    int import_mode_value; //determines whether or not to draw the bonds of 
                           //nanotubes (as lines) or the atoms of nanotubes 
                           //(as spheres).  It is 0 when we are in bond mode, 
                           //and 1 when we are in sphere mode
    int visibility_mode_value; //determines whether or not to draw the object.                                //It is 0 when in hide mode (don't draw the 
                               //object), and 1 when in show mode (draw the 
                               //object)

    //variables to store slider variables of view window, so that when we
    //delete the sliders, we can store their values and use them when the 
    //sliders are recreated
    float bond_width_value;
    float sphere_radius_value;
    int sphere_tesselation_value;
    float bond_colorR_value, bond_colorG_value, bond_colorB_value;
    float sphere_colorR_value, sphere_colorG_value, sphere_colorB_value;

    //TCL widgets for sliders, buttons, entries, etc. in import_objects window
    Tclvar_int_with_scale *translate_x;
    Tclvar_int_with_scale *translate_y;
    Tclvar_int_with_scale *translate_z;
    Tclvar_int_with_scale *rotate_x;
    Tclvar_int_with_scale *rotate_y;
    Tclvar_int_with_scale *rotate_z;
    Tclvar_float_with_scale *scale;

    Tclvar_int *import_mode; //0 is bond mode, 1 is sphere mode
    Tclvar_int *visibility_mode; //0 is hide mode, 1 is show mode
    Tclvar_int *delete_object;

    //TCL widgets for sliders, buttons, etc. in View window
    Tclvar_float_with_scale *bond_width;
    Tclvar_float_with_scale *sphere_radius;
    Tclvar_int_with_scale *sphere_tesselation;
    Tclvar_float_with_scale *bond_colorR;
    Tclvar_float_with_scale *bond_colorG;
    Tclvar_float_with_scale *bond_colorB;
    Tclvar_float_with_scale *sphere_colorR;
    Tclvar_float_with_scale *sphere_colorG;
    Tclvar_float_with_scale *sphere_colorB;

    static int imported_obj_count; //number of objects created so far

  public:
    imported_obj(); //default constructor
    void set_tcl_change_callback(); //set up the TCL callbacks for initially
                                    //visible widgets
    URPolygon *GetURPoly();

    //handlers for TCL widgets in import_objects window
    static void handle_translate_x_change(vrpn_int32,void*);
    static void handle_translate_y_change(vrpn_int32,void*);
    static void handle_translate_z_change(vrpn_int32,void*);
    static void handle_rotate_x_change(vrpn_int32,void*);
    static void handle_rotate_y_change(vrpn_int32,void*);
    static void handle_rotate_z_change(vrpn_int32,void*);
    static void handle_scale_change(vrpn_float64,void*);

    static void handle_import_mode_change(vrpn_int32,void*);
    static void handle_visibility_mode_change(vrpn_int32,void*);
    //static void handle_delete_object_change(vrpn_int32,void*);

    //handlers for TCL widgets in View window
    static void handle_bond_width_change(vrpn_float64,void*);
    static void handle_sphere_radius_change(vrpn_float64,void*);
    static void handle_sphere_tesselation_change(vrpn_int32,void*);
    static void handle_bond_colorR_change(vrpn_float64,void*);
    static void handle_bond_colorG_change(vrpn_float64,void*);
    static void handle_bond_colorB_change(vrpn_float64,void*);
    static void handle_sphere_colorR_change(vrpn_float64,void*);
    static void handle_sphere_colorG_change(vrpn_float64,void*);
    static void handle_sphere_colorB_change(vrpn_float64,void*);
    friend imported_obj_list; //allow imported_obj_list access to private variables
};

//imported_obj_list is a linked list of imported_obj objects
class imported_obj_list { 
  //private:
    imported_obj* imported_obj_list_head;
    imported_obj* imported_obj_list_tail;
  public:
    imported_obj_list(); //create a NULL list
    void import_new_obj(char*,UTree*x); //create and add an imported_obj to the list
};
