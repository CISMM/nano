/******************************************************************************\
@BCString 
--------------------------------------------------------------------------------
inheritance: BCString
description: 
\******************************************************************************/

#ifndef STRING_H
#define STRING_H

#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

class BCString 
{
  public:

    BCString();
    BCString(int);
    BCString(const char*);
    BCString(char);
    BCString(const BCString&);

    ~BCString();

    int Length() const;
    int BytesAllocated() const;
    int Empty() const;

    int operator < (const BCString&) const;
    int operator > (const BCString&) const;
    int operator == (const BCString&) const;
    int operator <= (const BCString&) const;
    int operator >= (const BCString&) const;
    int  operator != (const BCString&) const;

    BCString& operator = (const char*);
    BCString& operator = (char);
    BCString& operator = (const BCString&);

    BCString& operator += (const char*);
    BCString& operator += (char);
    BCString& operator += (const BCString&);

    friend BCString operator + (const BCString&, const BCString&);

    char& operator[](int);

    const char* Characters() const;

    operator const char*() const;

    friend ostream& operator << (ostream&, const BCString&);
    friend istream& operator >> (istream&, BCString&); 

  protected:

    int _length;
    int _bytesAllocated;
    char* _str; 

  private:

};
 

//----------------------------------------------------------------------
// inline functions
//----------------------------------------------------------------------

inline BCString::~BCString()
{
    delete [] _str;
}


inline int 
BCString::Length() const
{
    return _length;
}


inline int 
BCString::BytesAllocated() const
{
    return _bytesAllocated;
}


inline int 
BCString::Empty() const
{
    return _length == 0;
}


inline int BCString::operator < (const BCString &s) const
{
    return (strcmp(_str, s._str) < 0);
}


inline int 
BCString::operator > (const BCString &s) const
{
    return (strcmp(_str, s._str) > 0);
}


inline int 
BCString::operator == (const BCString &s) const
{
    return (strcmp(_str, s._str) == 0);
}


inline int
BCString::operator <= (const BCString &s) const
{
    return (strcmp(_str, s._str) <= 0);
}


inline int 
BCString::operator >= (const BCString &s) const
{
    return (strcmp(_str, s._str) >= 0);
}


inline int 
BCString::operator != (const BCString &s) const
{
    return (strcmp(_str, s._str) != 0);
}

//returns a const char *
inline const char* 
BCString::Characters() const 
{ 
    return &(_str[0]); 
}


//returns a const char *
inline BCString::operator const char*() const
{
    return &(_str[0]);
}


#endif // STRING_H







