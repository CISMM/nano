####################################################
# Makefile for SGI machine.
####################################################
#
# Define this when you are using OpenGL
#
GL_LIBS = -lGLU -lGL


####################################################
# Define this to be glut
# CXX = g++
CXX = CC -O2

GLUT_LIBS = -lglut
CXXFLAGS = -I/afs/unc/proj/walk/include -o32
LDFLAGS = -L/afs/unc/proj/walk/lib/sgi_irix

####################################################
# Define this to be your X libraries
SYS_LIBS = -lXmu -lXext -lX11 -lm 
EXT = 


all: main 

include depend

OBJS = vec3d.o ConeSphere.o 3Dobject.o Tips.o Unca.o main.o 
FILES= vec3d.c++ ConeSphere.c++ 3Dobject.c++ Tips.c++ Unca.c++ main.c++ 
main : $(OBJS) defns.h
	$(CXX) -o32 -o sim $(OBJS) $(LDFLAGS) $(GLUT_LIBS) $(GL_LIBS) $(SYS_LIBS)

dep :
	$(CXX) $(FILES) $(CXXFLAGS) $(LDFLAGS) $(GLUT_LIBS) $(GL_LIBS) $(SYS_LIBS) -M > depend


disp : disp.o
	$(CXX) -o32 -o disp disp.o $(LDFLAGS) $(GLUT_LIBS) $(GL_LIBS) $(SYS_LIBS)

clean:
	-rm -f *.o
	-rm sim$(EXT)





