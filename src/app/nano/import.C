#include <vrpn_Types.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <UTree.h>
#include <URPolygon.h>
#include <URTexture.h>
#include <FileGenerator.h>
#include <MSIFileGenerator.h>

#include <string.h>

#include <nmm_SimulatedMicroscope_Remote.h>	// so we know if there is an open connection for
											// sending tubes when we open a tube file 

///import_object handlers
static  void handle_import_file_change (const char *, void *);
static	void handle_current_object_new(const char*, void*);
static	void handle_current_object(const char*, void*);
static  void handle_import_scale_change (vrpn_float64, void *);
static  void handle_import_tess_change (vrpn_int32, void *);
static  void handle_import_axis_step_change (vrpn_int32, void *);
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
static	void handle_import_clamp (vrpn_int32, void *);
static	void handle_import_CCW (vrpn_int32, void *);
static	void handle_import_update_AFM (vrpn_int32, void *);
static	void handle_import_grab_object (vrpn_int32, void *);

static	void handle_spider_length_change(vrpn_float64, void*);
static	void handle_spider_width_change(vrpn_float64, void*);
static	void handle_spider_thick_change(vrpn_float64, void*);
static	void handle_spider_tess_change(vrpn_int32, void*);
static	void handle_spider_curve_change(vrpn_float64, void*);

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
Tclvar_int		import_clamp("import_clamp", 0, handle_import_clamp);
Tclvar_int		import_CCW("import_CCW", 1, handle_import_CCW);
Tclvar_int		import_update_AFM("import_update_AFM", 0, handle_import_update_AFM);
Tclvar_int		import_grab_object("import_grab_object", 0, handle_import_grab_object);
Tclvar_int		import_tess("import_tess", 10, handle_import_tess_change);
Tclvar_int		import_axis_step("import_axis_step", 10, handle_import_axis_step_change);
Tclvar_int      import_color_changed("import_color_changed", 0, handle_import_color_change);
Tclvar_int      import_r("import_r", 192);
Tclvar_int      import_g("import_g", 192);
Tclvar_int      import_b("import_b", 192);
Tclvar_float    import_alpha("import_alpha", 1, handle_import_alpha);
Tclvar_float    spider_length("spider_length", 5, handle_spider_length_change);
Tclvar_float    spider_width("spider_width", 2, handle_spider_width_change);
Tclvar_float    spider_thick("spider_thick", 0.1, handle_spider_thick_change);
Tclvar_int		spider_tess("spider_tess", 10, handle_spider_tess_change);
Tclvar_float    spider_curve("spider_curve", 0, handle_spider_curve_change);

//-----------------------------------------------------------------
///SimulatedMicroscope object
extern nmm_SimulatedMicroscope_Remote* SimulatedMicroscope;

//-----------------------------------------------------------------
///MSI File specific variables
Tclvar_int      msi_bond_mode("msi_bond_mode", 0, handle_msi_bond_mode);
Tclvar_float    msi_atom_radius("msi_atom_radius", 1, handle_msi_atom_radius);

//------------------------------------------------------------------------
//Functions necessary for all imported objects

static void handle_current_object_new(const char*, void*) {
	nmb_ListOfStrings temp;
	int i;

	// return NONE if user cancels open
	if (strcmp(current_object_new.string(), "NONE") != 0) {
		temp.addEntry(current_object_new.string());

		for (i = 0; i < imported_objects.numEntries(); i++) {
			temp.addEntry(imported_objects.entry(i));
		}

		imported_objects.clearList();

		for (i = 0; i < temp.numEntries(); i++) {
			imported_objects.addEntry(temp.entry(i));
		}


		if (imported_objects.getIndex("all") == -1) {
			// add "all" entry
			imported_objects.addEntry("all");
		}

		// remove none
		imported_objects.deleteEntry("none");
	}
}

