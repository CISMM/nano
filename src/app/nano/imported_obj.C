
/** \file imported_obj.C

Last modified 3/1/01 by Jason Clark
  This file is extremely out of date!!!  I am modifying just to allow
  compilation, however this file assumes that is is importing MSI files
  which is a bad assumption to make.

Last modified 7/1/99 by Leila Plummer

This file contains methods for classes imported_obj and imported_obj_list,
which are used for importing objects, such as nanotubes from tube_foundry,
using uberGraphics

Things which I didn't get around to, and which critically need to be done: 
--Add the ability to scale objects once they have been imported
  (this can probably be done fairly easily by adding a TCL scale variable
  to each object, and then modifying import_obj_list to monitor and change
  the uberGraphics scaling based upon this)
--Change the view window so that you can access it after a following object
  has been created--right now this software crashes when this occurs,
  because $page_count has already been modified.  We need to find a way to
  pass the CURRENT $page_count as a parameter
--Fix TCl sliders in View window so that they actually slide
--Add Delete Object capabilities
--Translate endpoints for translate_x, etc. need to be in sync with whatever
  microscape is doing to determine window size
--Verify that sphere tesselation actually affects something.

Things which I didn't get around to, and which probably should be done:
--Cause variables such as bond_width to be initialized in one or two places 
  only, not several
--Consider making View window a part of the import_objects window, rather
  than a separate window
--Place object filenames at top of tabs, rather than "Object #"
--Merge my sphere class with sphere stuff in globjects.c
*/

#include "imported_obj.h"
#include <MSIFileGenerator.h>
#include <FileGenerator.h>

// ############ getpid hack
#if defined (__CYGWIN__) && defined (VRPN_USE_WINSOCK_SOCKETS)

/* #include <sys/unistd.h>  // for getpid() */

// cannot include sys/unistd.h because it would cause a conflict with the
// windows-defined gethostbyname.  Instead, I'll declare getpid myslef.  This
// is really ugly and dangerous.
extern "C" {
pid_t getpid();
}


// there is also a different getpid defined in Process.h in the VC-6.0 include
// directory.  I think it has a different return type.

#endif
// ############

// M_PI not defined for VC++, for some reason. 
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

#ifndef Q_X
#define Q_X 0
#endif
#ifndef Q_Y
#define Q_Y 1
#endif
#ifndef Q_Z
#define Q_Z 2
#endif
#ifndef Q_W
#define Q_W 3
#endif

int imported_obj::imported_obj_count = 0; //initialize variable which keeps
                                          //track of how many objects have
                                          //been created

/*****************************************************************************
Methods for imported_obj
*****************************************************************************/

/** PURPOSE: Default constructor for imported_obj.  
    Initializes basic variables, including
    Tcl widgets to be shown in import_objects and View windows.  Sets up Tcl
    callbacks for Tcl widgets.*/
