#ifndef NMB_PLANE_SELECTION_H
#define NMB_PLANE_SELECTION_H

// nmb_PlaneSelection

// This class contains pointers to the planes that are mapped.  It contains
// a lookup function to find the current values of all the planes from the
// inputGrid.  This class was created to be used as a parameter to functions
// that need to know the planes (such as the draw functions).  This allows
// planes to be added without having to change all of the code that calls
// the routines, since it just calls the lookup() member to fill it in.

// We could, instead, consider putting these pointers on nmb_Dataset
// (in place of the Tclvar_selectors there).

class BCPlane;  // from BCPlane.h
class nmb_Dataset;  // from nmb_Dataset.h

class nmb_PlaneSelection {

  public:

    nmb_PlaneSelection (void);
    ~nmb_PlaneSelection (void);

    void lookup (nmb_Dataset *);
      // Fills in the data members with current data from nmb_Datset

    BCPlane * height;
    BCPlane * color;
    BCPlane * contour;
    BCPlane * alpha;
};

#endif  // NMB_PLANE_SELECTION_H
