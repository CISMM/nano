all : pdb

pdb: lexer lex.yy.o 
	gcc -o pdb lex.yy.o -ll

lexer : pdb.l 
	flex pdb.l

lex.yy.o: lex.yy.c
	gcc -c lex.yy.c

clean:
	rm -f *.o pdb

