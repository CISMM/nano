//////////////////////////////////////////////////////////////////
// Name: nmg_CloudTexturer::detail_point
// Scope: Private
// Description: Creates the detail information for the specified data
//              plane.  Uses lambert's cosine law, as for an ideally 
//              infinite cloud volume the lighting equation is theorized 
//              to approach lambert's law.  (Jupiter's surface is
//              nearly ideal). And this is MUCH faster than approximating
//              multiple-scattering effects of clouds
//////////////////////////////////////////////////////////////////
inline vrpn_float32 nmg_CloudTexturer::
detail_point(BCPlane *height_field, float x, float y, float intensity)
{
    VectorType n;
	q_vec_type norm;
    float u0;   //Incident angle
    Compute_Norm(x, y, n, height_field);
	norm[0] = n[0];
	norm[1] = n[1];
	norm[2] = n[2];
    u0 = q_vec_dot_product(norm, d_light);
	u0 = (u0 < 0) ? -u0 : u0;
    return intensity * u0;
}

//////////////////////////////////////////////////////////////////
// Name: nmg_CloudTexturer::uncertainty_point
// Scope: Private
// Description: Creates the uncertainty information for the specified
//              data plane
//////////////////////////////////////////////////////////////////
inline vrpn_float32 nmg_CloudTexturer::
uncertainty_point(BCPlane*, float, float, float)
{
    //Left alone, until we can get some real uncertainty data
    return 0;
}
