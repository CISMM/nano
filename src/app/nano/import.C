#include <vrpn_Types.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <UTree.h>
#include <URPolygon.h>
#include <URSpider.h>
#include <URWaveFront.h>
#include <FileGenerator.h>
#include <MSIFileGenerator.h>
#include <directstep.h>
#include <nmg_Graphics.h>

#include <string.h>
#include <stdlib.h>

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
static  void handle_import_lock_transx_change (vrpn_int32, void *);
static  void handle_import_lock_transy_change (vrpn_int32, void *);
static  void handle_import_lock_transz_change (vrpn_int32, void *);
static  void handle_import_rot_change (vrpn_float64, void *);
static  void handle_import_lock_rotx_change (vrpn_int32, void *);
static  void handle_import_lock_roty_change (vrpn_int32, void *);
static  void handle_import_lock_rotz_change (vrpn_int32, void *);
static  void handle_import_visibility (vrpn_int32, void *);
static  void handle_import_color_change (vrpn_int32, void *);
static  void handle_import_alpha (vrpn_float64, void *);
static  void handle_import_proj_text (vrpn_int32, void *);
static  void handle_import_text_image_mode_change(vrpn_int32, void*);
static	void handle_import_lock_object (vrpn_int32, void *);
static	void handle_import_lock_texture (vrpn_int32, void *);
static	void handle_import_CCW (vrpn_int32, void *);
static	void handle_import_update_AFM (vrpn_int32, void *);
static	void handle_import_grab_object (vrpn_int32, void *);
static	void handle_import_tune_trans_change (vrpn_int32, void *);
static	void handle_import_tune_rot_change (vrpn_int32, void *);

static	void handle_spider_current_leg(const char*, void*);
static	void handle_spider_length_change(vrpn_float64, void*);
static	void handle_spider_width_change(vrpn_float64, void*);
static	void handle_spider_thick_change(vrpn_float64, void*);
static	void handle_spider_tess_change(vrpn_int32, void*);
static	void handle_spider_beg_curve_change(vrpn_float64, void*);
static  void handle_spider_end_curve_change(vrpn_float64, void*);
static  void handle_spider_trans_leg_xy_change(vrpn_int32, void*);
static  void handle_spider_trans_leg_change(vrpn_float64, void*);
static  void handle_spider_rot_leg_change(vrpn_float64, void*);
static	void handle_spider_legs_change(vrpn_int32, void*);
static	void handle_spider_filename_change(const char*, void*);

//-----------------------------------------------------
//set direct step axis
static  void handle_set_ds_axis(vrpn_int32,void*);

///Import object handlers specifically for MSI files
static  void handle_msi_bond_mode (vrpn_int32, void *);
static  void handle_msi_atom_radius(vrpn_float64, void *);

// parse a .crv file
void parse_crv_file(const char*);

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
Tclvar_int		import_lock_transx("import_lock_transx",0, handle_import_lock_transx_change);
Tclvar_int		import_lock_transy("import_lock_transy",0, handle_import_lock_transy_change);
Tclvar_int		import_lock_transz("import_lock_transz",0, handle_import_lock_transz_change);
Tclvar_float	import_rotx("import_rotx",0, handle_import_rot_change);
Tclvar_float	import_roty("import_roty",0, handle_import_rot_change);
Tclvar_float	import_rotz("import_rotz",0, handle_import_rot_change);
Tclvar_int		import_lock_rotx("import_lock_rotx",0, handle_import_lock_rotx_change);
Tclvar_int		import_lock_roty("import_lock_roty",0, handle_import_lock_roty_change);
Tclvar_int		import_lock_rotz("import_lock_rotz",0, handle_import_lock_rotz_change);
Tclvar_int      import_visibility("import_visibility", 1, handle_import_visibility);
Tclvar_int      import_proj_text("import_proj_text", 1, handle_import_proj_text);
Tclvar_int      import_text_image_mode("import_text_image_mode", 1, handle_import_text_image_mode_change);
Tclvar_int		import_lock_object("import_lock_object", 0, handle_import_lock_object);
Tclvar_int		import_lock_texture("import_lock_texture", 0, handle_import_lock_texture);
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
Tclvar_int		import_tune_trans("import_tune_trans",0, handle_import_tune_trans_change);
Tclvar_int		import_tune_rot("import_tune_rot",0, handle_import_tune_rot_change);

