
/* public variables and functions for relaxation compensation
**/

#ifndef MF_RELAX_H
#define MF_RELAX_H

/* variables 
**/
extern int	RelaxComp;

/* functions:
**/
extern	int	relax_set_min (int);
extern	int	relax_set_sep (int);
extern	int	relax_init (long, long, double);
extern	int	relax_comp (long, long, float *);

#endif /* MF_RELAX_H */
