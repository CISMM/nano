#include "UTree.h"
#include "URTubeFile.h"
#include "URPolygon.h"

#include <nmm_SimulatedMicroscope_Remote.h>	// so we know if there is an open connection for
											// scaling tubes when we change scale 

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

extern nmm_SimulatedMicroscope_Remote* SimulatedMicroscope;

URTubeFile::URTubeFile():URPolygon() {
	obj_type = URTUBEFILE;

	tess = 10;
	axis_step = 10;


}

URTubeFile::~URTubeFile() {
	delete cylinders;
}

int URTubeFile::ChangeStaticFile(void* userdata) {
    // modifies the scale and translation so appears in same place...
	int i;
	extern Tclvar_float	import_scale;
	extern Tclvar_float import_transx;
	extern Tclvar_float import_transy;
	extern Tclvar_float import_transz;
	extern Tclvar_string current_object;

	change_static_file csf = *(change_static_file*) userdata;

	this->GetLocalXform().SetXOffset(csf.xoffset);
	this->GetLocalXform().SetYOffset(csf.yoffset);
	this->GetLocalXform().SetZOffset(csf.zoffset);

	this->GetLocalXform().SetScale(this->GetLocalXform().GetScale() * csf.scale);

	// if current object, update the tcl stuff
	if (strcmp(this->name, current_object.string()) == 0) {
		import_scale = this->GetLocalXform().GetScale();
	}


	const q_vec_type &q1 = this->GetLocalXform().GetTrans();

	q_vec_type q2, q3;

	q_vec_copy(q2, (double*)q1);
	q_vec_copy(q3, (double*)q1);

	q_vec_scale(q2, csf.scale, q2);

	this->GetLocalXform().SetTranslate(q2);

	// if current object, update the tcl stuff
	if (strcmp(this->name, current_object.string()) == 0) {
		import_transx = q2[0];
		import_transy = q2[1];
		import_transz = q2[2];
	}

	// line up cylinders to send to AFM properly
	for (i = 0; i < num_cylinders; i++) {
		cylinders[i].x1 += q3[0] - q2[0];
		cylinders[i].y1 += q3[1] - q2[1];
		cylinders[i].z1 += q3[2] - q2[2];
		cylinders[i].x2 += q3[0] - q2[0];
		cylinders[i].y2 += q3[1] - q2[1];
		cylinders[i].z2 += q3[2] - q2[2];
	}

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URTubeFile::ScaleAll(void* userdata) {
	double scale = *(double*) userdata;

	this->GetLocalXform().SetScale(scale);

	// send scale
	if (SimulatedMicroscope != NULL &&
		this->GetUpdateAFM()) {
		SimulatedMicroscope->encode_and_sendScale(scale);
	}

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URTubeFile::SetTransxAll(void* userdata) {	
	double transx = *(double*) userdata;
	const q_vec_type &trans = this->GetLocalXform().GetTrans();
	this->GetLocalXform().SetTranslate(transx, trans[1], trans[2]);

	// send trans
	if (SimulatedMicroscope != NULL &&
		this->GetUpdateAFM()) {
		SimulatedMicroscope->encode_and_sendTrans(this->GetLocalXform().GetTrans()[0],
													this->GetLocalXform().GetTrans()[1],
													this->GetLocalXform().GetTrans()[2]);
	}

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}

int URTubeFile::SetTransyAll(void* userdata) {	
	double transy = *(double*) userdata;
	const q_vec_type &trans = this->GetLocalXform().GetTrans();
	this->GetLocalXform().SetTranslate(trans[0], transy, trans[2]);

	// send trans
	if (SimulatedMicroscope != NULL &&
		this->GetUpdateAFM()) {
		SimulatedMicroscope->encode_and_sendTrans(this->GetLocalXform().GetTrans()[0],
													this->GetLocalXform().GetTrans()[1],
													this->GetLocalXform().GetTrans()[2]);
	}

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}

int URTubeFile::SetTranszAll(void* userdata) {	
	double transz = *(double*) userdata;
	const q_vec_type &trans = this->GetLocalXform().GetTrans();
	this->GetLocalXform().SetTranslate(trans[0], trans[1], transz);

	// send trans
	if (SimulatedMicroscope != NULL &&
		this->GetUpdateAFM()) {
		SimulatedMicroscope->encode_and_sendTrans(this->GetLocalXform().GetTrans()[0],
													this->GetLocalXform().GetTrans()[1],
													this->GetLocalXform().GetTrans()[2]);
	}

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}


int URTubeFile::SetRotAll(void* userdata) {
	double* array = (double*)userdata;
	double rotx = array[0];
	double roty = array[1];
	double rotz = array[2];

	q_vec_type euler;

	euler[2] = rotx;
	euler[1] = roty;
	euler[0] = rotz;

	q_type rot;
	q_from_euler(rot, euler[0], euler[1], euler[2]);

    this->GetLocalXform().SetRotate(rot[0], rot[1], rot[2], rot[3]);

	// send rot
	if (SimulatedMicroscope != NULL &&
		this->GetUpdateAFM()) {
		SimulatedMicroscope->encode_and_sendRot(euler[0], euler[1], euler[2]);
		cout << "rot Sent: " << "x: " << euler[2] << "\ty: " << euler[1] << "\tz: " << euler[0] << endl;
	}

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}


