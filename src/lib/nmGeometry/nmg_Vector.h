#ifndef NMG_VECTOR_H
#define NMG_VECTOR_H

#include <stdio.h>
#include <math.h>

class nmg_Vector_3f;

class nmg_Vector_3d {
 public:
   double x, y, z;

    nmg_Vector_3d (void)
      { x = 0.0; y = 0.0; z = 0.0;};
    nmg_Vector_3d (const double Xval, const double Yval, const double Zval)
      { x=Xval; y=Yval; z=Zval; };
    nmg_Vector_3d (const nmg_Vector_3d& v)
      { x=v.x; y=v.y; z=v.z; };
    inline nmg_Vector_3d (const nmg_Vector_3f& v);
    void Set (const double Xval, const double Yval, const double Zval)
      { x=Xval; y=Yval; z=Zval; }

    nmg_Vector_3d& operator = (const nmg_Vector_3d& A)        // ASSIGNMENT (=)
      { x=A.x; y=A.y; z=A.z;
        return(*this);  };
    inline nmg_Vector_3d& operator = (const nmg_Vector_3f& A);
    nmg_Vector_3d operator + (const nmg_Vector_3d& A) const   // ADDITION (+)
      { nmg_Vector_3d Sum(x+A.x, y+A.y, z+A.z);
        return(Sum); };
    inline nmg_Vector_3d operator + (const nmg_Vector_3f& A) const;// ADDITION (+)
    nmg_Vector_3d operator - (const nmg_Vector_3d& A) const   // SUBTRACTION (-)
      { nmg_Vector_3d Diff(x-A.x, y-A.y, z-A.z);
        return(Diff); };
    double operator * (const nmg_Vector_3d& A) const   // DOT-PRODUCT (*)
      { double DotProd = x*A.x+y*A.y+z*A.z;
        return(DotProd); };
    nmg_Vector_3d operator / (const nmg_Vector_3d& A) const   // CROSS-PRODUCT (/)
      { nmg_Vector_3d CrossProd(y*A.z-z*A.y, z*A.x-x*A.z, x*A.y-y*A.x);
        return(CrossProd); };
    nmg_Vector_3d operator * (const double s) const    // MULTIPLY BY SCALAR (*)
      { nmg_Vector_3d Scaled(x*s, y*s, z*s);
        return(Scaled); };
    nmg_Vector_3d operator / (const double s) const    // DIVIDE BY SCALAR (/)
      { nmg_Vector_3d Scaled(x/s, y/s, z/s);
        return(Scaled); };

    void operator += (const nmg_Vector_3d &A) // ACCUMULATED VECTOR ADDITION (+=)
      { x+=A.x; y+=A.y; z+=A.z; };
    inline void operator += (const nmg_Vector_3f &A);
    // ACCUMULATED VECTOR SUBTRACTION (-=)
    void operator -= (const nmg_Vector_3d &A)
      { x-=A.x; y-=A.y; z-=A.z; };
    inline void operator -= (const nmg_Vector_3f &A);
    void operator *= (const double s)     // ACCUMULATED SCALAR MULT (*=)
      { x*=s; y*=s; z*=s; };
    void operator /= (const double s)     // ACCUMULATED SCALAR DIV (/=)
      { x/=s; y/=s; z/=s; };
    nmg_Vector_3d operator - (void) const        // NEGATION (-)
      { nmg_Vector_3d Negated(-x, -y, -z);
        return(Negated); };
    int operator == (const nmg_Vector_3d &A)
       {return (x==A.x && y==A.y && z==A.z);}; // EQUIVALENCE (==)
    int operator != (const nmg_Vector_3d &A)
       {return (x!=A.x || y!=A.y || z!=A.z);};

    double operator [] (const int i) const // ALLOWS VECTOR ACCESS AS AN ARRAY.
      { return( (i==0)?x:((i==1)?y:z) ); };
    double & operator [] (const int i)
      { return( (i==0)?x:((i==1)?y:z) ); };

    double Length (void) const                 // LENGTH OF VECTOR
      { return ((double)sqrt(x*x+y*y+z*z)); };
    double LengthSqr (void) const              // LENGTH OF VECTOR (SQUARED)
      { return (x*x+y*y+z*z); };
    void Normalize (void)                      // NORMALIZE VECTOR
      { double L = Length();                   // CALCULATE LENGTH
        x/=L; y/=L; z/=L; };                   // DIVIDE COMPONENTS BY LENGTH

    void Print()
      { printf("(%.3f, %.3f, %.3f)\n",x, y, z); }

    void vmin (const nmg_Vector_3d & a)
      { if (x>a.x) x=a.x; if (y>a.y) y=a.y; if (z>a.z) z=a.z; }
    void vmax (const nmg_Vector_3d & a)
      { if (x<a.x) x=a.x; if (y<a.y) y=a.y; if (z<a.z) z=a.z; }
    nmg_Vector_3d multComponents (const nmg_Vector_3d& A) const
      { return nmg_Vector_3d (x*A.x, y*A.y, z*A.z); }

};

class nmg_Vector_3f {
 public:
   float x, y, z;

    nmg_Vector_3f (void)
      { x = 0.0; y = 0.0; z = 0.0;};
    nmg_Vector_3f (const float Xval, const float Yval, const float Zval)
      { x=Xval; y=Yval; z=Zval; };
    nmg_Vector_3f (const nmg_Vector_3f& v)
      { x=v.x; y=v.y; z=v.z; };
    nmg_Vector_3f (const nmg_Vector_3d& v)
      { x=(float)v.x; y=(float)v.y; z=(float)v.z; };
    void Set (const float Xval, const float Yval, const float Zval)
      { x=Xval; y=Yval; z=Zval; }

