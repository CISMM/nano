
%{
  
  /*
   *This lex program takes in a protein data bank file, returns the (X,Y,Z) 
 * values of atoms.
 */
  
  int start,xread,yread,zread=0;
  int skipnum=0;
%}

%%
^ATOM            {skipnum=5;start=1;}
[+-\.0-9]+          {if (!skipnum & start) {xread=1;start=0;} else skipnum--;
 if (xread) {printf("%s ",yytext); xread = 0;yread=1;}
 else if (yread) {printf ("%s ",yytext); yread = 0; zread=1;}
 else if (zread) {printf ("%s\n",yytext); zread=0;}
}
[\.0-9a-zA-Z]+	if (start) skipnum--;
.               |
\n		;

%%
