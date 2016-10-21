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
    const int sizeMaxPkt = MAX_PAYLOAD_SIZE + 12;
    pkt_t * bufPkt[MAX_WINDOW_SIZE];

    uint8_t indWinRe = 0; // Index of reading in window buffer
    uint8_t winFree = MAX_WINDOW_SIZE; // nb free place in window
    int lastSeqNum = 0;

    char bufRe[sizeMaxPkt];

    int outfd = fileno(outFile);
    int eof = 0;

    fd_set seSoRe;
    fd_set seOutFi;

    while(!eof) {

        FD_ZERO(&seSoRe);
        FD_ZERO(&seOutFi);

        FD_SET(sfd, &seSoRe);
        FD_SET(outfd, &seOutFi);

        if((select(sfd + 1, &seSoRe, &seOutFi, NULL, NULL)) < 0) {
            fprintf(stderr, "Reading_loop : Select error\n");
            break;
        }

        // Look in socket
        if(FD_ISSET(sfd, &seSoRe)) {

            // Read a packet in the socket and decode then.
            pkt_t * pktRe = pkt_new();
            ssize_t nbByteR = read(sfd, bufRe, sizeMaxPkt);
            int validPkt = pkt_decode(bufRe, nbByteR, pktRe);

            if(validPkt == PKT_OK && winFree > 0) { // Verify the integrity of pkt and if there is place in window buffer
                pkt_debug(pktRe);
                // Put in buf window
                bufPkt[pkt_get_seqnum(pktRe) % (MAX_WINDOW_SIZE + 1)] = pktRe;
                winFree--;
                // TODO ACK
                send_ack(sfd, pkt_get_seqnum(pktRe) + 1, winFree, pkt_get_timestamp(pktRe));
            } else {
                // If packet is not correct
                fprintf(stderr, "Packet not valid, error code : %d\n", validPkt);
                pkt_del(pktRe);
            }
        }

        if(FD_ISSET(sfd, &seOutFi)) {
            // If next pkt can be write
            if(bufPkt[indWinRe] != NULL && ((pkt_get_seqnum(bufPkt[indWinRe]) % (MAX_WINDOW_SIZE + 1))) == indWinRe) {

                if(pkt_get_length(bufPkt[indWinRe]) == 0) { // End of file receive
                    pkt_del(bufPkt[indWinRe]);
                    bufPkt[indWinRe] = NULL;
                    eof = 1;
                } else { // Packet with payload.
                    ssize_t nbByteW = write(outfd, pkt_get_payload(bufPkt[indWinRe]), pkt_get_length(bufPkt[indWinRe]));
                    fprintf(stderr, "Write in file, nb wrotte bytes : %d\n", (int) nbByteW);
                    pkt_del(bufPkt[indWinRe]);
                    bufPkt[indWinRe] = NULL;

                    indWinRe++;
                    if(indWinRe > MAX_WINDOW_SIZE) {
                        indWinRe = 0;
                    }
                    winFree++;
                }
            }
        }
    }
}

void send_ack(const int sfd, uint8_t seqnum, uint8_t window, uint32_t timestamp) {
    pkt_t * pktAck = pkt_new();

    pkt_set_seqnum(pktAck, seqnum);
    pkt_set_type(pktAck, PTYPE_ACK);
    pkt_set_window(pktAck, window);
    pkt_set_timestamp(pktAck, timestamp);

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

void writePkt(){



}
