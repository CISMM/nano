#include <vrpn_Types.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <UTree.h>
#include <URPolygon.h>
#include <URTexture.h>
#include <FileGenerator.h>
#include <MSIFileGenerator.h>
#include <string.h>

///import_object handlers
static  void handle_import_file_change (const char *, void *);
static	void handle_current_object_new(const char*, void*);
static	void handle_current_object(const char*, void*);
static  void handle_import_scale_change (vrpn_float64, void *);
static  void handle_import_tess_change (vrpn_int32, void *);
static  void handle_import_transx_change (vrpn_float64, void *);
static  void handle_import_transy_change (vrpn_float64, void *);
static  void handle_import_transz_change (vrpn_float64, void *);
static  void handle_import_rotx_change (vrpn_float64, void *);
static  void handle_import_roty_change (vrpn_float64, void *);
static  void handle_import_rotz_change (vrpn_float64, void *);
static  void handle_import_visibility (vrpn_int32, void *);
static  void handle_import_color_change (vrpn_int32, void *);
static  void handle_import_alpha (vrpn_float64, void *);
static  void handle_import_proj_text (vrpn_int32, void *);
static	void handle_import_CCW (vrpn_int32, void *);

///Import object handlers specifically for MSI files
static  void handle_msi_bond_mode (vrpn_int32, void *);
static  void handle_msi_atom_radius(vrpn_float64, void *);

//-----------------------------------------------------------------
///These variables are for controlling importing of objects
Tclvar_string	modelFile("modelFile", "", handle_import_file_change);
Tclvar_string	import_type("import_type", "");

Tclvar_list_of_strings imported_objects("imported_objects");
Tclvar_string current_object_new("current_object_new", "", handle_current_object_new);
Tclvar_string current_object("current_object", "", handle_current_object);

Tclvar_float	import_scale("import_scale",1, handle_import_scale_change);
Tclvar_float	import_transx("import_transx",0, handle_import_transx_change);
Tclvar_float	import_transy("import_transy",0, handle_import_transy_change);
Tclvar_float	import_transz("import_transz",0, handle_import_transz_change);
Tclvar_float	import_rotx("import_rotx",0, handle_import_rotx_change);
Tclvar_float	import_roty("import_roty",0, handle_import_roty_change);
Tclvar_float	import_rotz("import_rotz",0, handle_import_rotz_change);
Tclvar_int      import_visibility("import_visibility", 1, handle_import_visibility);
Tclvar_int      import_proj_text("import_proj_text", 1, handle_import_proj_text);
Tclvar_int		import_CCW("import_CCW", 1, handle_import_CCW);
Tclvar_int		import_tess("import_tess", 10, handle_import_tess_change);
Tclvar_int      import_color_changed("import_color_changed", 0, handle_import_color_change);
Tclvar_int      import_r("import_r", 192);
Tclvar_int      import_g("import_g", 192);
Tclvar_int      import_b("import_b", 192);
Tclvar_float    import_alpha("import_alpha", 1, handle_import_alpha);

//-----------------------------------------------------------------
///MSI File specific variables
Tclvar_int      msi_bond_mode("msi_bond_mode", 0, handle_msi_bond_mode);
Tclvar_float    msi_atom_radius("msi_atom_radius", 1, handle_msi_atom_radius);

//------------------------------------------------------------------------
//Functions necessary for all imported objects

static void handle_current_object_new(const char*, void*) {
	if (imported_objects.numEntries() == 0) {
		// add "all" entry
		imported_objects.addEntry("all");
	}

	imported_objects.addEntry(current_object_new.string());
}

static void handle_current_object(const char*, void*) {
	*(World.current_object) = current_object.string();

	if (strcmp(*(World.current_object), "all") != 0) {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			
			import_scale = obj.GetLocalXform().GetScale();

			const q_vec_type &v = obj.GetLocalXform().GetTrans();
			import_transx = v[0];
			import_transy = v[1];
			import_transz = v[2];

			const q_type &q = obj.GetLocalXform().GetRot();
			import_rotx = q[0];
			import_roty = q[1];
			import_rotz = q[2];

			import_visibility = obj.GetVisibility();

			import_proj_text = obj.ShowProjText();

			import_CCW = obj.GetCCW();
		}
	}
}


static void handle_import_file_change (const char *, void *) {
    static char * old_name = "";
    static const float convert = 1.0f/255;
	    
    if (modelFile.string()) {        
        if (strcmp(modelFile.string(),"") != 0) {          
            //Only try to create the object if there is a file specified.

            URPolygon *obj = new URPolygon;
			obj->SetCCW(import_CCW);
			obj->SetTess(import_tess);
            FileGenerator *gen = FileGenerator::CreateFileGenerator(modelFile.string());
            import_type = gen->GetExtension();
            obj->LoadGeometry(gen);
            obj->SetVisibility(import_visibility);
			obj->SetProjText(import_proj_text);
	        obj->SetColor3(convert * import_r, convert * import_g, convert * import_b);
            obj->SetAlpha(import_alpha);
            obj->GetLocalXform().SetScale(import_scale);
            obj->GetLocalXform().SetTranslate(import_transx, import_transy, import_transz);
            obj->GetLocalXform().SetRotate(import_rotx, import_roty, import_rotz, 1);
			// truncate name so it correlates to the option menu
			// i.e. C:/Data/cube.obj -> cube.obj
			char* name;
			name = new char[strlen(modelFile.string()) + 1];
			strcpy(name, modelFile.string());
			char* c = name + strlen(name);
			while (*(--c) != '/');
            World.TAddNode(obj, c + 1);
			delete name;
        }
// Took this out because we want more than one object loaded at a time...
// can't close files now...should change this...
/*        if (strcmp(old_name, "") != 0) {
            UTree *node = World.TGetNodeByName(old_name);

            node->TGetParent()->TRemoveTreeNode(node);
            
            delete node;
            delete [] old_name;
            old_name = NULL;
        }
*/

//        old_name = new char[strlen(modelFile.string())+1];
//        strcpy(old_name, modelFile.string());
    }
}

