
# Tuto : http://gl.developpez.com/tutoriel/outil/makefile/
CC=gcc
CFLAGS=-std=c99 -Wall -Werror -Wshadow -Wextra -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-all
LDFLAGS=-lz
EXEC=test sender receiver

all: $(EXEC)

test: test.o packet_implem.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender: sender.o
	$(CC) -o $@ $^ $(LDFLAGS)

receiver: receiver.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

debug: clean all

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)


