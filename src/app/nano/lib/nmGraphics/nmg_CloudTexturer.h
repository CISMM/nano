#ifndef _CLOUD_TEXTURER
#define _CLOUD_TEXTURER

#include <BCPlane.h>

#include <vrpn_Types.h> //For the vrpn types
#include <quat.h>  //For q_vec_type
#include <v.h>      //For VectorType
#include <normal.h> //For Compute_Norm

class nmg_State;

//Maybe, should create a compute norm using
//q_vec_type in some other place, because the
//code in this file, is based on the old(?) VectorType.
//It can be used however, because they are basically
//the same thing under the hood

class nmg_CloudTexturer 
{
public:
    nmg_CloudTexturer(int width, int height);
    
    vrpn_uint8* render_detail(nmg_State * state, 
                              vrpn_bool shadows = vrpn_false);
    vrpn_uint8* render_composite(nmg_State * state, 
                                 vrpn_bool shadows = vrpn_false);
    void set_light(q_vec_type lightdir, float intensity);
private:
    q_vec_type d_light;
    float d_intensity;
	int d_width, d_height;
	int d_screenWidth, d_screenHeight;
    
    vrpn_float32 *d_last_frame_detail;
    vrpn_float32 *d_last_frame_uncertainty;
    vrpn_int16 *d_shadow_map;
    
    void shadow_map(BCPlane *height_field);
    inline vrpn_float32 uncertainty_point(BCPlane *height_field, float x, float y, float intensity);
    inline vrpn_float32 detail_point(BCPlane *height_field, float x, float y, float intensity);
};

#include "nmg_CloudTexturer.I"

#endif
