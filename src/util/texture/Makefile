# spots by reaction-diffusion

CFLAGS = -O -I. -I/usr/include -I/usr/local/include
LIBS = /usr/local/lib/X11/libX11.a -lm

OBJ1 = linear.o graphics.o
OBJ2 = spots.o graphics.o

current: spots linear

linear: $(OBJ1) Makefile
	cc -o linear $(OBJ1) $(LIBS) $(CFLAGS)

spots: $(OBJ2) Makefile
	cc -o spots $(OBJ2) $(LIBS) $(CFLAGS)

