#ifndef IMPORT_H
#define IMPORT_H

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

///import_object handlers
static  void handle_import_file_change (const char *, void *);
static  void handle_import_scale_change (vrpn_float64, void *);
static  void handle_import_transx_change (vrpn_float64, void *);
static  void handle_import_transy_change (vrpn_float64, void *);
static  void handle_import_transz_change (vrpn_float64, void *);
static  void handle_import_rotx_change (vrpn_float64, void *);
static  void handle_import_roty_change (vrpn_float64, void *);
static  void handle_import_rotz_change (vrpn_float64, void *);
static  void handle_import_visibility (vrpn_int32, void *);
static  void handle_import_color_change (vrpn_int32, void *);
static  void handle_import_alpha (vrpn_float64, void *);

///Import object handlers specifically for MSI files
static  void handle_msi_bond_mode (vrpn_int32, void *);
static  void handle_msi_atom_radius(vrpn_float64, void *);

//-----------------------------------------------------------------
///These variables are for controlling importing of objects
Tclvar_string	modelFile("modelFile", "", handle_import_file_change);
Tclvar_string	import_type("import_type", "");
Tclvar_float	import_scale("import_scale",1, handle_import_scale_change);
Tclvar_float	import_transx("import_transx",0, handle_import_transx_change);
Tclvar_float	import_transy("import_transy",0, handle_import_transy_change);
Tclvar_float	import_transz("import_transz",0, handle_import_transz_change);
Tclvar_float	import_rotx("import_rotx",0, handle_import_rotx_change);
Tclvar_float	import_roty("import_roty",0, handle_import_roty_change);
Tclvar_float	import_rotz("import_rotz",0, handle_import_rotz_change);
Tclvar_int      import_visibility("import_visibility", 1, handle_import_visibility);
Tclvar_int      import_color_changed("import_color_changed", 0, handle_import_color_change);
Tclvar_int      import_r("import_r", 192);
Tclvar_int      import_g("import_g", 192);
Tclvar_int      import_b("import_b", 192);
Tclvar_float    import_alpha("import_alpha", 1, handle_import_alpha);

//-----------------------------------------------------------------
///MSI File specific variables
Tclvar_int      msi_bond_mode("msi_bond_mode", 0, handle_msi_bond_mode);
Tclvar_float    msi_atom_radius("msi_atom_radius", 1, handle_msi_atom_radius);
#endif