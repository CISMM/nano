#ifndef _input_h_
#define _input_h_

// data input to the genetic algorithm... from nanomanipulator data

extern char **VARIABLE_LIST;
extern int NUMBER_OF_VARIABLES;
extern int NUMBER_OF_INPUTS;
extern float ***INPUTS;
extern int DATA_WIDTH;
extern int DATA_HEIGHT;
extern int I_INDEX;
extern int J_INDEX;

void read_inputs( char input_file[] );
void create_input_files( char *input_file, char *datafile );
void hsv_to_rgb(float h, float s, float v, float *r, float *g, float *b);

#endif