    nmg_Vector_3f& operator = (const nmg_Vector_3f& A)        // ASSIGNMENT (=)
      { x=A.x; y=A.y; z=A.z;
        return(*this);  };
    nmg_Vector_3f& operator = (const nmg_Vector_3d& A)        // ASSIGNMENT (=)
      { x=(float)A.x; y=(float)A.y; z=(float)A.z;
        return(*this);  };
    nmg_Vector_3f operator + (const nmg_Vector_3f& A) const   // ADDITION (+)
      { nmg_Vector_3f Sum(x+A.x, y+A.y, z+A.z);
        return(Sum); };
    nmg_Vector_3f operator - (const nmg_Vector_3f& A) const   // SUBTRACTION (-)
      { nmg_Vector_3f Diff(x-A.x, y-A.y, z-A.z);
        return(Diff); };
    double operator * (const nmg_Vector_3f& A) const   // DOT-PRODUCT (*)
      { double DotProd = x*A.x+y*A.y+z*A.z;
        return(DotProd); };
    nmg_Vector_3f operator / (const nmg_Vector_3f& A) const   // CROSS-PRODUCT (/)
      { nmg_Vector_3f CrossProd(y*A.z-z*A.y, z*A.x-x*A.z, x*A.y-y*A.x);
        return(CrossProd); };
    nmg_Vector_3f operator * (const double s) const    // MULTIPLY BY SCALAR (*)
      { nmg_Vector_3f Scaled((float)(x*s), (float)(y*s), (float)(z*s));
        return(Scaled); };
    nmg_Vector_3f operator / (const double s) const    // DIVIDE BY SCALAR (/)
      { nmg_Vector_3f Scaled((float)(x/s), (float)(y/s), (float)(z/s));
        return(Scaled); };

    void operator += (const nmg_Vector_3f &A) // ACCUMULATED VECTOR ADDITION (+=)
      { x+=A.x; y+=A.y; z+=A.z; };
    // ACCUMULATED VECTOR SUBTRACTION (+=)
    void operator -= (const nmg_Vector_3f &A)
      { x-=A.x; y-=A.y; z-=A.z; };
    void operator *= (const double s)     // ACCUMULATED SCALAR MULT (*=)
      { x*=(float)s; y*=(float)s; z*=(float)s; };
    void operator /= (const double s)     // ACCUMULATED SCALAR DIV (/=)
      { x/=(float)s; y/=(float)s; z/=(float)s; };
    nmg_Vector_3f operator - (void) const        // NEGATION (-)
      { nmg_Vector_3f Negated(-x, -y, -z);
        return(Negated); };
    int operator == (const nmg_Vector_3f &A)
       {return (x==A.x && y==A.y && z==A.z);}; // EQUIVALENCE (==)
    int operator != (const nmg_Vector_3f &A)
       {return (x!=A.x || y!=A.y || z!=A.z);};

    float operator [] (const int i) const // ALLOWS VECTOR ACCESS AS AN ARRAY.
      { return( (i==0)?x:((i==1)?y:z) ); };
    float & operator [] (const int i)
      { return( (i==0)?x:((i==1)?y:z) ); };

    double Length (void) const                 // LENGTH OF VECTOR
      { return ((double)sqrt(x*x+y*y+z*z)); };
    double LengthSqr (void) const              // LENGTH OF VECTOR (SQUARED)
      { return (x*x+y*y+z*z); };
    void Normalize (void)                      // NORMALIZE VECTOR
      { double L = Length();                   // CALCULATE LENGTH
        x/=(float)L; y/=(float)L; z/=(float)L; };                   // DIVIDE COMPONENTS BY LENGTH

    void Print()
      { printf("(%.3f, %.3f, %.3f)\n",x, y, z); }

    void vmin (const nmg_Vector_3f & a)
      { if (x>a.x) x=a.x; if (y>a.y) y=a.y; if (z>a.z) z=a.z; }
    void vmax (const nmg_Vector_3f & a)
      { if (x<a.x) x=a.x; if (y<a.y) y=a.y; if (z<a.z) z=a.z; }
    nmg_Vector_3f multComponents (const nmg_Vector_3f& A) const
      { return nmg_Vector_3f (x*A.x, y*A.y, z*A.z); }

};

// inline
nmg_Vector_3d::nmg_Vector_3d (const nmg_Vector_3f& v)
{
  x = v.x; y = v.y; z = v.z;
}

// inline
nmg_Vector_3d& nmg_Vector_3d::operator = (const nmg_Vector_3f& A)
{
  x=A.x; y=A.y; z=A.z;
  return(*this);
}

// inline
nmg_Vector_3d nmg_Vector_3d::operator + (const nmg_Vector_3f& A) const
{ 
  nmg_Vector_3d Sum(x+A.x, y+A.y, z+A.z);
  return(Sum); 
}

// inline
void nmg_Vector_3d::operator += (const nmg_Vector_3f &A) 
{
  x+=A.x; y+=A.y; z+=A.z; 
}

// inline
void nmg_Vector_3d::operator -= (const nmg_Vector_3f &A)
{
  x-=A.x; y-=A.y; z-=A.z;
}

#endif
