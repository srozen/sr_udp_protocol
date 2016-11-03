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

/*
 * Main loop when the connection is established.
 */
int reading_loop(const int sfd, FILE * outFile);

/*
 * Send a ack to the sender
 */
void send_ack(const int sfd, uint8_t seqnum, uint8_t window, uint32_t timestamp);

/*
 * Read a packet in the socket and decode.
 *
 * @return pkt filled struct pointer or NULL when there is a error.
 */
pkt_t * read_packet(const int sizeMaxPkt, int sfd);

#endif // RECEIVER_H_INCLUDED
