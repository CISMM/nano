#ifndef NMB_INTERVAL_H
#define NMB_INTERVAL_H
/*===3rdtech===
  Copyright (c) 2002 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/

class nmb_Interval {

  public:

    nmb_Interval (void);  ///< constructs the (empty) interval [0, -1]
    nmb_Interval (int low, int high);
    nmb_Interval (const nmb_Interval &);
    ~nmb_Interval (void);

    nmb_Interval & operator = (const nmb_Interval &);
    nmb_Interval & operator += (const nmb_Interval &);
    nmb_Interval & operator -= (const nmb_Interval &);

    nmb_Interval operator + (const nmb_Interval &) const;
    nmb_Interval operator - (const nmb_Interval &) const;
      ///< does not handle cases that would split the base interval
      ///< into two intervals

    void clear(); ///< Resets to empty interval [0,-1]

    int operator == (const nmb_Interval &) const;
    int overlaps (const nmb_Interval &) const;
    int adjacent (const nmb_Interval &) const;
    int includes (int) const;
    int includes (const nmb_Interval &) const;
    int empty (void) const; 

    int low (void) const
      { return d_low; }
    int high (void) const
      { return d_high; }

  private:

    int d_low; 
    int d_high;
};

#endif  // NMB_INTERVAL_H
