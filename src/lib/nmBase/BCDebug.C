#include "BCDebug.h"

int BCDebug::_charactersPerLine = 55;
int BCDebug::_charactersPerTab = 6;

int BCDebug::_on = 1;

int BCDebug::_level = 0;

int BCDebug::_timesInvoked = 0;

int BCDebug::_unfinishedSubsetArray[LAST_SUBSET];

BCDebug::Function BCDebug::_functionStack[MAX_NUM_FUNCTIONS];


/******************************************************************************\
@BCDebug() --> constructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
BCDebug::BCDebug(const BCString& function, const CodeSubset subset)
{
    if (_timesInvoked == 0)
    {
	initializeStaticVariables();
	_timesInvoked++;
    }
    _functionStack[_level].name = function;
    _functionStack[_level].subset = subset;
    if (_on && _unfinishedSubsetArray[subset])
    {
	// drawSeparator();
	(void) indent();
        cout << "entering " << _functionStack[_level].name << endl;
    }
    _level++;
    _subset = subset;
}


/******************************************************************************\
@~BCDebug() --> destructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
BCDebug::~BCDebug()
{    
    _level--;
    
    if (_on && _unfinishedSubsetArray[_functionStack[_level].subset])
    {
	(void) indent();
        cout << "exiting " << _functionStack[_level].name << endl;
	// drawSeparator();
    }
}


/******************************************************************************\
@warn()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCDebug::warn(const BCString& label)
{
    if (_on && _unfinishedSubsetArray[_subset])
    {
	(void) indent();
	cout << "In " << _functionStack[_level - 1].name << ": ";
	cout << label << endl;
    }
}


/******************************************************************************\
@watch()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCDebug::watch(const BCString& label, const int value)
{
    if (_on && _unfinishedSubsetArray[_subset])
    {
	(void) indent();
        cout << "In " << _functionStack[_level - 1].name << ": ";
	cout << label << " = " << value << endl;
    }
}


/******************************************************************************\
@watch()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCDebug::watch(const BCString& label, const float value)
{
    if (_on && _unfinishedSubsetArray[_subset])
    {
	(void) indent();
        cout << "In " << _functionStack[_level - 1].name << ": ";
	cout << label << " = " << value << endl;
    }
}


/******************************************************************************\
@watch()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCDebug::watch(const BCString& label, const double value)
{
    if (_on && _unfinishedSubsetArray[_subset])
    {
	(void) indent();
        cout << "In " << _functionStack[_level - 1].name << ": ";
	cout << label << " = " << value << endl;
    }
}


/******************************************************************************\
@watch()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCDebug::watch(const BCString& label, const BCString& value)
{
    if (_on && _unfinishedSubsetArray[_subset])
    {
	(void) indent();
        cout << "In " << _functionStack[_level - 1].name << ": "; 
	cout << label << " = " << value << endl;
    }
}


/******************************************************************************\
@delay()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCDebug::delay(const long seconds)
{
    if (_on && _unfinishedSubsetArray[_subset])
    {
	(void) indent();
	for (int i = 0; i < seconds; i++)
	{
	    cout << ".";
	    cout.flush();
	    sleep(1);
	}
	cout << endl;
    }
}


/******************************************************************************\
@yes()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
int
BCDebug::yes()
{        
    return (_unfinishedSubsetArray[_subset]);
}


/******************************************************************************\
@on()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
int
BCDebug::on()
{
    return _on;
}


/******************************************************************************\
@indent()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
int
BCDebug::indent()
{

    long totalCharacters = _level * _charactersPerTab;

    for (int i = 0; i < totalCharacters; i++)
    {
    	cerr.put(' ');
    }

    return totalCharacters;
}


/******************************************************************************\
@drawSeparator()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCDebug::drawSeparator()
{
    if (_on)
    {
	for (int i = indent(); i < _charactersPerLine; i++)
	    cerr.put('-');
	cerr.put('\n');
    }
}


/******************************************************************************\
@initializeStaticVariables()
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 2-5-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCDebug::initializeStaticVariables()
{
    // causes segfault on linux (because they're already initialized?)
//#ifndef linux
#if !defined(linux) && !defined(__CYGWIN__)
    for (int i = 0; i < MAX_NUM_FUNCTIONS; i++)
    {
	_functionStack[i].subset = UNIDENTIFIED_SUBSET;
	_functionStack[i].name = BCString();
    }
#endif

    /* I don't know what this was supposed to do, but what it was
     * doing was addressing outside the array.  Badness, so I'm taking
     * it out.  XXX RMT
    _unfinishedSubsetArray[UNIDENTIFIED_SUBSET] = 1;
*/

    for (int j = 0; j < LAST_SUBSET; j++)
    {
	_unfinishedSubsetArray[j] = 1;
    }

    // exceptions
    
    //_unfinishedSubsetArray[GRID_CODE] = 0;
    //_unfinishedSubsetArray[GRID_LIST_CODE] = 0;

}

