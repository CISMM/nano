#ifndef NMG_SURFACEMASK
#define NMG_SURFACEMASK

class BCPlane;

class nmg_SurfaceMask
{
public:
    nmg_SurfaceMask();
    ~nmg_SurfaceMask();

    void init(int size);
    void setControlPlane(BCPlane *control);

    bool valid() {return d_derivationMode != NONE;}
    //Valid mask in this sense means one that isn't the 
    //default values

    void setDrawPartialMask(bool draw);
    bool quadMasked(int x, int y, int stride);

    int value(int x, int y) {return d_maskData[x + y * d_size];}
    void addValue(int x, int y, int value) {d_maskData[x + y * d_size] += value;}

    void deriveMask(float min_height, float max_height);

    void remove(nmg_SurfaceMask *other);
    ///This sets as masked in this mask, the unmasked portions
    ///of the other mask
    void add(nmg_SurfaceMask *other);
    ///This sets as unmasked in this mask, the unmasked portions
    ///of the other mask.  Should only be used to counter-act
    ///a previous remove

    //Debugging functions
    bool completeImage(nmg_SurfaceMask *other);
    int numberOfHoles();
private:
    enum DerivationMode {
        NONE, HEIGHT
    };
    int d_size;
    int *d_maskData;
    BCPlane *d_control;
    bool d_drawPartialMask;

    DerivationMode d_derivationMode;
    float d_minHeight, d_maxHeight;
};

#endif