imported_obj::imported_obj(){
  char widget_name[256];
  char page_name[256];
  char view_name[256];

  //be careful with changin initialization values here -- you must also 
  //change the corresponding TCL variables, and the corresponding variables
  //in the MSI files in the ugraphics directory

  x_last_rotated = 0.0;
  y_last_rotated = 0.0;
  z_last_rotated = 0.0;

  bond_width_value = 1.0;
  sphere_radius_value = 1.0;
  sphere_tesselation_value = 2;
  bond_colorR_value = 0.0;
  bond_colorG_value = 0.0;
  bond_colorB_value = 1.0;
  sphere_colorR_value = 0.0;
  sphere_colorG_value = 0.0;
  sphere_colorB_value = 1.0;

  import_mode_value = 0; //assume that we start out in bond mode
  visibility_mode_value = 1; //assume that we start out with object visible
 
  imported_obj_count++;
  object_number = imported_obj_count;
  URPoly = new URPolygon;

  //set up widgets for import_objects window and import mode 
  sprintf(widget_name, "translate_x%d",imported_obj_count);
  sprintf(page_name, "$pages(%d).pageframe", imported_obj_count);
  translate_x = new Tclvar_int(widget_name, 0);
  sprintf(widget_name, "translate_y%d", imported_obj_count);
  translate_y = new Tclvar_int(widget_name,0);
  sprintf(widget_name, "translate_z%d", imported_obj_count);
  translate_z = new Tclvar_int(widget_name,0);
  sprintf(widget_name, "rotate_x%d", imported_obj_count);
  rotate_x = new Tclvar_int(widget_name,0);
  sprintf(widget_name, "rotate_y%d", imported_obj_count);
  rotate_y = new Tclvar_int(widget_name,0);
  sprintf(widget_name, "rotate_z%d", imported_obj_count);
  rotate_z = new Tclvar_int(widget_name,0);
  sprintf(widget_name, "scale%d", imported_obj_count);
  scale = new Tclvar_float(widget_name, 20);
  sprintf(widget_name, "import_mode(%d)",imported_obj_count);
  import_mode = new Tclvar_int(widget_name,0);
  sprintf(widget_name, "visibility_mode(%d)",imported_obj_count);
  visibility_mode = new Tclvar_int(widget_name,0);
  /*sprintf(widget_name, "delete_object(%d)",imported_obj_count);
  delete_object = new Tclvar_int(widget_name,0);*/


  //set up widgets for view window -- at least for those that we will need in 
  //bond mode, which we assume that we start out in
  sprintf(view_name, "$viewobj(%d).viewframe", imported_obj_count);
  sprintf(widget_name, "bond_width%d",imported_obj_count);
  bond_width = new Tclvar_float(widget_name,1.0);
  sprintf(widget_name, "bond_colorR%d",imported_obj_count);
  bond_colorR = new Tclvar_float(widget_name,0.0);
  sprintf(widget_name, "bond_colorG%d",imported_obj_count);
  bond_colorG = new Tclvar_float(widget_name,0.0);
  sprintf(widget_name, "bond_colorB%d",imported_obj_count);
  bond_colorB = new Tclvar_float(widget_name,1.0);
  set_tcl_change_callback();

  next = NULL;
} /*imported_obj::imported_obj*/

void imported_obj::set_tcl_change_callback(){
/*PURPOSE: Sets up Tcl callback functions for import_objects window and 
           View window, assuming that we begin in bond mode*/
  translate_x->addCallback(handle_translate_x_change, this);
  translate_y->addCallback(handle_translate_y_change, this);
  translate_z->addCallback(handle_translate_z_change, this);
  rotate_x->addCallback(handle_rotate_x_change, this);
  rotate_y->addCallback(handle_rotate_y_change, this);
  rotate_z->addCallback(handle_rotate_z_change, this);
  scale->addCallback(handle_scale_change, this);
  import_mode->addCallback(handle_import_mode_change,this);
  visibility_mode->addCallback(handle_visibility_mode_change,this);
  //delete_object->addCallback(handle_delete_object_change,this);
  //assumes that we start out in bond mode
  bond_width->addCallback(handle_bond_width_change,this);
  bond_colorR->addCallback(handle_bond_colorR_change,this);
  bond_colorG->addCallback(handle_bond_colorG_change,this);
  bond_colorB->addCallback(handle_bond_colorB_change,this);
} /*imported_obj::set_tcl_change_callback*/

/** @return The object's URPolygon*/
URPolygon* imported_obj::GetURPoly(){
  return URPoly;
} /*imported_obj::GetURPoly*/

/** PURPOSE: Handle a change in the translate_x Tcl slider -- 
    Translates object to x-position new_value*/
void imported_obj::handle_translate_x_change(vrpn_int32 new_value,void *userdata){
  const double *t;
  imported_obj *me = (imported_obj *)userdata;

  t = me->URPoly->GetLocalXform().GetTrans();
  me->URPoly->GetLocalXform().SetTranslate(new_value,t[1],t[2]);
} /*imported_obj::handle_translate_x_change*/

/** PURPOSE: Handle a change in the translate_y Tcl slider --
    Translates object to y-position new_value*/
void imported_obj::handle_translate_y_change(vrpn_int32 new_value,void *userdata){
  const double *t;
  imported_obj *me = (imported_obj *)userdata;

  t = me->URPoly->GetLocalXform().GetTrans();
  me->URPoly->GetLocalXform().SetTranslate(t[0],new_value,t[2]);
} /*imported_obj::handle_translate_y_change*/

/** PURPOSE: Handle a change in the translate_z Tcl slider --
           Translates object to z-position new_value*/
void imported_obj::handle_translate_z_change(vrpn_int32 new_value,void *userdata){
  const double *t;
  imported_obj *me = (imported_obj *)userdata;

  t = me->URPoly->GetLocalXform().GetTrans();
  me->URPoly->GetLocalXform().SetTranslate(t[0],t[1],new_value);
} /*imported_obj::handle_translate_z_change*/

