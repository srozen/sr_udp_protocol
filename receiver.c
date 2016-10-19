#include "receiver.h"

int main(int argc, char * argv[]) {
    fprintf(stderr, "Receiver Launch : number args '%d'\n", argc);

    FILE * f = stdout;
    char openMode[] = "w";

    int port = 0;
    char * address = NULL;
    struct sockaddr_in6 addr;

    if(!read_args(argc, argv, &address, &port, &f, openMode)) {
        fprintf(stderr, "Fail read arguments\n");
        return EXIT_FAILURE;
    }

    // Address translation
    const char *err = real_address(address, &addr);
    if(err) {
        fprintf(stderr, "Could not resolve hostname %s: %s\n", address, err);
        return EXIT_FAILURE;
    }

    // Socket creation
    /* Get a socket */
    int sfd;
    sfd = create_socket(&addr, port, NULL, -1); /* Bound */
    if(sfd > 0 && wait_for_client(sfd) < 0) {  /* Connected */
        fprintf(stderr, "Could not connect the socket after the first message.\n");
        close(sfd);
        return EXIT_FAILURE;
    }
    if(sfd < 0) {
        fprintf(stderr, "Failed to create the socket!\n");
        return EXIT_FAILURE;
    }

    // TODO
    /*
     * Messages exchange
     */
    close(sfd);
    fprintf(stderr, "Exiting with success! (Receiver)\n");
    return EXIT_SUCCESS;
}

void reading_loop(int sfd, FILE * outFile) {
    fprintf(stderr, "Begin loop to read socket\n");

    pkt_t * pktRe = pkt_new();
    int sizeMaxPkt = MAX_PAYLOAD_SIZE + sizeof(pktRe);
    char bufRe[sizeMaxPkt];

    int ret = 0;

    fd_set selSo;

    while(1) {

        FD_ZERO(&selSo);

        FD_SET(sfd, &selSo);
        //FD_SET(outFile, &selSo);

        if((ret = select(sfd + 1, &selSo, NULL, NULL, NULL)) < 0) {
            fprintf(stderr, "Reading_loop : Select error\n");
            break;
        }

        // LOOK  in socket
        if(FD_ISSET(sfd, &selSo)) {

            ssize_t nbByteR = read(sfd, bufRe, sizeMaxPkt);

            pkt_decode(bufRe, nbByteR, pktRe);
            ssize_t nbByteW = write(fileno(outFile), pkt_get_payload(pktRe), pkt_get_length(pktRe));

            fprintf(stderr, "Write in file, nb wrotte bytes : %d", (int) nbByteW);
        }

    }
}
