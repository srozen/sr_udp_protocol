CC=gcc
CFLAGS=-std=c99 -Wall -Werror -Wshadow -Wextra -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-all -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE

# Uncomment to compile cUnit test 
LDFLAGS=-lz # -L$(HOME)/local/lib -lcunit

EXEC=sender receiver # unit_test
SRCDIR=src/
OUTDIR=./tests/

all: $(EXEC)

unit_test: $(SRCDIR)unit_test.o $(SRCDIR)functions.o $(SRCDIR)socket.o
	$(CC) -o $(OUTDIR)$@ $^ $(LDFLAGS)

sender: $(SRCDIR)sender.o $(SRCDIR)functions.o $(SRCDIR)socket.o $(SRCDIR)packet_implem.o $(SRCDIR)packet_debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

receiver: $(SRCDIR)receiver.o $(SRCDIR)functions.o $(SRCDIR)socket.o $(SRCDIR)packet_implem.o $(SRCDIR)packet_debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

debug: clean all

.PHONY: clean mrproper

clean:
	rm -rf $(SRCDIR)*.o

mrproper: clean
	rm -rf sender receiver $(OUTDIR)unit_test


