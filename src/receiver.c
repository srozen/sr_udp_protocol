#include "receiver.h"

int main(int argc, char * argv[]) {
    fprintf(stderr, "Receiver Launch : number args '%d'\n", argc);

    FILE * outFile = stdout;
    char openMode[] = "w";

    int port = 0;
    char * address = NULL;
    struct sockaddr_in6 addr;

    if(read_args(argc, argv, &address, &port, &outFile, openMode) != 0) {
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
    if(outFile != stdout) {
        close(fileno(outFile));
    }
    fprintf(stderr, "Exiting with success! (Receiver)\n");
    return EXIT_SUCCESS;
}

int reading_loop(int sfd, FILE * outFile) {
    fprintf(stderr, "Begin loop to read socket\n");

    // init variable
    const int sizeMaxPkt = MAX_PAYLOAD_SIZE + 12;
    const int windowSize = MAX_WINDOW_SIZE + 1;
    pkt_t * bufPkt[windowSize];
    for(int i = 0; i < windowSize; i++)
        bufPkt[i] = NULL;

    uint8_t nextSeq = 0; // Next seqnum waiting to write
    uint8_t winFree = MAX_WINDOW_SIZE; // nb free place in window
    uint8_t indWinRe = nextSeq % windowSize;

    uint8_t seqnumAck = 0; // seqNum waiting to read in socket

    int outfd = fileno(outFile);
    int nfsd = sfd;
    if(outfd > nfsd) {
        nfsd = outfd;
    }
    int eof = 0;

    fd_set selRe;
    fd_set selWri;

    FD_ZERO(&selRe);
    FD_ZERO(&selWri);

    while(!eof) {

        FD_SET(sfd, &selRe);
        FD_SET(sfd, &selWri);
        FD_SET(outfd, &selWri);


        if((select(nfsd + 1, &selRe, &selWri, NULL, NULL)) < 0) {
            fprintf(stderr, "An error occured on select %s (reading_loop)\n", strerror(errno));
            return EXIT_FAILURE;
        }

        // Look in socket
        if(FD_ISSET(sfd, &selRe) && winFree > 0) {
            pkt_t * pktRead = read_packet(sizeMaxPkt, sfd);
            if(pktRead != NULL) {
                if(bufPkt[pkt_get_seqnum(pktRead) % windowSize] != NULL) {
                    pkt_del(bufPkt[pkt_get_seqnum(pktRead) % windowSize]);
                }
                bufPkt[pkt_get_seqnum(pktRead) % windowSize] = pktRead;

                uint8_t cpseq = seqnumAck;

                if(seqnumAck == pkt_get_seqnum(pktRead)) {
                    increment_seqnum(&cpseq);
                    winFree--;
                }

                send_ack(sfd, cpseq, winFree, pkt_get_timestamp(pktRead));

                // DEBUG
                // fprintf(stderr, "I'm waiting packet number %d to write in file (index = %d)\n",seqnumAck, indWinRe);
            }
        }

        // If next pkt can be write
        if(bufPkt[indWinRe] != NULL && pkt_get_seqnum(bufPkt[indWinRe]) == seqnumAck) {
            if(pkt_get_length(bufPkt[indWinRe]) == 0) { // End of file receive
                fprintf(stderr, "End of file return, close connection\n");
                eof = 1;
            } else { // Packet with payload.
                ssize_t nbByteW = write(outfd, pkt_get_payload(bufPkt[indWinRe]), pkt_get_length(bufPkt[indWinRe]));
                pkt_del(bufPkt[indWinRe]);
                bufPkt[indWinRe] = NULL;
                increment_seqnum(&seqnumAck);
                indWinRe = seqnumAck % windowSize;
                if(winFree < MAX_WINDOW_SIZE) {
                    winFree++;
                }
                // DEBUG
                fprintf(stderr, "Write in output, nb wrotte bytes : %d\n", (int) nbByteW);
            }
        }
    }

    release_all_buffers(bufPkt, windowSize);
    return EXIT_SUCCESS;
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
        // DEBUG
        fprintf(stderr, "Ack send seqnum = %d, window = %d, number byte write : %d\n", seqnum, window, nbByteAck);
    } else {
        // DEBUG
        fprintf(stderr, "Error encoding ack, number error : %d\n", statusEnc);
    }

    free(bufEnc);
    pkt_del(pktAck);
}

pkt_t * read_packet(const int sizeMaxPkt, int sfd) {
    // Read a packet in the socket and decode then.
    char bufRe[sizeMaxPkt];

    pkt_t * pktRe = pkt_new();
    ssize_t nbByteR = read(sfd, bufRe, sizeMaxPkt);
    int validPkt = pkt_decode(bufRe, nbByteR, pktRe);

    if(validPkt == PKT_OK) { // Verify the integrity of pkt.
        // DEBUG
        pkt_debug(pktRe);
        return pktRe;
    } else { // Packet not valid
        // DEBUG
        fprintf(stderr, "Packet not valid, error code : %d\n", validPkt);
        pkt_del(pktRe);
        return NULL;
    }
}
