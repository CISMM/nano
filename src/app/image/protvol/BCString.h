/******************************************************************************\
@BCString 
--------------------------------------------------------------------------------
inheritance: BCString
description: 
\******************************************************************************/

#ifndef STRING_H
#define STRING_H

#include <iostream>
using namespace std;
#include <stdio.h>
#include <string.h>


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

/******************************************************************************\
@~BCString() --> destructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline BCString::~BCString()
{
    delete [] _str;
}


/******************************************************************************\
@Length
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline int 
BCString::Length() const
{
    return _length;
}


/******************************************************************************\
@BytesAllocated
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline int 
BCString::BytesAllocated() const
{
    return _bytesAllocated;
}


/******************************************************************************\
@Empty
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline int 
BCString::Empty() const
{
    return _length == 0;
}


/******************************************************************************\
@operator <
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline int BCString::operator < (const BCString &s) const
{
    return (strcmp(_str, s._str) < 0);
}


/******************************************************************************\
@operator >
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline int 
BCString::operator > (const BCString &s) const
{
    return (strcmp(_str, s._str) > 0);
}


/******************************************************************************\
@operator ==
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline int 
BCString::operator == (const BCString &s) const
{
    return (strcmp(_str, s._str) == 0);
}


/******************************************************************************\
@operator <=
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline int
BCString::operator <= (const BCString &s) const
{
    return (strcmp(_str, s._str) <= 0);
}


/******************************************************************************\
@operator >=
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline int 
BCString::operator >= (const BCString &s) const
{
    return (strcmp(_str, s._str) >= 0);
}


/******************************************************************************\
@operator !=
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline int 
BCString::operator != (const BCString &s) const
{
    return (strcmp(_str, s._str) != 0);
}


/******************************************************************************\
@Characters
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline const char* 
BCString::Characters() const 
{ 
    return &(_str[0]); 
}


/******************************************************************************\
@operator const char*
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
inline BCString::operator const char*() const
{
    return &(_str[0]);
}


#endif // STRING_H







