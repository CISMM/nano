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

    int init(int width, int height);
    void setControlPlane(BCPlane *control);

    void setDrawPartialMask(vrpn_bool draw);
    bool quadMasked(int x, int y, int stride);

    int value(int x, int y);

    /// Derive a mask for the Height method
    int deriveMask(float min_height, float max_height);
    /// Derive a mask for the Box method
    int deriveMask(float center_x, float center_y, float width,float height, 
                    float angle);
    /// Derive a mask for the NullData method
    int deriveMask();
    /// Derive a mask which is the inversion of other masks. 
    int deriveMask(nmg_SurfaceMask** masks, int numInvertMasks);

    ///If data is changing we may need to recompute the mask
    ///plane if we are using certain derivation schemes
    int update(nmb_Dataset *dataset, int stride, 
               int low_row, int high_row, int strips_in_x, 
               bool clear_needs_update = true);
    bool needsUpdate();

    ///Retrieve min and max valid parameters, from null data mask. 
    int getValidRange(short * minValid, short * maxValid) {
        *minValid = d_minValid; *maxValid = d_maxValid; return 0;
    }
    /// Query whether we can call getValidRange
    bool nullDataType() { return (d_derivationMode==NULLDATA); }

    ///Debugging function
    bool completeImage(nmg_SurfaceMask *other);
    ///Debugging function
    int numberOfHoles();
private:
    // XXX These are now private, because we no longer use
    // the d_maskData array for all masks, so they don't make
    // sense for all masks. 
    void setValue(int x, int y, int value) {d_maskData[x + y * d_width] = value;}
    void invertAdd(nmg_SurfaceMask *other);
    ///<This sets as masked in this mask, the unmasked portions
    ///<of the other mask
    void invertSubtract(nmg_SurfaceMask *other);
    ///<This sets as unmasked in this mask, the unmasked portions
    ///<of the other mask,  Should only be used to counter-act
    ///<a previous invertAdd
    void subtract(nmg_SurfaceMask *other);
    void add(nmg_SurfaceMask *other);
    void clearOn(); ///< Clear mask to all 1 (draw nothing)
    void clearOff(); ///< Clear mask to all 0 (draw whole surface)

    enum DerivationMode {
        NONE, HEIGHT, BOX, NULLDATA, INVERTMASKS
    };
    int d_height, d_width;
    char *d_maskData;
    BCPlane *d_control;
    nmb_Dataset * d_dataset;
    vrpn_bool d_drawPartialMask;

    DerivationMode d_derivationMode;
    nmg_SurfaceMask *d_oldDerivation; 
    //For undoing automatic derivations after someone might have
    //peformed an invertAdd or invertSubtract.

    bool d_needsUpdate;

    float d_minHeight, d_maxHeight;

    float d_centerX, d_centerY;
    float d_boxWidth, d_boxHeight;
    float d_boxAngle;

    short d_minValid, d_maxValid;

    nmg_SurfaceMask * d_invertMaskList[20];
    int d_numInvertMasks;

    int deriveHeight(int low_row, int high_row, int strips_in_x);
    int deriveBox(nmb_Dataset *dataset, int low_row, int high_row, int strips_in_x);
    int deriveNullData(int stride, int low_row, int high_row, int strips_in_x);
    int deriveInvertMasks(nmb_Dataset *dataset, int stride, 
                          int low_row, int high_row, int strips_in_x);
};

#endif
