
# Tuto : http://gl.developpez.com/tutoriel/outil/makefile/
CC=gcc
CFLAGS=-std=c99 -Wall -Werror -Wshadow -Wextra -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-all -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE
LDFLAGS=-lz -L$(HOME)/local/lib -lcunit
EXEC=sender receiver unit_test

all: $(EXEC)

unit_test: unit_test.o functions.o socket.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender: sender.o functions.o socket.o packet_implem.o packet_debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

receiver: receiver.o functions.o socket.o packet_implem.o packet_debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

debug: clean all

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)


