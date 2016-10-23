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

// > 2 * Latence (2000) millisecond
#define TIME_OUT 4300

void writing_loop(const int sfd, FILE * inFile);

void init_pkt(pkt_t * pkt, char * fileReadBuf, const uint16_t nbByteRe, const uint8_t seqNum, const ptypes_t type, const uint8_t winSize, const uint32_t timestamp);

void free_packet_buffer(pkt_t ** pktBuf, uint8_t seqnum, int pktBufSize, int winSize);

#endif // SENDER_H_INCLUDED