static void handle_current_object(const char*, void*) {
	*World.current_object = current_object.string();

	if (strcmp(*World.current_object, "all") != 0 &&
		strcmp(*World.current_object, "none") != 0) {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			
			import_scale = obj.GetLocalXform().GetScale();

			const q_vec_type &v = obj.GetLocalXform().GetTrans();
			import_transx = v[0];
			import_transy = v[1];
			import_transz = v[2];

			q_vec_type euler;
			q_to_euler(euler, obj.GetLocalXform().GetRot());
			import_rotx = Q_RAD_TO_DEG(euler[2]);
			import_roty = Q_RAD_TO_DEG(euler[1]);
			import_rotz = Q_RAD_TO_DEG(euler[0]);

			import_visibility = obj.GetVisibility();

			import_proj_text = obj.ShowProjText();
			import_clamp = obj.GetClamp();

			import_CCW = obj.GetCCW();

			import_update_AFM = obj.GetUpdateAFM();

			import_grab_object = obj.GetGrabObject();

			spider_length = obj.GetSpiderLength();
			spider_width = obj.GetSpiderWidth();
			spider_thick = obj.GetSpiderThick();
			spider_tess = obj.GetSpiderTess();
			spider_curve = Q_RAD_TO_DEG(obj.GetSpiderCurve());
		}
	}
	else {
		import_update_AFM = 0;
	}
}


static void handle_import_file_change (const char *, void *) {
    static const float convert = 1.0f/255;
	char* name;
	char* c;

    if (modelFile.string()) {  
        if (strcmp(modelFile.string(),"") != 0) {	// open the file
            //Only try to create the object if there is a file specified.
            URPolygon *obj = new URPolygon;
			obj->SetCCW(import_CCW);
			obj->SetTess(import_tess);
			obj->SetAxisStep(import_axis_step);
			obj->SetSpiderLength(spider_length);
			obj->SetSpiderWidth(spider_width);
			obj->SetSpiderThick(spider_thick);
			obj->SetSpiderTess(spider_tess);
			obj->SetSpiderCurve(Q_DEG_TO_RAD(spider_curve));
            FileGenerator *gen = FileGenerator::CreateFileGenerator(modelFile.string());
            import_type = gen->GetExtension();
            obj->LoadGeometry(gen);
            obj->SetVisibility(import_visibility);
			obj->SetProjText(import_proj_text);
			obj->SetClamp(import_clamp);
			// only allow update_AFM for .txt files
			if (strstr(modelFile.string(), ".txt") != 0) {
				obj->SetUpdateAFM(1);
			}
			else {
				obj->SetUpdateAFM(0);
			}
			obj->SetGrabObject(import_grab_object);
	        obj->SetColor3(convert * import_r, convert * import_g, convert * import_b);
            obj->SetAlpha(import_alpha);
/*
Took this out because we now set the translation automatically to the heightplane's origin
when loading
            obj->GetLocalXform().SetScale(import_scale);
            obj->GetLocalXform().SetTranslate(import_transx, import_transy, import_transz);
            obj->GetLocalXform().SetRotate(import_rotx, import_roty, import_rotz, 1);
*/
			// truncate name so it correlates to the option menu
			// i.e. C:/Data/cube.obj -> cube.obj

			name = new char[strlen(modelFile.string()) + 1];
			strcpy(name, modelFile.string());
			c = name + strlen(name);
			while (*(--c) != '/');
			World.TAddNode(obj, c + 1);

			// if a tube file, send to simulator
			if ((strstr(name, ".txt") != 0) && (SimulatedMicroscope != NULL)) {
				SimulatedMicroscope->sendCylinders(obj);
			}
			
			*World.current_object = name;

			// had to put this here so that the correct current_object is used
			import_update_AFM = obj->GetUpdateAFM();

			delete name;
        }
		else {	// close the current object
			if (strcmp(*World.current_object, "") != 0) {
				if (strcmp(*World.current_object, "all") == 0 ||
					strcmp(*World.current_object, "none") == 0) {
					return;
				}
				UTree *node = World.TGetNodeByName(*World.current_object);

			    node->TGetParent()->TRemoveTreeNode(node);
	

				imported_objects.deleteEntry(*World.current_object);
				// put "all" back at the end
				imported_objects.deleteEntry("all");
				imported_objects.addEntry("all");


				if (imported_objects.numEntries() == 1) {
					imported_objects.deleteEntry("all");
					imported_objects.addEntry("none");
				}
            
				delete node;
			}
        }
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

static  void handle_import_clamp (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int i = import_clamp;
		World.Do(&URender::SetClampAll, &i);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetClamp(import_clamp);
		}
	}
}

