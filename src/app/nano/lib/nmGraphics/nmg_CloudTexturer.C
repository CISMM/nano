#include "nmg_CloudTexturer.h"
#include <nmb_Globals.h>
#include <nmb_Dataset.h>
#include <BCGrid.h>
#include <nmb_Subgrid.h>
#include "graphics_globals.h"

////////////////////////////////////////////////////
// Name: nmg_CloudTexturer::constructor
// Scope: Public
// Description:
////////////////////////////////////////////////////
nmg_CloudTexturer::
nmg_CloudTexturer(int width, int height)
 : d_last_frame_detail((vrpn_float32*)NULL), d_screenWidth(width), d_screenHeight(height),
   d_width(0), d_height(0)
{
}

////////////////////////////////////////////////////
// Name: nmg_CloudTexturer::set_light
// Scope: Public
// Description:
////////////////////////////////////////////////////
void nmg_CloudTexturer::
set_light(q_vec_type lightdir, float intensity)
{
    d_light[0] = lightdir[0];
    d_light[1] = lightdir[1];
    d_light[2] = lightdir[2];
    d_intensity = intensity;
}

//////////////////////////////////////////////////////////////////
// Name: nmg_CloudTexturer::detail_map
// Scope: Public
// Description: Creates the detail information for the specified data
//              plane.  Uses lambert's cosine law, as for an ideally 
//              infinite cloud volume the lighting equation is theorized 
//              to approach lambert's law.  (Jupiter's surface is
//              nearly ideal). And this is MUCH faster than approximating
//              multiple-scattering effects of clouds
//
//        Note: Until the shadow map generation can be re-designed to allow
//              for it to be only partially recomputed, it doesn't make sense
//              to only partially compute the detail information since the
//              shadow map generation will so completely dominate in terms of
//              execution time.
//////////////////////////////////////////////////////////////////
vrpn_uint8* nmg_CloudTexturer::
render_detail(vrpn_bool shadows)
{
    BCPlane *height_field = dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());
    int minx, miny, maxx, maxy;
    int index, shadow_index;
    
    float wscale = (float)height_field->numX() / d_screenWidth;
    float hscale = (float)height_field->numY() / d_screenHeight;
    
    //Probably a good idea to go ahead and call this, even if we aren't
    //only recomputing locally
    
    //Figure out what portion of the image needs to be recomputed
    //dataset->range_of_change.GetBoundsAndClear(&minx, &maxx, &miny, &maxy);
    
    minx = 0; miny = 0;
    maxx = height_field->numX(); maxy = height_field->numY();
    index = 0;
    shadow_index = 0;
    
    if (d_width != (maxx-minx) || d_height != (maxy-miny)) {
        d_height = maxy - miny;
        d_width = maxx - minx;
        if (d_last_frame_detail != (vrpn_float32*)NULL) {
            delete [] d_last_frame_detail;
        }
        d_last_frame_detail = new vrpn_float32[(int)(d_width/wscale) * 
                                              (int)(d_height/hscale) * 3];
    }
    
    //For now recompute the entire shadow map.
    if (shadows) {
        shadow_map(height_field);
    }
    
    for(float y = miny; y < maxy; y+=hscale) {
        for(float x = minx; x < maxx; x+=wscale) {
            d_last_frame_detail[index] = detail_point(height_field,x,y,1);
            if (shadows) {
                d_last_frame_detail[index] *= d_shadow_map[shadow_index];
                shadow_index++;
            }
            //For now, this is just a black and white texture, so the color
            //is the same for all 3 points.  But for the sake of future expansion
            //and not having to change the interface, return an RGB triplet
            d_last_frame_detail[index+1] = d_last_frame_detail[index];
            d_last_frame_detail[index+2] = d_last_frame_detail[index];
            index += 3;
        }
    }

    vrpn_uint8 *new_frame = new vrpn_uint8[d_screenWidth * d_screenHeight * 3];
    index = 0;
    for(int i = 0; i < d_screenHeight; i++) {
        for(int j = 0; j < d_screenWidth; j++) {
            new_frame[index] = (vrpn_uint8) (255 * d_last_frame_detail[index]);
            new_frame[index+1] = new_frame[index];
            new_frame[index+2] = new_frame[index];
            index += 3;
        }
    }
    
    //For the moment, until the world is made a happier place just set this to
    //the entire size of the image
    g_minChangedX = 0;
    g_minChangedY = 0;
    g_maxChangedX = d_screenWidth - 1;
    g_maxChangedY = d_screenHeight - 1;

    return new_frame;
}

//////////////////////////////////////////////////////////////////
// Name: nmg_CloudTexturer::composite_map
// Scope: Public
// Description: Combines the shadow, detail and uncertainty information 
//              (if there is uncertainty information)
//////////////////////////////////////////////////////////////////
vrpn_uint8* nmg_CloudTexturer::
render_composite(vrpn_bool shadows)
{
    //Until we have uncertainty data, this is no better than calling
    //render_detail.
    return render_detail(shadows);
}

//////////////////////////////////////////////////////////////////
// Name: nmg_CloudTexturer::shadow_map
// Scope: Private
// Description: Creates the shadow map for the current data and
//              light position
// Problem:  At worst, this is an order n cubed operation.  Something
//           needs to be done to improve that, but the problem is that
//           there is no geometry to work with, and that causes problems.
//           I know that normally the data is triangulated, so is it
//           possible if I could get ahold of that, that I could do
//           something more efficient?  Something to look into.
//////////////////////////////////////////////////////////////////
void nmg_CloudTexturer::
shadow_map(BCPlane *height_field)
{
    int numx = height_field->numX();
    int numy = height_field->numY();
    int index = 0;
    
    
    //This is something of a question, how does the height of the light ray
    //from the point being check to the light source change?  Since we are
    //stepping through the grid from the starting point, trying to see if the
    //ray intersects anything, we need to know exactly how much to increment
    //for each step. Until a better idea occurs to me as to what would be the
    //correct way to do this, this will have to do.
    
    //Okay, so we know that there are real X and Y values, they vary somewhere
    //between 0 and 4000nm generally, but we have quantized that to some grid,
    //with integer values.  So I will do the same with Z.  Assuming the grid
    //is square, this should be the max z - min z divided by the number of X
    //(or Y) values.  Mutliply this value by the Z value of the light, and
    //that is the Z increment
    
    float dz = (height_field->maxValue() - height_field->minValue()) / 
        height_field->numX();
    dz *= d_light[2];
    
    //The shadow map is getting filled in with 1 for non-shadow and 0.3 for
    //shadow, which is kinda anti-intuitive, but makes it easier to use later.
    //0.3 was chosen arbitrarily because in reality multiple scattering effects
    //would make it nearly improbable that any point on a cloud would have no
    //light.
    for(int i = 0; i < numx; i++) {
        for(int j = 0; j < numy; j++) {
            float x = i + d_light[0];
            float y = j + d_light[1];
            float current_height = height_field->interpolatedValue(x,y) + dz;
            
            d_shadow_map[index] = 1;
            while((x < numx && x > 0) && (y < numy && y > 0)) {
                if (height_field->interpolatedValue(x,y) > current_height) {
                    d_shadow_map[index] = 0.3;
                    break;
                }
                x += d_light[0];
                y += d_light[1];
                current_height += dz;
            }
            index++;
        }
    }
}


