#ifndef SR_UDP_PROTOCOL_FUNCTIONS_H
#define SR_UDP_PROTOCOL_FUNCTIONS_H

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "packet_interface.h"

#define USAGE "Usage:\n-f FILE    Specify a file to send as data, or to store data in it (optional).\nHOSTNAME   IPv6 address or hostname to reach.\n PORTNUM    Port number.\n"

/* Initialize @address, @port and @f from user input
 * @f : File accessor if a filename is provided by the user
 */
int read_args(int argc, char * argv[], char** address, int * port, FILE** f, char * openMode);

/*
 * Return a timestamp from system time
 */
uint32_t timestamp();

/*
 * Free all packet buffer before closing
 */
void release_all_buffers(pkt_t ** pktBuf, int pktBufSize);

/*
 * For debug a packet
 */
void pkt_debug(pkt_t * pkt);

#endif //SR_UDP_PROTOCOL_FUNCTIONS_H
