#include "sender.h"

int main(int argc, char * argv[]) {
    fprintf(stderr, "Sender Launch : number args '%d'\n", argc);

    FILE * f = stdin;
    char openMode[] = "r";

    int port = 0;
    char * address = NULL;
    struct sockaddr_in6 addr;

    if(!read_args(argc, argv, &address, &port, &f, openMode)) {
        return EXIT_FAILURE;
    }

    // Address translation
    const char *err = real_address(address, &addr);
    if(err) {
        fprintf(stderr, "Could not resolve hostname %s: %s\n", address, err);
        return EXIT_FAILURE;
    }

    // Socket creation
    int sfd;
    sfd = create_socket(NULL, -1, &addr, port); /* Connected */
    if(sfd < 0) {
        fprintf(stderr, "Failed to create the socket!\n");
        return EXIT_FAILURE;
    }

    writing_loop(sfd, f);

    close(sfd);
    fprintf(stderr, "Exiting with success! (Sender)\n");
    return EXIT_SUCCESS;
}

void writing_loop(const int sfd, FILE * inFile) {

    fprintf(stderr, "Begin loop to write in socket\n");

    pkt_t * pktWr = pkt_new();
    size_t sizeMaxPkt = MAX_PAYLOAD_SIZE + sizeof(pktWr);

    char bufWr[sizeMaxPkt];
    char bufRe[MAX_PAYLOAD_SIZE];

    // TODO in loop depending the file and wait ack

    ssize_t nbByteRe = read(fileno(inFile),bufRe,MAX_PAYLOAD_SIZE);
    pkt_set_payload(pktWr,bufRe, nbByteRe);
    pkt_encode(pktWr, bufWr, &sizeMaxPkt);

    ssize_t nbByteWr = write(sfd, bufWr, sizeMaxPkt);
    fprintf(stderr, "nb byte Send %d\n", (int)nbByteWr);
    /*
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

    } */

}
