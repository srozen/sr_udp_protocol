#include "receiver.h"

int main(int argc, char * argv[]) {
    fprintf(stderr, "Receiver Launch : number args '%d'\n", argc);

    FILE * outFile = stdout;
    char openMode[] = "w";

    int port = 0;
    char * address = NULL;
    struct sockaddr_in6 addr;

    if(!read_args(argc, argv, &address, &port, &outFile, openMode)) {
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

    // Enter in reading loop
    reading_loop(sfd, outFile);

    close(sfd);
    fprintf(stderr, "Exiting with success! (Receiver)\n");
    return EXIT_SUCCESS;
}

void reading_loop(int sfd, FILE * outFile) {
    fprintf(stderr, "Begin loop to read socket\n");

    // init variable
    pkt_t * pktRe = pkt_new();
    int sizeMaxPkt = MAX_PAYLOAD_SIZE + sizeof(pktRe);
    char bufRe[sizeMaxPkt];

    int outfd = fileno(outFile);
    int eof = 0;
    fd_set selSo;

    while(!eof) {

        FD_ZERO(&selSo);

        FD_SET(sfd, &selSo);
        FD_SET(outfd, &selSo);

        if((select(sfd + 1, &selSo, NULL, NULL, NULL)) < 0) {
            fprintf(stderr, "Reading_loop : Select error\n");
            break;
        }

        // Look in socket
        if(FD_ISSET(sfd, &selSo)) {

            ssize_t nbByteR = read(sfd, bufRe, sizeMaxPkt);
            int validPkt = pkt_decode(bufRe, nbByteR, pktRe);

            if(validPkt == PKT_OK) { // Verify the integrity of pkt
                pkt_debug(pktRe);
                send_ack(sfd, pkt_get_seqnum(pktRe));

                if(pkt_get_length(pktRe) > 0) {
                    ssize_t nbByteW = write(outfd, pkt_get_payload(pktRe), pkt_get_length(pktRe));

                    fprintf(stderr, "Write in file, nb wrotte bytes : %d\n", (int) nbByteW);

                } else { // End of file receive
                    eof = 1;
                }
            } else {
                fprintf(stderr, "Packet not valid, error code : %d\n", validPkt);
            }
        }
    }
}

void send_ack(const int sfd, uint8_t seqnum) {
    pkt_t * pktAck = pkt_new();

    pkt_set_seqnum(pktAck, seqnum);
    pkt_set_type(pktAck, PTYPE_ACK);
    // TODO add window ??

    size_t lenBuf = sizeof(pktAck);
    char * bufEnc = malloc(lenBuf);
    int statusEnc = pkt_encode(pktAck, bufEnc, &lenBuf);

    if(statusEnc == PKT_OK) {
        int nbByteAck = write(sfd, bufEnc, lenBuf);
        fprintf(stderr, "Ack send, number byte write : %d\n", nbByteAck);
    } else {
        fprintf(stderr, "Error encoding ack, number error : %d\n", statusEnc);
    }

    free(bufEnc);
    pkt_del(pktAck);
}
