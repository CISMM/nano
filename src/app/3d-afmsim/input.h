#ifndef _INPUT_H_
#define _INPUT_H_

// AFM tip
extern SphereTip sp;
extern InvConeSphereTip ics;
extern Tip tip;

void initObs( void );
void addSpheresFromFile (char *filename, double no_of_nm_in_one_unit);
void addTrianglesFromFile(char *filename, double scale);
#endif
