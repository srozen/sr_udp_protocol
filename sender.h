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

// > 2 * Latence (2000)
#define TIME_OUT 5000

void writing_loop(const int sfd, FILE * inFile);

#endif // SENDER_H_INCLUDED