Tclvar_list_of_strings spider_which_leg("spider_which_leg");
Tclvar_string	spider_current_leg("spider_current_leg", "", handle_spider_current_leg);
Tclvar_float    spider_length("spider_length", 5, handle_spider_length_change);
Tclvar_float    spider_width("spider_width", 2, handle_spider_width_change);
Tclvar_float    spider_thick("spider_thick", 0.1, handle_spider_thick_change);
Tclvar_int		spider_tess("spider_tess", 10, handle_spider_tess_change);
Tclvar_float    spider_beg_curve("spider_beg_curve", 0, handle_spider_beg_curve_change);
Tclvar_float    spider_end_curve("spider_end_curve", 0, handle_spider_end_curve_change);
Tclvar_int      spider_trans_leg_xy("spider_trans_leg_xy", 1, handle_spider_trans_leg_xy_change);
Tclvar_float    spider_trans_leg("spider_trans_leg", 0, handle_spider_trans_leg_change);
Tclvar_float    spider_rot_leg("spider_rot_leg", 0, handle_spider_rot_leg_change);
Tclvar_int		spider_legs("spider_legs", 8, handle_spider_legs_change);
Tclvar_string	spider_filename("spider_filename", "", handle_spider_filename_change);

int				current_leg = -1;		// integer value of the current leg
										// set when the tcl string value is changed

//-----------------------------------------------------------------
///SimulatedMicroscope object
extern nmm_SimulatedMicroscope_Remote* SimulatedMicroscope;

//-----------------------------------------------------------------
///nmg_Graphics object
extern nmg_Graphics* graphics;

//-----------------------------------------------------------------
///MSI File specific variables
Tclvar_int      msi_bond_mode("msi_bond_mode", 0, handle_msi_bond_mode);
Tclvar_float    msi_atom_radius("msi_atom_radius", 1, handle_msi_atom_radius);

//------------------------------------------------------------------------
//variables for direct step axis
Tclvar_int   set_ds_axis("setting_direct_step_axis", 0, handle_set_ds_axis);
extern int setting_axis;

//------------------------------------------------------------------------
//Functions necessary for all imported objects

