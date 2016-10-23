#ifndef SR_UDP_PROTOCOL_FUNCTIONS_H
#define SR_UDP_PROTOCOL_FUNCTIONS_H

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>

/* Initialize @address, @port and @f from user input
 * @f : File accessor if a filename is provided by the user
 */
int read_args(int argc, char * argv[], char** address, int * port, FILE** f, char * openMode);

/*
 * Return a timestamp from system time
 */
uint32_t timestamp();

#endif //SR_UDP_PROTOCOL_FUNCTIONS_H
