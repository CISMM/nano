#ifndef _INPUT_H_
#define _INPUT_H_

// AFM tip
//extern SphereTip sp;
//extern InvConeSphereTip ics;
extern Tip tip;

void initObs( int numtoDraw );
void addSpheresFromFile (char *filename, double no_of_nm_in_one_unit, 
bool rad_exists);
void addTrianglesFromFile(char *filename, double scale);
#endif
