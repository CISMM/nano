#ifndef NMG_HAPTIC_GRAPHICS_H
#define NMG_HAPTIC_GRAPHICS_H

#include <quat.h>
#include <globjects.h>

extern q_vec_type fp_origin_j, fp_normal_j;
extern int config_feelPlane_temp;
extern int config_feelGrid_temp;

extern int fg_xside;
extern int fg_yside;
extern q_vec_type * fg_vertices;

class nmg_haptic_graphics {


public:


    int show_feel_plane;
    int show_feel_grid;
    int showing_feel_plane;
    int showing_feel_grid;
    q_vec_type fp_origin;
    q_vec_type fp_normal;

    nmg_haptic_graphics();
    void get_feel_plane();
    void set_show_feel_plane(int on);
    void set_show_feel_grid(int on);

    int get_show_feel_plane();
    int get_show_feel_grid();
    void do_show_feel_plane(int on);
    void do_show_feel_grid(int on);
    void update_origin(q_vec_type *, q_vec_type * );
    void setFeelPlane(q_vec_type , q_vec_type );
    static void setFeelGrid(int xside, int yside,
                      q_vec_type * vertices);


private:

};
#endif //NMG_HAPTIC_GRAPHICS_H