static void handle_current_object_new(const char*, void*) {
	nmb_ListOfStrings temp;
	int i;

    // don't add if .crv file
    if (strstr(current_object_new.string(), ".crv") != 0) {
        return;
    }

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

			import_lock_transx = obj.GetLockTransx();
			import_lock_transy = obj.GetLockTransy();
			import_lock_transz = obj.GetLockTransz();

			q_vec_type euler;
			q_to_euler(euler,(double*)obj.
				GetLocalXform().GetRot());
			import_rotx = Q_RAD_TO_DEG(euler[2]);
			import_roty = Q_RAD_TO_DEG(euler[1]);
			import_rotz = Q_RAD_TO_DEG(euler[0]);

			import_lock_rotx = obj.GetLockRotx();
			import_lock_roty = obj.GetLockRoty();
			import_lock_rotz = obj.GetLockRotz();

			import_visibility = obj.GetVisibility();

			import_proj_text = obj.GetProjTextEnable();
			import_lock_object = obj.GetLockObject();
			import_lock_texture = obj.GetLockTexture();

			import_tune_trans = obj.GetTuneTrans();
			import_tune_rot = obj.GetTuneRot();

			import_grab_object = obj.GetGrabObject();

			// if wavefront
			if (obj.GetType() == URWAVEFRONT) {
				URWaveFront* wave = (URWaveFront*)&obj;
				import_CCW = wave->GetCCW();
			}
			// if tube file
			else if (obj.GetType() == URTUBEFILE) {
				URTubeFile* tube = (URTubeFile*)&obj;
				import_update_AFM = tube->GetUpdateAFM();
			}
			// if spider
			else if (obj.GetType() == URSPIDER) {
				URSpider* spi = (URSpider*)&obj;
				char buf[2];
				spider_which_leg.addEntry("all");
				for (int i = 0; i < spi->GetSpiderLegs(); i++) {
					sprintf(buf, "%d", i + 1);
					spider_which_leg.addEntry(buf);
				}

				if (current_leg == -1) {	// all

					// don't do anything--we don't want all legs set to the same value inadvertently

				}
				else {
					spider_length = spi->GetSpiderLength(current_leg);
					spider_width = spi->GetSpiderWidth(current_leg);
					spider_thick = spi->GetSpiderThick(current_leg);
					spider_tess = spi->GetSpiderTess(current_leg);
					spider_beg_curve = Q_RAD_TO_DEG(spi->GetSpiderBegCurve(current_leg));
                    spider_end_curve = Q_RAD_TO_DEG(spi->GetSpiderEndCurve(current_leg));
                    if (spider_trans_leg_xy == 1) {
                        spider_trans_leg = spi->GetSpiderLegX(current_leg);
                    }
                    else {
                        spider_trans_leg = spi->GetSpiderLegY(current_leg);
                    }
                    spider_rot_leg = spi->GetSpiderLegRot(current_leg);
				}
			}
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

	URPolygon* obj;
    if (modelFile.string()) {  
        if (strcmp(modelFile.string(),"") != 0) {	// open the file
            //Only try to create the object if there is a file specified.

            // Check for spi_curve file.  Don't want to generate a new object for this,
            // just change the spider curve values
            if (strstr(modelFile.string(), ".crv") != 0) {
                parse_crv_file(modelFile.string());
                return;
            }

			FileGenerator *gen = FileGenerator::CreateFileGenerator(modelFile.string());
            import_type = gen->GetExtension();

			if (strcmp(import_type, "obj") == 0) {
				// create as a wavefront object
				obj = new URWaveFront;
			}
			else if (strcmp(import_type, "spi") == 0) {
				// create as a spider object
				obj = new URSpider;
			}
			else if (strcmp(import_type, "txt") == 0) {
				// create as a tube file object
				obj = new URTubeFile;
			}
			else {
				// default to polygon object
				obj = new URPolygon;
			}

			// set up variables dependent on the object type
			if (obj->GetType() == URWAVEFRONT) {
				// set up wavefront stuff
				URWaveFront* wave = (URWaveFront*)obj;
				wave->SetCCW(import_CCW);
			}
			else if (obj->GetType() == URSPIDER) {
				// set up spider stuff
				URSpider* spi = (URSpider*)obj;
				for (int i = 0; i < spider_legs; i++) {
					spi->SetSpiderLength(i, spider_length);
					spi->SetSpiderWidth(i, spider_width);
					spi->SetSpiderThick(i, spider_thick);
					spi->SetSpiderTess(i, spider_tess);
					spi->SetSpiderBegCurve(i, Q_DEG_TO_RAD(spider_beg_curve));
                    spi->SetSpiderEndCurve(i, Q_DEG_TO_RAD(spider_end_curve));
                    spi->SetSpiderLegX(i, spider_trans_leg);
                    spi->SetSpiderLegY(i, spider_trans_leg);
                    spi->SetSpiderLegRot(i, spider_rot_leg);
				}
				spi->SetSpiderLegs(spider_legs);
			}
			else if (obj->GetType() == URTUBEFILE) {
				// set up tube file stuff
				URTubeFile* tube = (URTubeFile*)obj;
				tube->SetTess(import_tess);
				tube->SetAxisStep(import_axis_step);
			}

			// load the geometry for the object
			obj->LoadGeometry(gen);

			if (obj->GetType() == URSPIDER && strcmp(modelFile.string(), "/spider.spi") != 0) {
				// this is a spider loaded from a file.  change the name and set the number of spider legs
				modelFile = "/spider.spi";
				spider_legs = current_leg + 1;
			}


			// set up various variables
            obj->SetVisibility(import_visibility);
			obj->SetProjTextEnable(import_proj_text);
			obj->SetLockObject((import_lock_object!=0));
			obj->SetLockTexture(import_lock_texture);

			// only allow update_AFM for .txt files
			if (obj->GetType() == URTUBEFILE) {
				URTubeFile* tube = (URTubeFile*)obj;
				tube->SetUpdateAFM(1);
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
			obj->SetLockTransx(import_lock_transx);
			obj->SetLockTransy(import_lock_transy);
			obj->SetLockTransz(import_lock_transz);
			obj->SetLockRotx(import_lock_rotx);
			obj->SetLockRoty(import_lock_roty);
			obj->SetLockRotz(import_lock_rotz);
			obj->SetTuneTrans(import_tune_trans);
			obj->SetTuneRot(import_tune_rot);
			// truncate name so it correlates to the option menu
			// i.e. C:/Data/cube.obj -> cube.obj

			name = new char[strlen(modelFile.string()) + 1];
			strcpy(name, modelFile.string());
			c = name + strlen(name);
			while (*(--c) != '/');
			World.TAddNode(obj, c + 1);

			// if a tube file, send to simulator
			if ((obj->GetType() == URTUBEFILE) && (SimulatedMicroscope != NULL)) {
				URTubeFile* tube = (URTubeFile*)obj;
				SimulatedMicroscope->sendCylinders(tube);
			}
			
			*World.current_object = name;

			// had to put this here so that the correct current_object is used
			if (obj->GetType() == URTUBEFILE) {
				URTubeFile* tube = (URTubeFile*)obj;
				import_update_AFM = tube->GetUpdateAFM();
			}

			delete [] name;
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

void parse_crv_file(const char* fname) {
    char buffer[512];
	char* token;
	ifstream readfile;

    // Check to make sure the current object is a spider
    if (strcmp(*World.current_object, "spider.spi") != 0) {
        printf("Error.  Current object is not a spider object.\n");
        return;
    }
		
    UTree *node = World.TGetNodeByName("spider.spi");
	URSpider &obj = (URSpider&)node->TGetContents();

    readfile.open(fname);
    assert(readfile);

    if(readfile.bad()) {
		cerr << "Unable to open input file" << endl;
        return;
    }

    int i = 0;
    while(!readfile.eof()) {
		readfile.getline(buffer, 512);

        // skip Leg column
		token = strtok(buffer, " \t\n");

		if (token != NULL) {
            // get beginning curvature
			token = strtok(NULL, " \t\n");
            obj.SetSpiderBegCurve(i, Q_DEG_TO_RAD(atof(token)));

            // get ending curvature
			token = strtok(NULL, " \t\n");
            obj.SetSpiderEndCurve(i, Q_DEG_TO_RAD(atof(token)));
		}
        i++;
	}
    
    obj.ReloadGeometry();

    readfile.close();
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
		World.Do(&URender::SetProjTextEnableAll, &i);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetProjTextEnable(import_proj_text);
		}
	}
}

static void handle_import_text_image_mode_change (vrpn_int32, void*) {
    if (import_text_image_mode == 1) {
        // SURFACE
        if (graphics->getTextureMode() == nmg_Graphics::COLORMAP) {
            graphics->setTextureMode(nmg_Graphics::COLORMAP, 
                                        nmg_Graphics::SURFACE_REGISTRATION_COORD);
        }
        else {
            graphics->setTextureMode(nmg_Graphics::VIDEO,
                                        nmg_Graphics::SURFACE_REGISTRATION_COORD);
        }
    }
    else {
        // MODEL
        if (graphics->getTextureMode() == nmg_Graphics::COLORMAP) {
            graphics->setTextureMode(nmg_Graphics::COLORMAP, 
                                        nmg_Graphics::MODEL_REGISTRATION_COORD);
        }
        else {
            graphics->setTextureMode(nmg_Graphics::VIDEO,
                                        nmg_Graphics::MODEL_REGISTRATION_COORD);
        }

        // set tcl callbacks to create an object for the texture
		if (World.TGetNodeByName("projtextobj.ptx") == NULL) {
	        modelFile = "/projtextobj.ptx";
			current_object_new = "projtextobj.ptx";
		}
    }
}

static  void handle_import_lock_object (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int i = import_lock_object;
		World.Do(&URender::SetLockObjectAll, &i);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetLockObject(import_lock_object!=0);
		}
	}
}

