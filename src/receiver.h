#ifndef RECEIVER_H_INCLUDED
#define RECEIVER_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include "functions.h"
#include "socket.h"
#include "packet_interface.h"
#include "packet_debug.h"

int reading_loop(const int sfd, FILE * outFile);

void send_ack(const int sfd, uint8_t seqnum, uint8_t window, uint32_t timestamp);

pkt_t * read_packet(const int sizeMaxPkt, int sfd);

#endif // RECEIVER_H_INCLUDED