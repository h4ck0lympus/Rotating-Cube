CC=gcc
CFLAGS=$(shell pkg-config sdl2 --cflags) -lm
LIBS=$(shell pkg-config sdl2 --libs)
SRC=rotating-cube.c
OBJ=rotating-cube

all:
	$(CC) $(CFLAGS) $(LIBS) $(SRC) -o $(OBJ)

clean:
	rm -rf $(OBJ)
