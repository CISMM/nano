#ifndef NMM_RELAXCOMP_H
#define NMM_RELAXCOMP_H

/*********
 * nmm_RelaxComp
 * Purpose: encapsulate relaxation compensation
 * 
 * When the AFM switches from oscillating mode to contact mode, there
 * is an offset between the height values measured by the two
 * modes. In either mode, height is measured by measuring the position
 * of the tip's support (it's point of attachment). When the tip is
 * oscillating, it moves both above and below its support point, and
 * the surface is touched somewhere below the tips support
 * point. However, in contact mode, the tip must bend a bit when it
 * touches the surface, so the tip is above its support
 * point. Relaxation compensation tries to measure the difference
 * between these two heights, and get rid of it so the user doesn't
 * see this offset.
 *
 * In fact, it gets more complicated. The Digital Instruments (DI)
 * AFM, which is no longer used, would gradually "relax" to the final
 * value of the offset after a switch, over a minute or more. Trial
 * and error discovered that this relaxation was basically exponential
 * decay, 1/t. So RelaxComp takes measurements over a short time, fits
 * those measurements to a 1/t decay, and then tells you the right
 * offset later when you ask. The Topometrix AFM seems to go to the
 * final offset right after a switch, so RelaxComp will just measure
 * the final offset and return it when asked.
 *
 * When do we use RelaxComp. We can do compensation when going from
 * oscillating->conact and contact->oscillating, but if we do both
 * numerical errors can cause strange drifts. We take the imaging
 * surface as the real one, and compensate for the offset when we are
 * modifying the surface.
 *************/

#include "nmb_Types.h"   //for vrpn_bool
#include <vrpn_Types.h>
#include <vrpn_Shared.h>

class nmm_Microscope_Remote;

class nmm_RelaxComp 
{
public:
    enum RelaxType {DECAY, CONST_OFFSET};
    nmm_RelaxComp(nmm_Microscope_Remote*);
    void enable(RelaxType relax_type);
    void disable();
    int	set_ignore_time_ms (int);
    int	set_separation_time_ms (int);
    int	start_fix (long sec, long usec, double def_height);
    float fix_height (long sec, long usec, float height);
    int	stop_fix(); 
    vrpn_bool is_ignoring_points();
    vrpn_bool is_enabled();

private:
    nmm_Microscope_Remote* microscope;
    
    enum RelaxState {DISABLED, IDLE, CALC_MODEL_POINT1, CALC_MODEL_POINT2,
		      DECAY_COMP, CONST_COMP};
    //Records the state of relaxation compensation.
    RelaxState	current_state;
    RelaxType  type_of_compensation;
    long	relax_sec0 ;
    long	relax_usec0 ;
    double	TIgnore , TSep , TAvg ;
    double	TMax;
    double	z_offset ;
    double	def_z;
    double	t1, z1;
    double	t2, z2;
    int	n_avg ;
    double decay_const;


};

extern int	RelaxComp;


#endif
