#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<math.h>
#include <nmb_ColorMap.h>

void	flookup(nmb_ColorMap *m, char *name, float value)
{
	float		fr,fg,fb,fa;
	m->lookup(value, &fr, &fg, &fb, &fa);
	printf("%s at %g is (%g,%g,%g, %g)\n",name,value,fr,fg,fb,fa);
}

void	ilookup(nmb_ColorMap *m, char *name, float value)
{
	int		ir,ig,ib,ia;
	m->lookup(value, &ir, &ig, &ib, &ia);
	printf("%s at %g is (%d,%d,%d, %d)\n",name,value,ir,ig,ib,ia);
}

void	main(unsigned argc, char *argv[])
{
	nmb_ColorMap	red;
	nmb_ColorMap	blue("blue");

	red.load_from_file("red");

	flookup(&red,"red",0.2);
	flookup(&blue,"blue",0.4);

	ilookup(&red,"red",0.3);
	ilookup(&blue,"blue",2.0);
}

