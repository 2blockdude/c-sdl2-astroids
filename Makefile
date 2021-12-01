CC = gcc
CFLAGS = -g -Wall -Wextra -std=c99
LIBS = -lm -lSDL2
MAIN = astroids
SRC = sdl2-game-window primitives
OBJ = $(MAIN:%=%.o) $(SRC:%=%.o)
DEPS = $(SRC:%=%.h)

all: $(MAIN) $(OBJ)

$(MAIN): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(MAIN)
	rm -f $(OBJ)
