####################################################
# Makefile for SGI machine.
####################################################
#
# Define this when you are using OpenGL
#
GL_LIBS = -lGLU -lGL


####################################################
# Define this to be glut
#CXX = g++
CXX = CC -g -o32


GLUT_LIBS = -lglut
CXXFLAGS = -I/afs/unc/proj/walk/include -lang:std
LDFLAGS = -L/afs/unc/proj/walk/lib/sgi_irix

####################################################
# Define this to be your X libraries
SYS_LIBS = -lXmu -lXext -lX11 -lm 
EXT = 

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: sim

include depend

OBJS = Vec3d.o ConeSphere.o 3Dobject.o Tips.o Unca.o Uncertw.o dna.o scan.o input.o draw.o lightcol.o uncert.o sim.o 
FILES= Vec3d.cpp ConeSphere.cpp 3Dobject.cpp Tips.cpp Unca.cpp Uncertw.cpp dna.cpp scan.cpp input.cpp draw.cpp lightcol.cpp uncert.cpp sim.cpp
sim : $(OBJS) defns.h depend
	$(CXX) -o sim $(OBJS) $(LDFLAGS) $(GLUT_LIBS) $(GL_LIBS) $(SYS_LIBS)

dep : depend
	$(CXX) $(FILES) $(CXXFLAGS) $(LDFLAGS) $(GLUT_LIBS) $(GL_LIBS) $(SYS_LIBS) -M > depend


disp : disp.o
	$(CXX) -o disp disp.o $(LDFLAGS) $(GLUT_LIBS) $(GL_LIBS) $(SYS_LIBS)

clean:
	-rm -f *.o
	-rm sim$(EXT)

depend:
	touch depend

