#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "functions.h"
#include "socket.h"

int main(int argc, char * argv[]) {
    fprintf(stderr, "Receiver Launch : number args '%d', args '%s'\n", argc, argv[0]);

    FILE * f = NULL;
    char openMode[] = "w+";

    int port = 0;
    char * address = NULL;
    struct sockaddr_in6 addr;

    if(readArgs(argc, argv, &address, &port, &f, openMode)){
        // Address translation
        const char *err = real_address(address, &addr);
        if (err) {
            fprintf(stderr, "Could not resolve hostname %s: %s\n", address, err);
            return EXIT_FAILURE;
        }

        // Socket creation
        /* Get a socket */
        int sfd;
        sfd = create_socket(&addr, port, NULL, -1); /* Bound */
        if (sfd > 0 && wait_for_client(sfd) < 0) { /* Connected */
            fprintf(stderr, "Could not connect the socket after the first message.\n");
            close(sfd);
            return EXIT_FAILURE;
        }
        if (sfd < 0) {
            fprintf(stderr, "Failed to create the socket!\n");
            return EXIT_FAILURE;
        }
        // TODO
        /*
         * Messages exchange
         */
        close(sfd);
        fprintf(stderr, "Exiting with success!\n");
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
