#include "nmg_haptic_graphics.h"
#include <globjects.h>

#include <nmb_Types.h>
#include <nmb_Globals.h>
#include <nmb_String.h>
#include <nmb_Line.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include "..\..\interaction.h"


q_vec_type * fg_vertices;
int fg_xside = 0;
int fg_yside = 0;

nmg_haptic_graphics::nmg_haptic_graphics() {

    show_feel_plane = 0;
    show_feel_grid = 0;

}


void nmg_haptic_graphics::get_feel_plane() { 
    return ;
}

void nmg_haptic_graphics::set_show_feel_plane(int on) {

    show_feel_plane = on;
}


void nmg_haptic_graphics::set_show_feel_grid(int on) {

    show_feel_grid = on;
}

int nmg_haptic_graphics::get_show_feel_grid() {
    return show_feel_grid;
}

int nmg_haptic_graphics::get_show_feel_plane() {
    return show_feel_plane;
}

void nmg_haptic_graphics::do_show_feel_plane(int on) {
    enableFeelPlane(this,on);
    config_feelPlane_temp = on;
    showing_feel_plane = on;  
}

void nmg_haptic_graphics::do_show_feel_grid(int on) {
    enableFeelGrid(on);
    config_feelGrid_temp = on;
    showing_feel_grid = on;

}


void nmg_haptic_graphics::update_origin(q_vec_type * origin, q_vec_type * normal ) {
  q_vec_copy(*origin, fp_origin);
  q_vec_copy(*normal, fp_normal);

}

void nmg_haptic_graphics::setFeelPlane(q_vec_type origin, q_vec_type normal ) {
  q_vec_copy(fp_origin, origin);
  q_vec_copy(fp_normal, normal);

  q_vec_copy(fp_origin_j, origin);
  q_vec_copy(fp_normal_j, normal);

}

void nmg_haptic_graphics::setFeelGrid(int xside, int yside, 
                                      q_vec_type * vertices){

    fg_xside = xside;
    fg_yside = yside;
    fg_vertices = vertices;
    

}
