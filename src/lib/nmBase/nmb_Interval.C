/*===3rdtech===
  Copyright (c) 2002 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmb_Interval.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

nmb_Interval::nmb_Interval (void) :
    d_low (0),
    d_high (-1) {

}

nmb_Interval::nmb_Interval (int low, int high) :
    d_low (low),
    d_high (high) {

}



nmb_Interval::nmb_Interval (const nmb_Interval & i) :
    d_low (i.d_low),
    d_high (i.d_high) {

}


nmb_Interval::~nmb_Interval (void) {

}

nmb_Interval & nmb_Interval::operator = (const nmb_Interval & i) {
  d_low = i.d_low;
  d_high = i.d_high;
  return *this;
}

nmb_Interval & nmb_Interval::operator += (const nmb_Interval & i) {
  if (i.empty()) return *this;
  if (this->empty()) {
      d_low = i.d_low;
      d_high = i.d_high;
  } else {
      d_low = min(d_low, i.d_low);
      d_high = max(d_high, i.d_high);
  }
  return *this;
}

nmb_Interval & nmb_Interval::operator -= (const nmb_Interval & i) {
  if (i.empty() || this->empty()) return *this;
  int newlow = d_low;
  int newhigh = d_high;

  if ((i.d_high >= d_low) &&
      (i.d_low <= d_low))
    newlow = i.d_high + 1;

  if ((i.d_low <= d_high) &&
      (i.d_high >= d_high))
    newhigh = i.d_low - 1;

  d_low = newlow;
  d_high = newhigh;

  return *this;
}


nmb_Interval nmb_Interval::operator + (const nmb_Interval & i) const {
  nmb_Interval v (*this);
  v += i;
  return v;
}

nmb_Interval nmb_Interval::operator - (const nmb_Interval & i) const {
  nmb_Interval v (*this);
  v -= i;
  return v;
}

void nmb_Interval::clear() {
    d_low = 0;
    d_high = -1;
}

int nmb_Interval::operator == (const nmb_Interval & i) const {
  return ((d_low == i.d_low) && (d_high == i.d_high));
}

int nmb_Interval::overlaps (const nmb_Interval & i) const {
  if ((i.d_high >= d_low) &&
      (i.d_high <= d_high) && (i.d_low <= i.d_high)) return 1;
  if ((i.d_low <= d_high) &&
      (i.d_low >= d_low) && (i.d_high >= i.d_low)) return 1;
  return 0;
}

int nmb_Interval::adjacent (const nmb_Interval & i) const {
  if (i.d_high == d_low - 1) return 1;
  if (i.d_low == d_high + 1) return 1;
  return 0;
}

int nmb_Interval::includes (int val) const {
    return ((val <= d_high) && (val >= d_low));
}

int nmb_Interval::includes (const nmb_Interval & i) const {
    return ((i.d_high <= d_high) && (i.d_low >= d_low));
}

int nmb_Interval::empty (void) const {
  return (d_low > d_high);
}




