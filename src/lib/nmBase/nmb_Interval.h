#ifndef NMB_INTERVAL_H
#define NMB_INTERVAL_H

class nmb_Interval {

  public:

    nmb_Interval (void);  // constructs the (empty) interval [0, -1]
    nmb_Interval (int low, int high);
    nmb_Interval (const nmb_Interval &);
    ~nmb_Interval (void);

    nmb_Interval & operator = (const nmb_Interval &);
    nmb_Interval & operator += (const nmb_Interval &);
    nmb_Interval & operator -= (const nmb_Interval &);

    nmb_Interval operator + (const nmb_Interval &) const;
    nmb_Interval operator - (const nmb_Interval &) const;
      // does not handle cases that would split the base interval
      // into two intervals

    int operator == (const nmb_Interval &) const;
    int overlaps (const nmb_Interval &) const;
    int adjacent (const nmb_Interval &) const;
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
