#include <math.h>
#include <malloc.h>	// NULL is defined here?
#include <stdio.h>
#include "Position.h"

/***********************************************************
 * Methods for class Position - see header for class description
 ***********************************************************/
// hmm. I guess there aren't any right now.


/***********************************************************
 * Methods for class Position_list - see header for class description
 ***********************************************************/


/* insert into list - add a new item after the current one, and the
 * new item becomes current.
 *   good for inserting at end of list. 
 * returns 0 if successful, -1 if fails */
int Position_list::insert(Position * p)
{
    // special case - empty list
    if (_head==NULL) {
	_head = _tail = _curr = p;
    // special case - curr is end of list
    } else if (_curr==_tail) {
	_curr->next(p);
	p->prev(_curr);
	_curr = _tail = p;
    // default case - insert in middle (can't insert at head)
    } else {
	_curr->next()->prev(p);
	_curr->next(p);
	p->prev(_curr);
	p->next(_curr->next());
	_curr = p;
    }
    return 0;
}

int Position_list::insert(double x, double y)
{
    Position * p = new Position(x,y);
    if (p==NULL) return (-1);
    else
    return ((insert(p)));
}

int Position_list::insert(double x, double y, int id)
{
    Position * p = new Position(x,y);
    if (p==NULL) return (-1);
    else {
	p->setIconID(id);
	return ((insert(p)));
    }
}

int Position_list::insert(double x, double y, long sec, long usec)
{
    Position * p = new Position(x,y,sec,usec);
    if (p==NULL) return (-1);
    else
    return ((insert(p)));
}

/* insert into list - add a new item before the current one, and the
 * new item becomes current.
 *   good for inserting at head of the list. 
 * returns 0 if successful, -1 if fails */
int Position_list::insertPrev(Position * p)
{
    // special case - empty list
    if (_head==NULL) {
	_head = _tail = _curr = p;
    // special case - curr is head of list
    } else if (_curr==_head) {
	_curr->prev(p);
	p->next(_curr);
	_curr = _head = p;
    // default case - insert in middle (can't insert at tail)
    } else {
	_curr = _curr->prev();
	return ((insert(p))); // trick :-) -> less code to debug.
    }

    return 0;
}

int Position_list::insertPrev(double x, double y)
{
    Position * p = new Position(x,y);
    if (p==NULL) return (-1);
    else
    return ((insertPrev(p)));
}

int Position_list::insertPrev(double x, double y, int id)
{
    Position * p = new Position(x,y);
    if (p==NULL) return (-1);
    else {
	p->setIconID(id);
	return ((insertPrev(p)));
    }
}

int Position_list::insertPrev(double x, double y, long sec, long usec)
{
    Position *p = new Position(x,y,sec,usec);
    if (p==NULL) return (-1);
    else
    return ((insertPrev(p)));
}

/* deletes the current item.
 * curr is changed to point to the next item, except at the tail,
 *     where it points to the new tail.
 * returns 0 if successful, -1 if fails */
int Position_list::del()
{
     // special case - empty item
    if (_curr==NULL) {
	return -1;
    // special case - curr is head of list
    } else if (_curr==_head) {
	if (_curr->next() != NULL)
		_curr->next()->prev(NULL);
	_head = _curr->next();
	delete (_curr);
	_curr = _head;
    // special case - curr is end of list
    } else if (_curr==_tail) {
	_curr->prev()->next(NULL);
	_tail = _curr->prev();
	delete (_curr);
	_curr = _tail;
    // default case - delete in middle 
    } else {
	_curr->next()->prev(_curr->prev());
	_curr->prev()->next(_curr->next());
	Position * temp = _curr->next();
	delete (_curr);
	_curr = temp;
    }
    return 0;
}