static  void handle_import_update_AFM (vrpn_int32, void *)
{
	// only do on a per object basis...only allow for .txt files

	// if all selected, do nothing
	if (strcmp(*World.current_object, "all") != 0 &&
		strstr(*World.current_object, ".txt") != 0) {

		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetUpdateAFM(import_update_AFM);

			// do update
			if (SimulatedMicroscope != NULL &&
				obj.GetUpdateAFM()) {
				SimulatedMicroscope->encode_and_sendScale(obj.GetLocalXform().GetScale());
				SimulatedMicroscope->encode_and_sendTrans(obj.GetLocalXform().GetTrans()[0],
															obj.GetLocalXform().GetTrans()[1],
															obj.GetLocalXform().GetTrans()[2]);
				q_vec_type q;
				q_to_euler(q,obj.GetLocalXform().GetRot());
				SimulatedMicroscope->encode_and_sendRot(q[0],q[1],q[2]);
				cout << "rot Sent: " << "x: " << q[2] << "\ty: " << q[1] << "\tz: " << q[0] << endl;
				
			}
		}
	}
	else {
		import_update_AFM = 0;
	}
}

static  void handle_import_grab_object (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int g = import_grab_object;

// only do per object for now...

//		World.Do(&URender::SetGrabObjectAll, &g);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetGrabObject(import_grab_object);
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
    UTree *node = World.TGetNodeByName(*World.current_object);
    if (node != NULL) {
        URender &obj = node->TGetContents();
        obj.SetTess(import_tess);
    }
}

static  void handle_import_axis_step_change (vrpn_int32, void *)
{
    UTree *node = World.TGetNodeByName(*World.current_object);
    if (node != NULL) {
        URender &obj = node->TGetContents();
        obj.SetAxisStep(import_axis_step);
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

			// if a tube file and update_AFM selected, send scale
			if ((strstr(*World.current_object, ".txt") != 0) && 
				(SimulatedMicroscope != NULL) &&
				obj.GetUpdateAFM()) {
				SimulatedMicroscope->encode_and_sendScale(import_scale);
			}
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
			
			// if a tube file and update_AFM selected, send trans
			if ((strstr(*World.current_object, ".txt") != 0) && 
				(SimulatedMicroscope != NULL) &&
				obj.GetUpdateAFM()) {
				SimulatedMicroscope->encode_and_sendTrans(obj.GetLocalXform().GetTrans()[0],
															obj.GetLocalXform().GetTrans()[1],
															obj.GetLocalXform().GetTrans()[2]);
			}
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
			
			// if a tube file and update_AFM selected, send trans
			if ((strstr(*World.current_object, ".txt") != 0) && 
				(SimulatedMicroscope != NULL) &&
				obj.GetUpdateAFM()) {
				SimulatedMicroscope->encode_and_sendTrans(obj.GetLocalXform().GetTrans()[0],
															obj.GetLocalXform().GetTrans()[1],
															obj.GetLocalXform().GetTrans()[2]);
			}
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
						
			// if a tube file and update_AFM selected, send trans
			if ((strstr(*World.current_object, ".txt") != 0) && 
				(SimulatedMicroscope != NULL) &&
				obj.GetUpdateAFM()) {
				SimulatedMicroscope->encode_and_sendTrans(obj.GetLocalXform().GetTrans()[0],
															obj.GetLocalXform().GetTrans()[1],
															obj.GetLocalXform().GetTrans()[2]);
			}
		}
    }
}

static  void handle_import_rotx_change (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		double i = Q_DEG_TO_RAD(import_rotx);
		double j = Q_DEG_TO_RAD(import_roty);
		double k = Q_DEG_TO_RAD(import_rotz);
		double array[3];
		array[0] = i;
		array[1] = j;
		array[2] = k;
		World.Do(&URender::SetRotAll, (void *)array);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
		    URender &obj = node->TGetContents();

			q_vec_type euler;

			euler[2] = Q_DEG_TO_RAD(import_rotx);
			euler[1] = Q_DEG_TO_RAD(import_roty);
			euler[0] = Q_DEG_TO_RAD(import_rotz);

			q_type rot;
			q_from_euler(rot, euler[0], euler[1], euler[2]);

		    obj.GetLocalXform().SetRotate(rot[0], rot[1], rot[2], rot[3]);
						
			// if a tube file and update_AFM selected, send rot
			if ((strstr(*World.current_object, ".txt") != 0) && 
				(SimulatedMicroscope != NULL) &&
				obj.GetUpdateAFM()) {
				SimulatedMicroscope->encode_and_sendRot(euler[0], euler[1], euler[2]);
				cout << "rot Sent: " << "x: " << euler[2] << "\ty: " << euler[1] << "\tz: " << euler[0] << endl;
			}
		}
    }
}

