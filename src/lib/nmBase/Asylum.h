/*===3rdtech===
  Copyright (c) 2007 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef _ASYLUM_H_
#define _ASYLUM_H_

// Placeholder for Asylum-specific code.
#include <stdlib.h>  // free()
#include <nmb_Types.h>
    // architecture-independent sizes


class AsylumFile
{
public:
   AsylumFile() :
      d_header( NULL),
      d_length(0)
   { }
   
   ~AsylumFile() {
      delete [] d_header;
   }
   
   int parseHeader(const char *header, vrpn_int32 length)
   {
      delete [] d_header;
      d_header = new char[length + 1];
      strncpy(d_header, header, length);
      d_header[length] = '\0';
      d_length = length;
      return 0;
   }

   const char * getHeader() { return d_header; }
   int getLength() { return d_length; }
   
   bool valid() { return d_header != NULL; }
   
private:
   char * d_header;
   vrpn_int32 d_length;

};

#endif
