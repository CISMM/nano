CXX=g++
CXXFLAGS=-march=pentium3 -O3 -Wall
#CXX=icc
#CXXFLAGS=-march=pentium3 -O3 -ip #-ipo # -ipo_c
GL_LIBS=-lGL -lGLU -lglut


all : display_trace depth

optimized : depth_opt display_opt

display_opt :
	icc -DNDEBUG -march=pentiumiii -O3 -ipo -o display_trace display_trace.cpp common.cpp -lGL -lGLU -lglut

depth_opt :
	icc -DNDEBUG -march=pentiumiii -O3 -ipo -o depth depth_time.cpp common.cpp

display_trace : display_trace.o common.o
	$(CXX) -o $@ $< common.o $(GL_LIBS)

depth : depth_time.o common.o
	$(CXX) -o $@ $< common.o

test : test.o common.o
	$(CXX) -o $@ $< common.o


display_trace.o :: common.h

depth_time.o :: common.h

common.o :: common.h

test.o :: common.h

clean :
	$(RM) *.o
