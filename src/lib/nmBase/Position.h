/* The nanoManipulator and its source code have been released under the
 * Boost software license when nanoManipulator, Inc. ceased operations on
 * January 1, 2014.  At this point, the message below from 3rdTech (who
 * sublicensed from nanoManipulator, Inc.) was superceded.
 * Since that time, the code can be used according to the following
 * license.  Support for this system is now through the NIH/NIBIB
 * National Research Resource at cismm.org.

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef POSITION_H
#define POSITION_H

#include <math.h>  // for sqrt()
#include <stdio.h>
//#include "string"

class Position;
class Position_list;

/**
 * class Position
 *    Stores an x-y result and the time the result was taken.
 *    also has next and prev pointers for use in 
 *    Position_list - a linked list of Positions.
 */
class Position
{
    friend class Position_list;
private:
    Position * _next;
    Position * _prev;

protected:
    double _x;
    double _y;
    double _z;
    
    long _sec;
    long _usec;

    int _icon_id;   ///<the id of an icon to represent this
			    ///< position in the world.

    inline Position *next() { return _next; }
    inline Position *prev() { return _prev; }
    inline void next(Position * n) { _next = n; }
    inline void prev(Position * p) { _prev = p; }
    
public:
    Position (const double x, const double y) :
      _next (NULL), _prev (NULL),
      _x (x), _y (y), _z (0),
      _sec (0), _usec (0)
    { }

    Position (const double x, const double y, const double z) :
      _next (NULL), _prev (NULL),
      _x (x), _y (y), _z (z),
      _sec (0), _usec (0)
    { }

    Position (const double x, const double y, const long sec, const long usec) :
      _next (NULL), _prev (NULL),
      _x (x), _y (y), 
      _sec (sec), _usec (usec)
    { }

    Position (const double x, const double y, const double z,
	      const long sec, const long usec) :
      _next (NULL), _prev (NULL),
      _x (x), _y (y), _z (z),
      _sec (sec), _usec (usec)
    { }

    /// copy constructor. Default probably would have been ok.
    Position (const Position& p) :
      _next (p._next), _prev (p._prev),
      _x (p._x), _y (p._y), _z (p._z),
      _sec (p._sec), _usec (p._usec)
    { }

    inline void setTime(long sec, long usec) { _sec = sec; _usec = usec; }
    inline void set(double x, double y) { _x = x; _y = y; }
    inline void set(double x, double y, double z) 
      { _x = x; _y = y; _z = z; }
    inline double x() const { return _x; }
    inline double y() const { return _y; }
    inline double z() const { return _z; }
    inline void setIconID(int id) { _icon_id = id; }
    inline int iconID() { return _icon_id; }

    inline double dist2d(const Position * p) 
    { return(sqrt((_x - p->x())*(_x - p->x()) + (_y - p->y())*(_y - p->y())));}

    inline double dist3d(const Position * p) 
    { return(sqrt((_x - p->x())*(_x - p->x()) + 
		  (_y - p->y())*(_y - p->y()) +
		  (_z - p->z())*(_z - p->z())));}
};


/**
 * class Position_list
 *    A list of Positions, which allows addition and deletion at the
 *    current spot, and has start(), next(), done() functions
 *    for use in a for loop. Use like this:
 *    Position_list pos;
 *    // fill list with some stuff, using pos.insert()
 *    for(pos.start(); pos.notDone(); pos.next()) {
 *       // do stuff with pos.currX(), pos.currY() and pos.currTime()
 *    }
 */
class Position_list 
{
protected:
    Position * _head;
    Position * _curr;
    Position * _tail;
public:
    Position_list() : _head(NULL), _curr(NULL), _tail(NULL) { }
    
    /* methods for use in a for loop - see class comment above */
    inline void start() { _curr = _head; }
    inline int notDone() { return(_curr != NULL); }
    inline Position * next() {
	if (_curr == NULL) {
		return NULL;
	} else {
		_curr = _curr->next();
		return _curr;
	}
    }
    inline Position * prev() {
	if (_curr == NULL) {
		return NULL;
	} else {
		_curr = _curr->prev();
		return _curr;
	}
    }

    /* methods to let you look around in the list, without
     * affecting your position in the list */
    inline Position * curr() { return _curr; }
    inline double currX() { return _curr->x(); }
    inline double currY() { return _curr->y(); }
    inline double currZ() { return _curr->z(); }

    inline Position * peekNext() {
	if (_curr == NULL) {
		return NULL;
	} else {
		return (_curr->next());
	}
    }

    inline Position * peekPrev() {
	if (_curr == NULL) {
		return NULL;
	} else {
		return (_curr->prev());
	}
    }

    inline int empty() { return (_head== NULL); }
    
    /** change the current item to head or tail */
    inline void goToHead() { _curr = _head; }
    inline void goToTail() { _curr = _tail; }
    /** insert into list - add a new item after the current one, and the
     * new item becomes current.
     *   good for inserting at end of list. 
     * returns 0 if successful, -1 if fails */
    int insert(Position * p);
    int insert(double x, double y);
    int insert(double x, double y, int id);
    int insert(double x, double y, long sec, long usec);

    int insert(double x, double y, double z);
    int insert(double x, double y, double z, int id);
    int insert(double x, double y, double z, long sec, long usec);

    /** insert into list - add a new item before the current one, and the
     * new item becomes current.
     *   good for inserting at head of the list. 
     * returns 0 if successful, -1 if fails */
    int insertPrev(Position * p);
    int insertPrev(double x, double y);
    int insertPrev(double x, double y, int id);
    int insertPrev(double x, double y, long sec, long usec);

    int insertPrev(double x, double y, double z);
    int insertPrev(double x, double y, double z, int id);
    int insertPrev(double x, double y, double z, long sec, long usec);
    /** deletes the current item.
     * returns 0 if successful, -1 if fails */
    int del();

// currTime    
};

#endif