/** PURPOSE: Handle a change in the rotate_x Tcl slider --
           Causes combined rotation around x-axis to be new_value degrees
NOTE: Basically takes a rotation function from the quat library and 
           modifies it for our purposes**/
void imported_obj::handle_rotate_x_change (vrpn_int32 new_value, void *userdata) {
  q_type r;
  double length, cosA, sinA;
  double x, y, z, angle, desired_angle;
  imported_obj *me = (imported_obj *)userdata;

  x = 1;
  y = 0;
  z = 0;

  //we want to rotate only the change in angle values since last rotation,
  //not the full new angle value (eg if we rotated 60 degrees last time and
  //the slider now says to rotate 90 degrees, we only want to rotate 30 
  //degrees) so we subtract last_rotated from desired_angle, and convert
  //to radians
  desired_angle = new_value*M_PI/180;
  angle = desired_angle - me->x_last_rotated;
  me->x_last_rotated = desired_angle;

  #define  Q_EPSILON   (1e-10)

  /* normalize vector */

  length = sqrt( x*x + y*y + z*z );

  /* if zero vector passed in, just return identity quaternion	*/

  if ( length < Q_EPSILON )
  {
    r[Q_X] = 0;
    r[Q_Y] = 0;
    r[Q_Z] = 0;
    r[Q_W] = 1;
    me->URPoly->GetLocalXform().AddRotate(r);
    return;
  }
  x /= length;
  y /= length;
  z /= length;
  cosA = cos(angle / 2.0);
  sinA = sin(angle / 2.0);
  r[Q_W] = cosA;
  r[Q_X] = sinA * x;
  r[Q_Y] = sinA * y;
  r[Q_Z] = sinA * z;
  me->URPoly->GetLocalXform().AddRotate(r);
} /*imported_obj::handle_rotate_x_change*/

/** PURPOSE: Handle a change in the rotate_y Tcl slider --
           Causes combined rotation around y-axis to be new_value degrees
NOTE: Basically takes a rotation function from the quat library and 
           modifies it for our purposes**/
void imported_obj::handle_rotate_y_change (vrpn_int32 new_value, void *userdata) {
  q_type r;
  double length, cosA, sinA;
  double x, y, z, angle, desired_angle;
  imported_obj *me = (imported_obj *)userdata;

  x = 0;
  y = 1;
  z = 0;

  //we want to rotate only the change in angle values since last rotation,
  //not the full new angle value (eg if we rotated 60 degrees last time and
  //the slider now says to rotate 90 degrees, we only want to rotate 30 
  //degrees) so we subtract last_rotated from desired_angle, and convert
  //to radians
  desired_angle = new_value*M_PI/180;
  angle = desired_angle - me->y_last_rotated;
  me->y_last_rotated = desired_angle;

  #define  Q_EPSILON   (1e-10)

  /* normalize vector */
  length = sqrt( x*x + y*y + z*z );
  /* if zero vector passed in, just return identity quaternion	*/
  if ( length < Q_EPSILON )
    {
    r[Q_X] = 0;
    r[Q_Y] = 0;
    r[Q_Z] = 0;
    r[Q_W] = 1;
    me->URPoly->GetLocalXform().AddRotate(r);
    return;
  }
  x /= length;
  y /= length;
  z /= length;
  cosA = cos(angle / 2.0);
  sinA = sin(angle / 2.0);

  r[Q_W] = cosA;
  r[Q_X] = sinA * x;
  r[Q_Y] = sinA * y;
  r[Q_Z] = sinA * z;
  me->URPoly->GetLocalXform().AddRotate(r);
} /*imported_obj::handle_rotate_y_change*/

/** PURPOSE: Handle a change in the rotate_z Tcl slider --
           Causes combined rotation around z-axis to be new_value degrees
NOTE: Basically takes a rotation function from the quat library and 
           modifies it for our purposes */
