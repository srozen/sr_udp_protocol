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
#include <time.h>

#include "packet_debug.h"
#include "functions.h"
#include "socket.h"
#include "packet_interface.h"

// > 2 * Latence (2000) millisecond. min=4000, add 300 for traetement of packet
#define TIME_OUT 4300

#define NB_LAUNCH_EOF 3

int writing_loop(const int sfd, FILE * inFile);

int send_new_packet(const int sfd, const int pktBufSize, const size_t sizeMaxPkt, pkt_t ** pktBuf, pkt_t * pktToSend,  ssize_t nbByteRe);

void init_pkt(pkt_t * pkt, char * fileReadBuf, const uint16_t nbByteRe, const uint8_t seqNum, const ptypes_t type, const uint8_t winSize, const uint32_t timestamp);

void free_packet_buffer(pkt_t ** pktBuf, uint8_t seqnum, int pktBufSize, int winSize);

void timeout_check(const size_t sizeMaxPkt, const int sfd,pkt_t ** pktBuf, int pktBufSize, int * eof, int * timeEof);

int can_send_packet(uint8_t ackSeqnum, uint8_t seqnum, int winSize);

#endif // SENDER_H_INCLUDED
