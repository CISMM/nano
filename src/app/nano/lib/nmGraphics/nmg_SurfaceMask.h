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

    void setDrawPartialMask(vrpn_bool draw);
    bool quadMasked(int x, int y, int stride);

    int value(int x, int y) {return d_maskData[x + y * d_width];}
    void addValue(int x, int y, int value) {d_maskData[x + y * d_width] += value;}

    int deriveMask(float min_height, float max_height);
    int deriveMask(float center_x, float center_y, float width,float height, 
                    float angle);
    //If data is changing we may need to recompute the mask
    //plane if we are using certain derivation schemes
    int rederive(nmb_Dataset *dataset);
    int needsDerivation();

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

    bool d_needsDerivation;

    float d_minHeight, d_maxHeight;

    float d_centerX, d_centerY;
    float d_boxWidth, d_boxHeight;
    float d_boxAngle;

    void deriveHeight();
    void deriveBox(nmb_Dataset *dataset);
};

#endif
