CC = CC +a1 -g
IFLAGS = -I..
LFLAGS = -g
LIBS = -lm

NANOOBJS = BCGrid.o BCPlane.o BCDebug.o BCString.o readTopometrixFile.o Topo.o

nanoNC: $(NANOOBJS) nanoNC.o
	$(CC) $(LFLAGS) nanoNC.o $(NANOOBJS) $(LIBS) -o nanoNC

BCGrid.C:
	ln -s ../$@ .

BCPlane.C:
	ln -s ../$@ .

BCDebug.C:
	ln -s ../$@ .

BCString.C:
	ln -s ../$@ .

readTopometrixFile.C:
	ln -s ../$@ .

Topo.C:
	ln -s ../$@ .

.C.o:
	$(CC) $(IFLAGS) -c $<

clean:
	\rm -f nanoNC nanoNC.o $(NANOOBJS)