static  void handle_import_roty_change (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		double i = Q_DEG_TO_RAD(import_rotx);
		double j = Q_DEG_TO_RAD(import_roty);
		double k = Q_DEG_TO_RAD(import_rotz);
		double array[3];
		array[0] = i;
		array[1] = j;
		array[2] = k;
		World.Do(&URender::SetRotAll, array);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
		    URender &obj = node->TGetContents();

			q_vec_type euler;

			euler[2] = Q_DEG_TO_RAD(import_rotx);
			euler[1] = Q_DEG_TO_RAD(import_roty);
			euler[0] = Q_DEG_TO_RAD(import_rotz);

			q_type rot;
			q_from_euler(rot, euler[0], euler[1], euler[2]);

		    obj.GetLocalXform().SetRotate(rot[0], rot[1], rot[2], rot[3]);
						
			// if a tube file and update_AFM selected, send rot
			if ((strstr(*World.current_object, ".txt") != 0) && 
				(SimulatedMicroscope != NULL) &&
				obj.GetUpdateAFM()) {
				SimulatedMicroscope->encode_and_sendRot(euler[0], euler[1], euler[2]);
				cout << "rot Sent: " << "x: " << euler[2] << "\ty: " << euler[1] << "\tz: " << euler[0] << endl;
			}
		}
    }
}

static  void handle_import_rotz_change (vrpn_float64, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		double i = Q_DEG_TO_RAD(import_rotx);
		double j = Q_DEG_TO_RAD(import_roty);
		double k = Q_DEG_TO_RAD(import_rotz);
		double array[3];
		array[0] = i;
		array[1] = j;
		array[2] = k;
		World.Do(&URender::SetRotAll, array);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
		    URender &obj = node->TGetContents();

			q_vec_type euler;

			euler[2] = Q_DEG_TO_RAD(import_rotx);
			euler[1] = Q_DEG_TO_RAD(import_roty);
			euler[0] = Q_DEG_TO_RAD(import_rotz);

			q_type rot;
			q_from_euler(rot, euler[0], euler[1], euler[2]);

		    obj.GetLocalXform().SetRotate(rot[0], rot[1], rot[2], rot[3]);
						
			// if a tube file and update_AFM selected, send rot
			if ((strstr(*World.current_object, ".txt") != 0) && 
				(SimulatedMicroscope != NULL) &&
				obj.GetUpdateAFM()) {
				SimulatedMicroscope->encode_and_sendRot(euler[0], euler[1], euler[2]);
				cout << "rot Sent: " << "x: " << euler[2] << "\ty: " << euler[1] << "\tz: " << euler[0] << endl;
			}
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


// Spider stuff
static  void handle_spider_length_change (vrpn_float64, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URender &obj = node->TGetContents();
		
		obj.SetSpiderLength(spider_length);
		obj.ReloadGeometry();
	}
}

static  void handle_spider_width_change (vrpn_float64, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URender &obj = node->TGetContents();
		
		obj.SetSpiderWidth(spider_width);
		obj.ReloadGeometry();
	}
}

static  void handle_spider_thick_change (vrpn_float64, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URender &obj = node->TGetContents();
		
		obj.SetSpiderThick(spider_thick);
		obj.ReloadGeometry();
	}
}

static  void handle_spider_tess_change (vrpn_int32, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URender &obj = node->TGetContents();
		
		obj.SetSpiderTess(spider_tess);
		obj.ReloadGeometry();
	}
}

static  void handle_spider_curve_change (vrpn_float64, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URender &obj = node->TGetContents();
		
		obj.SetSpiderCurve(Q_DEG_TO_RAD(spider_curve));
		obj.ReloadGeometry();
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
