
#ifndef dbimage_h
#define dbimage_h

// #include <string.h>

class DBImage {
public:
    
    DBImage(char* filename);
    ~DBImage();

    double* const double_array() { return _da; }

    // Here the intensities are scaled to [0, 255].
    unsigned char* const byte_array() { return _ba; }

    unsigned   xdim() const { return _xdim; }
    unsigned   ydim() const { return _ydim; }

    double pixval(int x, int y) { return _da[y * _xdim + x]; }

    double  min_pixval() const { return _min_pixval; }
    double  max_pixval() const { return _max_pixval; }
    double  x_nm_perpixel() const { return _x_nm_perpixel; }
    double  y_nm_perpixel() const { return _y_nm_perpixel; }

private:
    double* _da;
    unsigned char* _ba;
    int _xdim;
    int _ydim;
    double _min_pixval;    // Height or intensity.
    double _max_pixval;
    double _x_nm_perpixel; // x and y scales in nanometers per pixel
    double _y_nm_perpixel;

};


#endif // dbimage_h