void imported_obj::handle_rotate_z_change (vrpn_int32 new_value, void *userdata) {
  q_type r;
  double length, cosA, sinA;
  double x, y, z, angle, desired_angle;
  imported_obj *me = (imported_obj *)userdata;

  x = 0;
  y = 0;
  z = 1;

  //we want to rotate only the change in angle values since last rotation,
  //not the full new angle value (eg if we rotated 60 degrees last time and
  //the slider now says to rotate 90 degrees, we only want to rotate 30 
  //degrees) so we subtract last_rotated from desired_angle, and convert
  //to radians
  desired_angle = new_value*M_PI/180;
  angle = desired_angle - me->z_last_rotated;
  me->z_last_rotated = desired_angle;

  #define  Q_EPSILON   (1e-10)
  /* normalize vector */
  length = sqrt( x*x + y*y + z*z );
  /* if zero vector passed in, just return identity quaternion	*/
  if ( length < Q_EPSILON )
  {
    r[Q_X] = 0;
    r[Q_Y] = 0;
    r[Q_Z] = 0;
    r[Q_W] = 1;
    me->URPoly->GetLocalXform().AddRotate(r);
    return;
  }

  x /= length;
  y /= length;
  z /= length;

  cosA = cos(angle / 2.0);
  sinA = sin(angle / 2.0);

  r[Q_W] = cosA;
  r[Q_X] = sinA * x;
  r[Q_Y] = sinA * y;
  r[Q_Z] = sinA * z;
  me->URPoly->GetLocalXform().AddRotate(r);
} /*imported_obj::handle_rotate_z_change*/

/** PURPOSE: Handle a change in the scale Tcl slider --
	     Causes the object to be drawn using the new scale factor */
void imported_obj::handle_scale_change(vrpn_float64 new_value, void *userdata) {

  imported_obj *me = (imported_obj *)userdata;

  me->URPoly->GetLocalXform().SetScale(new_value);
} /*imported_obj::handle_scale_change*/

/** PURPOSE: Handle a change of import mode, which is caused by pressing the
             Atoms or Bonds button, thereby changing the import mode */