static  void handle_import_visibility (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int i = import_visibility;
		World.Do(&URender::SetVisibilityAll, &i);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetVisibility(import_visibility);
		}
    }
}

static  void handle_import_proj_text (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int i = import_proj_text;
		World.Do(&URender::SetProjTextAll, &i);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetProjText(import_proj_text);
		}
	}
}

static  void handle_import_CCW (vrpn_int32, void *)
{
    UTree *node = World.TGetNodeByName(*World.current_object);
    if (node != NULL) {
        URender &obj = node->TGetContents();
        obj.SetCCW(import_CCW);
    }
}

static  void handle_import_tess_change (vrpn_int32, void *)
{
printf("import_tess = %d\n", import_tess);
    UTree *node = World.TGetNodeByName(*World.current_object);
    if (node != NULL) {
        URender &obj = node->TGetContents();
printf("import_tess = %d\n", import_tess);
        obj.SetTess(import_tess);
    }
}

static  void handle_import_scale_change (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		double i = import_scale;
		World.Do(&URender::ScaleAll, &i);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.GetLocalXform().SetScale(import_scale);
		}
	}
}

static  void handle_import_transx_change (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		double i = import_transx;
		World.Do(&URender::SetTransxAll, &i);
	}
	else {
	    UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			const q_vec_type &trans = obj.GetLocalXform().GetTrans();
			obj.GetLocalXform().SetTranslate(import_transx, trans[1], trans[2]);
		}
    }
}

static  void handle_import_transy_change (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		double i = import_transy;
		World.Do(&URender::SetTransyAll, &i);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			const q_vec_type &trans = obj.GetLocalXform().GetTrans();
			obj.GetLocalXform().SetTranslate(trans[0], import_transy, trans[2]);
		}
    }
}

static  void handle_import_transz_change (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		double i = import_transz;
		World.Do(&URender::SetTranszAll, &i);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			const q_vec_type &trans = obj.GetLocalXform().GetTrans();
			obj.GetLocalXform().SetTranslate(trans[0], trans[1], import_transz);
		}
    }
}

static  void handle_import_rotx_change (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		double i = import_rotx;
		World.Do(&URender::SetRotxAll, &i);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
		    URender &obj = node->TGetContents();
		    const q_type &rot = obj.GetLocalXform().GetRot();
		    obj.GetLocalXform().SetRotate(import_rotx, rot[1], rot[2], rot[3]);
		}
    }
}

static  void handle_import_roty_change (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		double i = import_roty;
		World.Do(&URender::SetRotyAll, &i);
	}
	else {
	    UTree *node = World.TGetNodeByName(*World.current_object);
	    if (node != NULL) {
	        URender &obj = node->TGetContents();
	        const q_type &rot = obj.GetLocalXform().GetRot();
	        obj.GetLocalXform().SetRotate(rot[0], import_roty, rot[2], rot[3]);
	    }
	}
}

static  void handle_import_rotz_change (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		double i = import_rotz;
		World.Do(&URender::SetRotzAll, &i);
	}
	else {
	    UTree *node = World.TGetNodeByName(*World.current_object);
	    if (node != NULL) {
	        URender &obj = node->TGetContents();
	        const q_type &rot = obj.GetLocalXform().GetRot();
	        obj.GetLocalXform().SetRotate(rot[0], rot[1], import_rotz, rot[3]);
	   }
	}
}

static  void handle_import_color_change (vrpn_int32, void *)
{
	static const float convert = 1.0f/255;

	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		RGB i;
		i.red = convert * import_r;
		i.green = convert * import_g;
		i.blue = convert * import_b;
		World.Do(&URender::SetColorAll, &i);
	}
	else {
	   UTree *node = World.TGetNodeByName(*World.current_object);
	   if (node != NULL) {
	       URender &obj = node->TGetContents();
	       obj.SetColor3(convert * import_r, convert * import_g, convert * import_b);
		}
	}
}

static  void handle_import_alpha (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		float i = import_alpha;
		World.Do(&URender::SetAlphaAll, &i);
	}
	else {
	    UTree *node = World.TGetNodeByName(*World.current_object);
	    if (node != NULL) {
	        URender &obj = node->TGetContents();
	        obj.SetAlpha(import_alpha);
	    }
	}
}

//------------------------------------------------------------------------
//MSI File Specific functions
static  void handle_msi_bond_mode (vrpn_int32, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URPolygon &obj = (URPolygon&)node->TGetContents();
        ((MSIFileGenerator*)obj.GetGenerator())->SetImportMode(msi_bond_mode);
        obj.ReloadGeometry();
    }
}

static  void handle_msi_atom_radius(vrpn_float64, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URPolygon &obj = (URPolygon&)node->TGetContents();
        ((MSIFileGenerator*)obj.GetGenerator())->SetSphereRadius(msi_atom_radius);
        obj.ReloadGeometry();
    }
}