static  void handle_import_lock_texture (vrpn_int32, void *)
{
	int lock = (int)import_lock_texture;
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		World.Do(&URender::SetLockTextureAll, &lock);
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetLockTexture(lock);
		}
	}
}

static  void handle_import_update_AFM (vrpn_int32, void *)
{

// ONLY NEED FOR TUBE OBJECTS...FOR NOW



	// only do on a per object basis...

	// if all selected, do nothing
	if (strcmp(*World.current_object, "all") != 0) {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			if (node->TGetContents().GetType() == URTUBEFILE) {
				URTubeFile &tube = (URTubeFile&)node->TGetContents();
				tube.SetUpdateAFM(import_update_AFM);

				// do update
				if (SimulatedMicroscope != NULL &&
					tube.GetUpdateAFM()) {
					SimulatedMicroscope->encode_and_sendScale(tube.GetLocalXform().GetScale());
					SimulatedMicroscope->encode_and_sendTrans(tube.GetLocalXform().GetTrans()[0],
																tube.GetLocalXform().GetTrans()[1],
																tube.GetLocalXform().GetTrans()[2]);
					q_vec_type q;
					q_to_euler(q,(double*)tube.GetLocalXform().GetRot());
					SimulatedMicroscope->encode_and_sendRot(q[0],q[1],q[2]);
				}
				return;
			}
		}
	}
	// if we get here, it is not a tube file, so set to 0
	import_update_AFM = 0;
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
        URWaveFront &obj = (URWaveFront&)node->TGetContents();
        obj.SetCCW(import_CCW);
    }
}