void imported_obj::handle_import_mode_change (vrpn_int32 /*new_value*/, 
					      void *userdata) {
  char view_name[256];
  char widget_name[256];
  imported_obj *me = (imported_obj *)userdata;

  if (me->import_mode_value == 0)
    me->import_mode_value = 1;
  else
    me->import_mode_value = 0;
  if (me->import_mode_value == 0){ //we are now in bond mode
    delete me->sphere_radius;
    me->sphere_radius = NULL;
    delete me->sphere_tesselation;
    me->sphere_tesselation = NULL;
    delete me->sphere_colorR;
    me->sphere_colorR = NULL;
    delete me->sphere_colorG;
    me->sphere_colorG = NULL;
    delete me->sphere_colorB;
    me->sphere_colorB = NULL;

    //make new bond widgets    
    sprintf(view_name, "$viewobj(%d).viewframe",me->object_number);
    sprintf(widget_name, "bond_width%d",me->object_number);
    me->bond_width = new Tclvar_float(widget_name, me->bond_width_value);
    sprintf(widget_name, "bond_colorR%d",me->object_number);
    me->bond_colorR = new Tclvar_float(widget_name, me->bond_colorR_value);
    sprintf(widget_name, "bond_colorG%d",me->object_number);
    me->bond_colorG = new Tclvar_float(widget_name, me->bond_colorG_value);
    sprintf(widget_name, "bond_colorB%d",me->object_number);
    me->bond_colorB = new Tclvar_float(widget_name, me->bond_colorB_value);
    //set up bond callbacks
    me->bond_width->addCallback(handle_bond_width_change,me);
    me->bond_colorR->addCallback(handle_bond_colorR_change,me);
    me->bond_colorG->addCallback(handle_bond_colorG_change,me);
    me->bond_colorB->addCallback(handle_bond_colorB_change,me);
    //Set up MSI flag to reflect that we are now in bond mode
    ((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetImportMode(0);
  }  
  else { //we are now in sphere mode
    delete me->bond_width;
    me->bond_width = NULL;
    delete me->bond_colorR;
    me->bond_colorR = NULL;
    delete me->bond_colorG;
    me->bond_colorG = NULL;
    delete me->bond_colorB;
    me->bond_colorB = NULL;
    //make sphere widgets
    sprintf(view_name, "$viewobj(%d).viewframe",me->object_number);
    sprintf(widget_name, "sphere_radius%d",me->object_number);
    me->sphere_radius = new Tclvar_float(widget_name, me->sphere_radius_value);
    sprintf(widget_name, "sphere_tesselation%d",me->object_number);
    me->sphere_tesselation = new Tclvar_int(widget_name, me->sphere_tesselation_value);
    sprintf(widget_name, "sphere_colorR%d",me->object_number);
    me->sphere_colorR = new Tclvar_float(widget_name, me->sphere_colorR_value);
    sprintf(widget_name, "sphere_colorG%d",me->object_number);
    me->sphere_colorG = new Tclvar_float(widget_name, me->sphere_colorG_value);
    sprintf(widget_name, "sphere_colorB%d",me->object_number);
    me->sphere_colorB = new Tclvar_float(widget_name, me->sphere_colorB_value);
    me->sphere_radius->addCallback(handle_sphere_radius_change,me);
    me->sphere_tesselation->addCallback(handle_sphere_tesselation_change,me);
    //set up sphere callbacks
    me->sphere_colorR->addCallback(handle_sphere_colorR_change,me);
    me->sphere_colorG->addCallback(handle_sphere_colorG_change,me);
    me->sphere_colorB->addCallback(handle_sphere_colorB_change,me);
    //Set up MSI flag to reflect that we are now in sphere mode
    ((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetImportMode(1);
  }
  //Reload the display lists so that the object will now be drawn in the new mode
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_import_mode_change*/

/** PURPOSE: Handle a change of the visibility mode, which is caused
           by pressing the Hide or Show button*/
void imported_obj::handle_visibility_mode_change (vrpn_int32 /*new_value*/, 
						  void *userdata) {

  imported_obj *me = (imported_obj *)userdata;

  if (me->visibility_mode_value == 0)
    me->visibility_mode_value = 1;
  else
    me->visibility_mode_value = 0;
  //Change MSI flag to reflect whether or not to draw the object
  if (me->visibility_mode_value == 0){ //we are now in hide mode
    ((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetVisibilityMode(0);
  }  
  else { //we are now in sphere mode
    ((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetVisibilityMode(1);
  }
  //Rebuild display lists
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_visibility_mode_change*/

/** PURPOSE: Handle a change of the bond_width slider*/
void imported_obj::handle_bond_width_change (vrpn_float64 new_value, void *userdata) {
  imported_obj *me = (imported_obj *)userdata;
  //Change MSI bond_width value
  ((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetBondWidth(new_value);
  me->bond_width_value = new_value;
  //Rebuild the display lists
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_bond_width_change*/

/** PURPOSE: Handle a change of the bond's R color coordinate*/
void imported_obj::handle_bond_colorR_change (vrpn_float64 new_value, void *userdata) {
  imported_obj *me = (imported_obj *)userdata;
  //Change MSI bond R color coordinate
  //((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetBondColorR(new_value);
  me->bond_colorR_value = new_value;
  //Rebuild display lists
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_bond_colorR_change*/

/** PURPOSE: Handle a change of the bond's G color coordinate*/
void imported_obj::handle_bond_colorG_change (vrpn_float64 new_value, void *userdata) {
  imported_obj *me = (imported_obj *)userdata;
  //Change MSI bond G color coordinate
  //((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetBondColorG(new_value);
  me->bond_colorG_value = new_value;
  //Rebuild display list
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_bond_colorG_change*/

/** PURPOSE: Handle a change of the bond's B color coordinate*/
void imported_obj::handle_bond_colorB_change (vrpn_float64 new_value, void *userdata) {
  imported_obj *me = (imported_obj *)userdata;
  //Change MSI bond B color coordinate
  //((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetBondColorB(new_value);
  me->bond_colorB_value = new_value;
  //Rebuild display list
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_bond_colorB_change*/

/** PURPOSE: Handle a change of the sphere_radius widget*/
void imported_obj::handle_sphere_radius_change (vrpn_float64 new_value, void *userdata) {
  imported_obj *me = (imported_obj *)userdata;
  //Change MSI radius value
  ((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetSphereRadius(new_value);
  me->sphere_radius_value = new_value;
  //Rebuild display list
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_sphere_radius_change*/

/**PURPOSE: Handle a change of the sphere_tesselation widget*/
void imported_obj::handle_sphere_tesselation_change (vrpn_int32 new_value, void *userdata) {
  imported_obj *me = (imported_obj *)userdata;
  //Change MSI sphere depth value
  ((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetSphereDepth(new_value);
  me->sphere_tesselation_value = new_value;
  //Rebuild display list
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_sphere_tesselation_change*/

/** PURPOSE: Handle a change of the sphere_color's R coordinate*/
void imported_obj::handle_sphere_colorR_change (vrpn_float64 new_value, void *userdata) {
  imported_obj *me = (imported_obj *)userdata;
  //Change MSI sphere color R coordinate
  //((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetSphereColorR(new_value);
  me->sphere_colorR_value = new_value;
  //Rebuild display list
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_sphere_colorR_change*/

/** PURPOSE: Handle a change of the sphere_color's G coordinate*/
void imported_obj::handle_sphere_colorG_change (vrpn_float64 new_value, void *userdata) {
  imported_obj *me = (imported_obj *)userdata;
  //((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetSphereColorG(new_value);
  me->sphere_colorG_value = new_value;
  //Rebuild the display list
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_sphere_colorG_change*/

/** PURPOSE: Handle a change of the sphere_color's B coordinate*/
void imported_obj::handle_sphere_colorB_change (vrpn_float64 new_value, void *userdata) {
  imported_obj *me = (imported_obj *)userdata;
  //((MSIFileGenerator*)me->GetURPoly()->GetGenerator())->SetSphereColorB(new_value);
  me->sphere_colorB_value = new_value;
  //Rebuild display list
  me->GetURPoly()->ReloadGeometry();
} /*imported_obj::handle_sphere_colorG_change*/

/*void imported_obj::handle_delete_object_change (vrpn_int32 new_value, void *userdata) {
  //for later development?
}*/

/******************************************************************************
  Methods for imported_obj_list class
******************************************************************************/

imported_obj_list::imported_obj_list() { //default constructor
  imported_obj_list_head = NULL;
  imported_obj_list_tail = NULL;
} /*imported_obj_list::imported_obj_list*/

/** PURPOSE: Create an import_obj and add it to the display list.  Also, add
             its URPoly to the uberGraphics World, and set its basic
             uberGraphics values
    @param filename Filename of new object file whose geometry will be loaded
    @param World Pointer to uberGraphics World to which we are going to add
               the new object*/
void imported_obj_list::import_new_obj(char* filename,UTree *World) {
    GeometryGenerator *gen = FileGenerator::CreateFileGenerator(filename);
    //Empty linked list
    if(imported_obj_list_head==NULL){
        imported_obj_list_head = new imported_obj;
        imported_obj_list_tail = imported_obj_list_head;
        if (imported_obj_list_head==NULL){
            cerr << "Memory fault\n"; 
            //kill(getpid(),SIGINT);
            return;
        }
        else{
            if (imported_obj_list_head->URPoly!=NULL){
                imported_obj_list_head->GetURPoly()->GetLocalXform().SetScale(ugraphics_scale);
                imported_obj_list_head->GetURPoly()->GetLocalXform().SetTranslate(0,0,0);
                imported_obj_list_head->GetURPoly()->LoadGeometry(gen);
                World->TAddNode(imported_obj_list_head->GetURPoly(),filename);
            }
            else{ 
                cerr << "Memory fault\n"; 
                //kill(getpid(),SIGINT);
                delete imported_obj_list_head;
                imported_obj_list_head = NULL;
                /* This can't possibly be right - if we delete it, we can't
                reference it! I'm taking it out!!! */
                imported_obj_list_head->GetURPoly()->GetLocalXform().SetScale(ugraphics_scale);
                imported_obj_list_head->GetURPoly()->GetLocalXform().SetTranslate(0,0,0);
                imported_obj_list_head->GetURPoly()->LoadGeometry(gen);
                World->TAddNode(imported_obj_list_head->GetURPoly(),filename);
            }
        }
    }
    else{ //Nonempty linked list
        imported_obj_list_tail->next = new imported_obj;
        if (imported_obj_list_tail->next == NULL){
            cerr << "Memory fault\n"; 
            //kill(getpid(),SIGINT);
            return;
        }
        else{
            if (imported_obj_list_tail->next->URPoly!=NULL){
                imported_obj_list_tail = imported_obj_list_tail->next;
                imported_obj_list_tail->GetURPoly()->GetLocalXform().SetScale(ugraphics_scale);
                imported_obj_list_tail->GetURPoly()->GetLocalXform().SetTranslate(0,0,0);
                imported_obj_list_tail->GetURPoly()->LoadGeometry(gen);
                World->TAddNode(imported_obj_list_tail->GetURPoly(),filename);
            }
            else{
                cerr <<"Memory fault\n"; 
                //kill(getpid(), SIGINT);
                delete imported_obj_list_tail;
                imported_obj_list_tail = NULL;
            }
        }
    }
}/*imported_obj_list::import_new_obj*/
