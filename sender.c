#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "functions.h"
#include "socket.h"

int main(int argc, char * argv[]){
    fprintf(stderr,"Sender Launch : number args '%d', args '%s'\n", argc, argv[0]);

    FILE * f = NULL;
    char openMode[] = "r";

    int port = 0;
    char * address = NULL;
    struct sockaddr_in6 addr;


    if(readArgs(argc, argv, &address, &port, &f, openMode)){
        const char *err = real_address(address, &addr);
        if (err) {
            fprintf(stderr, "Could not resolve hostname %s: %s\n", address, err);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
