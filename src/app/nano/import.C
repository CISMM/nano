#include "import.h"
#include "UTree.h"
#include "URPolygon.h"
#include "URTexture.h"
#include "FileGenerator.h"
#include "MSIFileGenerator.h"


//------------------------------------------------------------------------
//Functions necessary for all imported objects
static void handle_import_file_change (const char * , void *) {
    static char * old_name = NULL;
    static const float convert = 1.0f/255;
    
    if (modelFile.string()) {        
        URPolygon *obj = new URPolygon;
        FileGenerator *gen = FileGenerator::CreateFileGenerator(modelFile.string());
        import_type = gen->GetExtension();
        obj->LoadGeometry(gen);
        obj->SetVisibility(import_visibility);
        obj->SetColor3(convert * import_r, convert * import_g, convert * import_b);
        obj->SetAlpha(import_alpha);
        obj->GetLocalXform().SetScale(import_scale);
        obj->GetLocalXform().SetTranslate(import_transx, import_transy, import_transz);
        obj->GetLocalXform().SetRotate(import_rotx, import_roty, import_rotz, 1);
        World.TAddNode(obj, modelFile.string());

        if (old_name != NULL) {
            UTree *node = World.TGetNodeByName(old_name);

            node->TGetParent()->TRemoveTreeNode(node);
            
            delete node;
            delete [] old_name;
            old_name = NULL;
        }

        old_name = new char[strlen(modelFile.string())+1];
        strcpy(old_name, modelFile.string());
    }
}

static  void handle_import_visibility (vrpn_int32, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URender &obj = node->TGetContents();
        obj.SetVisibility(import_visibility);
    }
}

static  void handle_import_scale_change (vrpn_float64, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URender &obj = node->TGetContents();
        obj.GetLocalXform().SetScale(import_scale);
    }
}

static  void handle_import_transx_change (vrpn_float64, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URender &obj = node->TGetContents();
        const q_vec_type &trans = obj.GetLocalXform().GetTrans();
        obj.GetLocalXform().SetTranslate(import_transx, trans[1], trans[2]);
    }
}

static  void handle_import_transy_change (vrpn_float64, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URender &obj = node->TGetContents();
        const q_vec_type &trans = obj.GetLocalXform().GetTrans();
        obj.GetLocalXform().SetTranslate(trans[0], import_transy, trans[2]);
    }
}

static  void handle_import_transz_change (vrpn_float64, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URender &obj = node->TGetContents();
        const q_vec_type &trans = obj.GetLocalXform().GetTrans();
        obj.GetLocalXform().SetTranslate(trans[0], trans[1], import_transz);
    }
}

static  void handle_import_rotx_change (vrpn_float64, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URender &obj = node->TGetContents();
        const q_type &rot = obj.GetLocalXform().GetRot();
        obj.GetLocalXform().SetRotate(import_rotx, rot[1], rot[2], rot[3]);
    }
}

static  void handle_import_roty_change (vrpn_float64, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URender &obj = node->TGetContents();
        const q_type &rot = obj.GetLocalXform().GetRot();
        obj.GetLocalXform().SetRotate(rot[0], import_roty, rot[2], rot[3]);
    }
}

static  void handle_import_rotz_change (vrpn_float64, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URender &obj = node->TGetContents();
        const q_type &rot = obj.GetLocalXform().GetRot();
        obj.GetLocalXform().SetRotate(rot[0], rot[1], import_rotz, rot[3]);
    }
}

static  void handle_import_color_change (vrpn_int32, void *)
{
    static const float convert = 1.0f/255;
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URender &obj = node->TGetContents();
        obj.SetColor3(convert * import_r, convert * import_g, convert * import_b);
    }
}

static  void handle_import_alpha (vrpn_float64, void *)
{
    UTree *node = World.TGetNodeByName(modelFile.string());
    if (node != NULL) {
        URender &obj = node->TGetContents();
        obj.SetAlpha(import_alpha);
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