// For Tube Files
static  void handle_import_tess_change (vrpn_int32, void *)
{
    UTree *node = World.TGetNodeByName(*World.current_object);
    if (node != NULL) {
        URTubeFile &obj = (URTubeFile&)node->TGetContents();
        obj.SetTess(import_tess);
    }
}

// For Tube Files
static  void handle_import_axis_step_change (vrpn_int32, void *)
{
    UTree *node = World.TGetNodeByName(*World.current_object);
    if (node != NULL) {
        URTubeFile &obj = (URTubeFile&)node->TGetContents();
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
			if (obj.GetType() == URTUBEFILE && 
				SimulatedMicroscope != NULL) {
				URTubeFile* tube = (URTubeFile*)&obj;
				if (tube->GetUpdateAFM()) {
					SimulatedMicroscope->encode_and_sendScale(import_scale);
				}
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
			// Note:  Locks are only applied when "grabbing" the object with
			// the phantom/mouse phantom.  No check is applied here.
			URender &obj = node->TGetContents();
			const q_vec_type &trans = obj.GetLocalXform().GetTrans();
			obj.GetLocalXform().SetTranslate(import_transx, trans[1], trans[2]);
			
			// if a tube file and update_AFM selected, send trans
			if (obj.GetType() == URTUBEFILE && 
				SimulatedMicroscope != NULL) {
				URTubeFile* tube = (URTubeFile*)&obj;
				if (tube->GetUpdateAFM()) {
					SimulatedMicroscope->encode_and_sendTrans(obj.GetLocalXform().GetTrans()[0],
															obj.GetLocalXform().GetTrans()[1],
															obj.GetLocalXform().GetTrans()[2]);
				}
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
			// Note:  Locks are only applied when "grabbing" the object with
			// the phantom/mouse phantom.  No check is applied here.
			URender &obj = node->TGetContents();
			const q_vec_type &trans = obj.GetLocalXform().GetTrans();
			obj.GetLocalXform().SetTranslate(trans[0], import_transy, trans[2]);
			
			// if a tube file and update_AFM selected, send trans
			if (obj.GetType() == URTUBEFILE && 
				SimulatedMicroscope != NULL) {
				URTubeFile* tube = (URTubeFile*)&obj;
				if (tube->GetUpdateAFM()) {
					SimulatedMicroscope->encode_and_sendTrans(obj.GetLocalXform().GetTrans()[0],
															obj.GetLocalXform().GetTrans()[1],
															obj.GetLocalXform().GetTrans()[2]);
				}
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
			// Note:  Locks are only applied when "grabbing" the object with
			// the phantom/mouse phantom.  No check is applied here.
			URender &obj = node->TGetContents();
			const q_vec_type &trans = obj.GetLocalXform().GetTrans();
			obj.GetLocalXform().SetTranslate(trans[0], trans[1], import_transz);
						
			// if a tube file and update_AFM selected, send trans
			if (obj.GetType() == URTUBEFILE && 
				SimulatedMicroscope != NULL) {
				URTubeFile* tube = (URTubeFile*)&obj;
				if (tube->GetUpdateAFM()) {
					SimulatedMicroscope->encode_and_sendTrans(obj.GetLocalXform().GetTrans()[0],
															obj.GetLocalXform().GetTrans()[1],
															obj.GetLocalXform().GetTrans()[2]);
				}
			}
		}
    }
}

static  void handle_import_rot_change (vrpn_float64, void *)
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
			// Note:  Locks are only applied when "grabbing" the object with
			// the phantom/mouse phantom.  No check is applied here.

		    URender &obj = node->TGetContents();
			q_type rot;
			q_vec_type euler;

			euler[2] = Q_DEG_TO_RAD(import_rotx);
			euler[1] = Q_DEG_TO_RAD(import_roty);
			euler[0] = Q_DEG_TO_RAD(import_rotz);

			q_from_euler(rot, euler[0], euler[1], euler[2]);

			obj.GetLocalXform().SetRotate(rot[0], rot[1], rot[2], rot[3]);

			if (set_ds_axis == 1) {
				//current object the direct step axis?
				if (strstr(*World.current_object, ".dsa") != 0) {
					//send the rotation to direct step
					set_axis(rot);
				}	
			}
						
			// if a tube file and update_AFM selected, send rot
			if (obj.GetType() == URTUBEFILE && 
				SimulatedMicroscope != NULL) {
				URTubeFile* tube = (URTubeFile*)&obj;
				if (tube->GetUpdateAFM()) {
					SimulatedMicroscope->encode_and_sendRot(euler[0], euler[1], euler[2]);
				}
			}
		}
    }
}


