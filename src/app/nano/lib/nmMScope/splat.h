#ifndef SPLAT_H
#define SPLAT_H

class BCGrid;
class Point_results;

float ** mkSplat (BCGrid *);
int ptSplat (int *, BCGrid *, Point_results *);

#endif  // SPLAT_H
