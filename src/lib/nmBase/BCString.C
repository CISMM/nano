#include "BCString.h"

/**
BCString() --> constructor
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString::BCString()
{
    // cout << "ENTERING BCString()" << endl;

    _length = 0;
    _bytesAllocated = 1;
    _str = new char[1];

    // cout << "ENTERING BCString()" << endl;
}


/**
BCString() --> constructor
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString::BCString(int i)
{
    // cout << "ENTERING BCString(" << i << ")" << endl;

    char* temp = new char[10];

    sprintf(temp, "%d", i);
    _length = strlen(temp);
    _bytesAllocated = _length + 1;
    _str = new char[_bytesAllocated];
    strcpy(_str, temp);

    // cout << "EXITING BCString(" << i << ")" << endl;
}


/**
BCString() --> constructor
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString::BCString(const char *s)
{   
    // cout << "ENTERING BCString(" << s << ")" << endl;

    _length = strlen(s);
    _bytesAllocated = _length + 1;
    _str = new char[_bytesAllocated];
    strcpy(_str, s);

    // cout << "EXITING BCString(" << s << ")" << endl;
}


/**
BCString() --> constructor
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString::BCString(char c)
{   
    _length = 1;
    _bytesAllocated = 2;
    _str = new char[_bytesAllocated];
    _str[0] = c;
    _str[1] = '\0';
}


/**
BCString() --> constructor
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString::BCString(const BCString& s)
{   
    // cout << "ENTERING BCString(const BCString& s)" << endl;

    int i;

    _length = s._length;
    _bytesAllocated = s._bytesAllocated;
    _str = new char[_bytesAllocated];
    for (i = 0; i < _length; i++)
    {
	_str[i] = s._str[i];
    }
    _str[i] = '\0';

    // cout << "EXITING BCString(const BCString& s)" << endl;
}


/**
operator =
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString&
BCString::operator = (const char* s)
{ 
    // cout << "ENTERING = (const char* s)" << endl;

    _length = strlen(s);
    _bytesAllocated = _length + 1;
    delete [] _str;
    _str = new char[_bytesAllocated];
    strcpy(_str, s);

    // cout << "EXITING = (const char* s)" << endl;
    return *this;
}


/**
operator =
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString&
BCString::operator = (char c)
{ 
    _length = 1;
    _bytesAllocated = 2;
    delete [] _str;
    _str = new char[_bytesAllocated];
    _str[0] = c;
    _str[1] = '\0';
    return *this;
}


/**
operator =
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString&
BCString::operator = (const BCString& s)
{  
    // cout << "ENTERING = (const BCString& s)" << endl;

    if (this != &s)
    {
	_length = s._length;
	_bytesAllocated = s._bytesAllocated;
	delete [] _str;
	_str = new char[_bytesAllocated];
	strcpy(_str, s._str);
    }

    // cout << "EXITING = (const BCString& s)" << endl;
    return *this;
}


/**
operator +=
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString&
BCString::operator += (const char* s)
{ 
    _length += strlen(s);
    _bytesAllocated = _length + 1;
    char *temp = new char[_bytesAllocated];
    strcpy(temp, _str);
    strcat(temp, s);
    delete [] _str;
    _str = new char[_bytesAllocated];
    strcpy(_str, temp);
    return *this;
}


/**
operator +=
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString&
BCString::operator += (char c)
{ 
    _length += 1;
    _bytesAllocated += 1;
    char *temp = new char[_bytesAllocated];
    strcpy(temp, _str);
    temp[_length] = c;
    delete [] _str;
    _str = new char[_bytesAllocated];
    strcpy(_str, temp);
    return *this;
}


/**
operator +=
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString&
BCString::operator += (const BCString& s)
{  
    _length += s._length;
    _bytesAllocated += s._length;
    char *temp = new char[_bytesAllocated];
    strcpy(temp, _str);
    strcat(temp, s._str);
    delete [] _str;
    _str = new char[_bytesAllocated];
    strcpy(_str, temp);
    return *this;	
}


/**
operator +
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
BCString
operator + (const BCString& s1, const BCString& s2)
{
    // cout << "ENTERING + (const BCString& s1, const BCString& s2)" << endl;
    char *temp = new char[s1._length + s2._length + 1];

    strcpy(temp, s1._str);
    strcat(temp, s2._str);
    BCString s3(temp);

    // cout << "EXITING + (const BCString& s1, const BCString& s2)" << endl;
    return s3;
}


/**
operator[]
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
char&
BCString::operator[](int index)
{
    if (index > _length) 
	return _str[_length];
    else
	return _str[index];
}
    

/**
operator <<
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
ostream&
operator << (ostream& os, const BCString& s)
{
    // cout << "ENTERING <<" << endl;
    os << s.Characters();
    // cout << "EXITING <<" << endl;
    return os;
}


/**
operator >>
    
        @author Kimberly Passarella Jones
 @date 2-5-95 by Kimberly Passarella Jones
*/
istream&
operator >> (istream& is, BCString& s)
{ 
    const int bufferSize = 256;
    char buffer[bufferSize];

    if (is.get(buffer, bufferSize))
	s = BCString(buffer);

    return is;
}
