#ifndef NMG_SURFACEMASK
#define NMG_SURFACEMASK

#include <vrpn_Types.h>

class BCPlane;
class nmb_Dataset;

class nmg_SurfaceMask
{
public:
    nmg_SurfaceMask();
    ~nmg_SurfaceMask();

    void init(int width, int height);
    void setControlPlane(BCPlane *control);

    bool valid() {return d_derivationMode != NONE;}
    //Valid mask in this sense means one that isn't the 
    //default values

    void setDrawPartialMask(vrpn_bool draw);
    bool quadMasked(int x, int y, int stride);

    int value(int x, int y) {return d_maskData[x + y * d_width];}
    void addValue(int x, int y, int value) {d_maskData[x + y * d_width] += value;}

    void deriveMask(float min_height, float max_height);
    void deriveMask(float center_x, float center_y, float width,float height, 
                    float angle, nmb_Dataset *dataset);
    //If data is changing we may need to recompute the mask
    //plane if we are using certain derivation schemes
    void rederive(vrpn_bool force = VRPN_FALSE);

    void invertAdd(nmg_SurfaceMask *other);
    ///This sets as masked in this mask, the unmasked portions
    ///of the other mask
    void invertSubtract(nmg_SurfaceMask *other);
    ///This sets as unmasked in this mask, the unmasked portions
    ///of the other mask.  Should only be used to counter-act
    ///a previous invertAdd
    void subtract(nmg_SurfaceMask *other);
    void add(nmg_SurfaceMask *other);
    void clear();

    //Debugging functions
    bool completeImage(nmg_SurfaceMask *other);
    int numberOfHoles();
private:
    enum DerivationMode {
        NONE, HEIGHT, BOX
    };
    int d_height, d_width;
    int *d_maskData;
    BCPlane *d_control;
    vrpn_bool d_drawPartialMask;

    DerivationMode d_derivationMode;
    nmg_SurfaceMask *d_oldDerivation; 
    //For undoing automatic derivations after someone might have
    //peformed an invertAdd or invertSubtract.

    float d_minHeight, d_maxHeight;
};

#endif
