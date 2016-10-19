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

void reading_loop(const int sfd, FILE * outFile);

#endif // RECEIVER_H_INCLUDED