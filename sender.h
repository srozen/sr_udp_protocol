#ifndef SENDER_H_INCLUDED
#define SENDER_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "functions.h"
#include "socket.h"
#include "packet_interface.h"

// > 2 * Latence (2) second
#define TIME_OUT 5

void writing_loop(const int sfd, FILE * inFile);

void free_packet_buffer(pkt_t ** pktBuf, uint8_t seqnum, int pktBufSize, int winSize);

#endif // SENDER_H_INCLUDED