static  void handle_import_lock_transx_change (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int l = import_lock_transx;

// only do per object for now...

//		World.Do
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetLockTransx(import_lock_transx);
		}
	}
}

static  void handle_import_lock_transy_change (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int l = import_lock_transy;

// only do per object for now...

//		World.Do
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetLockTransy(import_lock_transy);
		}
	}
}

static  void handle_import_lock_transz_change (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int l = import_lock_transz;

// only do per object for now...

//		World.Do
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetLockTransz(import_lock_transz);
		}
	}
}

static  void handle_import_lock_rotx_change (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int l = import_lock_rotx;

// only do per object for now...

//		World.Do
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetLockRotx(import_lock_rotx);
		}
	}
}

static  void handle_import_lock_roty_change (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int l = import_lock_roty;

// only do per object for now...

//		World.Do
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetLockRoty(import_lock_roty);
		}
	}
}

static  void handle_import_lock_rotz_change (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int l = import_lock_rotz;

// only do per object for now...

//		World.Do
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetLockRotz(import_lock_rotz);
		}
	}
}

static  void handle_import_tune_trans_change (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int l = import_lock_transx;

// only do per object for now...

//		World.Do
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetTuneTrans(import_tune_trans);
		}
	}
}

static  void handle_import_tune_rot_change (vrpn_int32, void *)
{
	// if all selected, do for all loaded objects
	if (strcmp(*World.current_object, "all") == 0) {
		int l = import_lock_transx;

// only do per object for now...

//		World.Do
	}
	else {
		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URender &obj = node->TGetContents();
			obj.SetTuneRot(import_tune_rot);
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

static void handle_spider_current_leg(const char*, void*) 
{
	if (strcmp(spider_current_leg.string(), "all") == 0) {
		// set to -1
		current_leg = -1;
	}
	else {
		current_leg = atoi(spider_current_leg.string()) - 1;

		UTree *node = World.TGetNodeByName(*World.current_object);
		if (node != NULL) {
			URSpider &obj = (URSpider&)node->TGetContents();
			spider_length = obj.GetSpiderLength(current_leg);
			spider_width = obj.GetSpiderWidth(current_leg);
			spider_thick = obj.GetSpiderThick(current_leg);
			spider_tess = obj.GetSpiderTess(current_leg);
			spider_beg_curve = Q_RAD_TO_DEG(obj.GetSpiderBegCurve(current_leg));
            spider_end_curve = Q_RAD_TO_DEG(obj.GetSpiderEndCurve(current_leg));
            if (spider_trans_leg_xy == 1) {
                spider_trans_leg = obj.GetSpiderLegX(current_leg);
            }
            else {
                spider_trans_leg = obj.GetSpiderLegY(current_leg);
            }
            spider_rot_leg = obj.GetSpiderLegRot(current_leg);
		}
	}
}

static  void handle_spider_length_change (vrpn_float64, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URSpider &obj = (URSpider&)node->TGetContents();
	
		if (current_leg == -1) {
			// do for all
			for (int i = 0; i < obj.GetSpiderLegs(); i++) {
				obj.SetSpiderLength(i, spider_length);
			}
		}
		else {
			obj.SetSpiderLength(current_leg, spider_length);
		}
		obj.ReloadGeometry();
	}
}

static  void handle_spider_width_change (vrpn_float64, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URSpider &obj = (URSpider&)node->TGetContents();
		
		if (current_leg == -1) {
			// do for all
			for (int i = 0; i < obj.GetSpiderLegs(); i++) {
				obj.SetSpiderWidth(i, spider_width);
			}
		}
		else {
			obj.SetSpiderWidth(current_leg, spider_width);
		}
		obj.ReloadGeometry();
	}
}

