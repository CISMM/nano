/******************************************************************************\
@BCDebug
--------------------------------------------------------------------------------
inheritance: BCDebug
description: 
\******************************************************************************/

#ifndef BCDEBUG_H
#define BCDEBUG_H

#ifdef	_WIN32

#include <windows.h>
#include <winnt.h>

#ifndef sleep
#define	sleep(x)	Sleep( DWORD(1000.0 * x) )
#endif

#else
#include <unistd.h> // for sleep()
#endif
#include "BCString.h"


// if you add to this enumerated type be sure to modify BCDebug::initializeStaticVariables
typedef enum 
{
    UNIDENTIFIED_SUBSET = 0,

    GRID_CODE,
    PLANE_CODE,

    // please keep the following list alphabetized to avoid duplication when adding to it

     LAST_SUBSET = 100
} CodeSubset;


const long MAX_NUM_FUNCTIONS = 15; // arbitrary!


class BCDebug
{

  public:

    BCDebug(const BCString& function, const CodeSubset subset = UNIDENTIFIED_SUBSET);
    ~BCDebug();

    void warn(const BCString& label);

    void watch(const BCString& label, const int value);
    void watch(const BCString& label, const float value);
    void watch(const BCString& label, const double value);
    void watch(const BCString& label, const BCString& value);

    void delay(const long seconds);

    int yes();

    int on();
    inline void turnOn() {_on = 0; };
    inline void turnOff() {_on = 0; };

    int indent();
    void drawSeparator();

    struct Function {
	CodeSubset subset;
	BCString name;
    };
    
  protected:

    static int _charactersPerTab;
    static int _charactersPerLine;

    static int _on;

    static int _level;

    static int _timesInvoked;

    static int _unfinishedSubsetArray [LAST_SUBSET];

    static Function _functionStack [MAX_NUM_FUNCTIONS];

    int _subset;

    void initializeStaticVariables();

  private:

};



#endif // BCDEBUG_H
