
# Tuto : http://gl.developpez.com/tutoriel/outil/makefile/
CC=gcc

CFLAGS=-std=c99 -Wall -Werror -Wshadow -Wextra -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-all
LDFLAGS=-lz
EXEC=test sender
SRC= test.c packet_implem.c
OBJ= $(SRC:.c=.o)

all: $(EXEC)

test: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

test.o: packet_interface.h

sender: sender.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

debug: clean all

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)