static  void handle_spider_thick_change (vrpn_float64, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URSpider &obj = (URSpider&)node->TGetContents();
		
		if (current_leg == -1) {
			// do for all
			for (int i = 0; i < obj.GetSpiderLegs(); i++) {
				obj.SetSpiderThick(i, spider_thick);
			}
		}
		else {
			obj.SetSpiderThick(current_leg, spider_thick);
		}
		obj.ReloadGeometry();
	}
}

static  void handle_spider_tess_change (vrpn_int32, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URSpider &obj = (URSpider&)node->TGetContents();
		
		if (current_leg == -1) {
			// do for all
			for (int i = 0; i < obj.GetSpiderLegs(); i++) {
				obj.SetSpiderTess(i, spider_tess);
			}
		}
		else {
			obj.SetSpiderTess(current_leg, spider_tess);
		}
		obj.ReloadGeometry();
	}
}

static  void handle_spider_beg_curve_change (vrpn_float64, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URSpider &obj = (URSpider&)node->TGetContents();
		
		if (current_leg == -1) {
			// do for all
			for (int i = 0; i < obj.GetSpiderLegs(); i++) {
				obj.SetSpiderBegCurve(i, Q_DEG_TO_RAD(spider_beg_curve));
			}
		}
		else {
			obj.SetSpiderBegCurve(current_leg, Q_DEG_TO_RAD(spider_beg_curve));
		}
		obj.ReloadGeometry();
	}
}

static  void handle_spider_end_curve_change (vrpn_float64, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URSpider &obj = (URSpider&)node->TGetContents();
		
		if (current_leg == -1) {
			// do for all
			for (int i = 0; i < obj.GetSpiderLegs(); i++) {
				obj.SetSpiderEndCurve(i, Q_DEG_TO_RAD(spider_end_curve));
			}
		}
		else {
			obj.SetSpiderEndCurve(current_leg, Q_DEG_TO_RAD(spider_end_curve));
		}
		obj.ReloadGeometry();
	}
}

static  void handle_spider_trans_leg_xy_change (vrpn_int32, void*)
{
    if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URSpider &obj = (URSpider&)node->TGetContents();
		
		if (current_leg != -1) {
            if (spider_trans_leg_xy == 1) {
                spider_trans_leg = obj.GetSpiderLegX(current_leg);
            }
            else {
                spider_trans_leg = obj.GetSpiderLegY(current_leg);
            }
		}
		obj.ReloadGeometry();
	}
}

static  void handle_spider_trans_leg_change (vrpn_float64, void*)
{
    if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URSpider &obj = (URSpider&)node->TGetContents();
		
		if (current_leg != -1) {
            if (spider_trans_leg_xy == 1) {
			    obj.SetSpiderLegX(current_leg, spider_trans_leg);
            }
            else {
                obj.SetSpiderLegY(current_leg, spider_trans_leg);
            }
		}
		obj.ReloadGeometry();
	}
}

static  void handle_spider_rot_leg_change (vrpn_float64, void*)
{
    if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URSpider &obj = (URSpider&)node->TGetContents();
		
		if (current_leg != -1) {
			    obj.SetSpiderLegRot(current_leg, spider_rot_leg);
		}
		obj.ReloadGeometry();
	}
}

static  void handle_spider_legs_change (vrpn_int32, void *)
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");
		URSpider &obj = (URSpider&)node->TGetContents();

		// update the list of strings
		int i;
		char buf[2];
		if (obj.GetSpiderLegs() < spider_legs) {
			for (i = obj.GetSpiderLegs() + 1; i <= spider_legs; i++) {
				sprintf(buf, "%d", i);
				spider_which_leg.addEntry(buf);
			}
		}
		else {
			for (i = spider_legs + 1; i <= obj.GetSpiderLegs(); i++) {
				sprintf(buf, "%d", i);
				spider_which_leg.deleteEntry(buf);
			}
		}

		obj.SetSpiderLegs(spider_legs);

		obj.ReloadGeometry();
	}
}

static void handle_spider_filename_change (const char*, void*) 
{
	if (strcmp(*World.current_object, "spider.spi") == 0) {
		UTree *node = World.TGetNodeByName("spider.spi");

		if (node != NULL) {
			URSpider &obj = (URSpider&)node->TGetContents();

			if (strstr(spider_filename, "spider.spi") != 0) {
				printf("Can't use filename \"spider.spi\".  Save under a different name\n");
			}
			else {
				obj.SaveSpider(spider_filename);
			}
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
//-------------------------------------------------------
//direct step axis specific function
// for setting an axis with direct step.
static void handle_set_ds_axis(vrpn_int32 i, void*) {
setting_axis = i;
}
