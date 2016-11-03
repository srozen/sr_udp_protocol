#include "sender.h"

int main(int argc, char * argv[]) {
    fprintf(stderr, "Sender Launch : number args '%d'\n", argc);

    FILE * inFile = stdin;
    char openMode[] = "r";

    int port = 0;
    char * address = NULL;
    struct sockaddr_in6 addr;

    if(read_args(argc, argv, &address, &port, &inFile, openMode) != 0) {
        return EXIT_FAILURE;
    }
    const char *err = real_address(address, &addr);
    if(err) {
        fprintf(stderr, "Could not resolve hostname %s: %s\n", address, err);
        return EXIT_FAILURE;
    }
    int sfd;
    sfd = create_socket(NULL, -1, &addr, port);
    if(sfd < 0) {
        fprintf(stderr, "Failed to create the socket!\n");
        return EXIT_FAILURE;
    }

    if(writing_loop(sfd, inFile) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    close(sfd);
    if(inFile != stdin) {
        close(fileno(inFile));
    }
    fprintf(stderr, "Exiting with success! (Sender)\n");
    return EXIT_SUCCESS;
}

int writing_loop(const int sfd, FILE * inFile) {
    // Can't change
    const size_t sizeMaxPkt = MAX_PAYLOAD_SIZE + HEADER_SIZE;
    const int pktBufSize = MAX_WINDOW_SIZE + 1;
    const int infd = fileno(inFile);

    // Packets buffer
    pkt_t * pktBuffer[pktBufSize];
    for(int i = 0; i < pktBufSize; i++)
        pktBuffer[i] = NULL;

    uint8_t nextSeqnum = 0; // First seqnum must be 0, correspond to the next seqnum to send.
    uint8_t winSize = 1; //Window size must be 1 at begining
    uint8_t ackSeqnum = MAX_SEQNUM; // seqnum of last Ack receive
    int eofSeqnum = -1; // seqnum of eof

    int nfsd = sfd;
    if(infd > nfsd) {
        nfsd = infd;
    }
    int eof = 0; // To exit the loop
    int timeEof = NB_LAUNCH_EOF; // Nb of maximal time we resend eof
    int eofRead = 0; // If eofRead have be read
    int lastSeqSend = -1; // seqnum of

    fd_set selRe;
    fd_set selWri;

    FD_ZERO(&selRe);
    FD_ZERO(&selWri);

    while(!eof) {

        FD_SET(infd, &selRe);
        FD_SET(sfd, &selRe);
        FD_SET(sfd, &selWri);

        if((select(nfsd + 1, &selRe, &selWri, NULL, NULL)) < 0) {
            fprintf(stderr, "An error occured on select %s (writing_loop)\n", strerror(errno));
            return EXIT_FAILURE;
        }

        /*When you can read in file and the windows size is not empty,
          also if the end of file was not send */
        if(FD_ISSET(infd, &selRe) && timeEof == NB_LAUNCH_EOF && can_send_packet(ackSeqnum, nextSeqnum, winSize) == 0) {

            // Read from inFile
            char fileReadBuf[MAX_PAYLOAD_SIZE];
            ssize_t nbByteRe = read(infd, fileReadBuf, MAX_PAYLOAD_SIZE);

            // Packet initialization + encode
            pkt_t * pktWr = pkt_new();
            init_pkt(pktWr, fileReadBuf, nbByteRe, nextSeqnum, PTYPE_DATA, winSize, timestamp());

            if(nbByteRe == 0) {
                eofRead = 1;
                lastSeqSend = nextSeqnum;
                pkt_del(pktWr);
            } else if(send_new_packet(sfd, pktBufSize, sizeMaxPkt, pktBuffer, pktWr, nbByteRe) == 0) {
                increment_seqnum(&nextSeqnum);
            } else {
                pkt_del(pktWr);
            }
        }

        if(eofRead == 1 && (int)ackSeqnum == lastSeqSend) { // Waiting to send eof
            pkt_t * pktEof = pkt_new();
            init_pkt(pktEof, "", 0, nextSeqnum, PTYPE_DATA, winSize, timestamp());
            if(send_new_packet(sfd, pktBufSize, sizeMaxPkt, pktBuffer, pktEof, 0) == 0) {
                timeEof--;
                increment_seqnum(&nextSeqnum);
                eofSeqnum = nextSeqnum;
                increment_seqnum(&nextSeqnum);
                // DEBUG
                fprintf(stderr, "End of file send for the first time (eofSeqnum = %d)\n", eofSeqnum);
            } else {
                pkt_del(pktEof);
            }

            eofRead = 0;
        }

        // When a socket have received a ACK
        if(FD_ISSET(sfd, &selRe)) {

            char socketReadBuf[sizeMaxPkt];
            ssize_t nbByteReSo = read(sfd, socketReadBuf, sizeMaxPkt); // Read data from SFD
            if(nbByteReSo < 0) {
                fprintf(stderr, "An error occured reading ACK in socket : %s\n", strerror(errno));
                return EXIT_FAILURE;
            }

            pkt_t * pktRe = pkt_new();
            int decodeRet = pkt_decode(socketReadBuf, nbByteReSo, pktRe); // Create new packet from buffer
            if(decodeRet != PKT_OK) {
                fprintf(stderr, "Error in decode of ack, error code = %d\n", decodeRet);
            } else {
                // DEBUG
                pkt_debug(pktRe);
                ackSeqnum = pkt_get_seqnum(pktRe);
                free_packet_buffer(pktBuffer, ackSeqnum, pktBufSize);
                winSize = pkt_get_window(pktRe); // Take the window of receiver
                if(ackSeqnum == eofSeqnum) {
                    fprintf(stderr, "Ack of end file receive, close connection.\n");
                    eof = 1;
                }
            }
            pkt_del(pktRe);
        }
        timeout_check(sizeMaxPkt, sfd, pktBuffer, pktBufSize, &eof, &timeEof);
        // TODO : if not respond since x second => finish
    }
    release_all_buffers(pktBuffer, pktBufSize);
    return EXIT_SUCCESS;
}

int send_new_packet(const int sfd, const int pktBufSize, const size_t sizeMaxPkt, pkt_t ** pktBuf, pkt_t * pktToSend,  ssize_t nbByteRe) {
    size_t sizeEncode = sizeMaxPkt;
    char socketWriteBuf[sizeEncode];
    int validPkt = pkt_encode(pktToSend, socketWriteBuf, &sizeEncode);

    if(validPkt != PKT_OK) {
        fprintf(stderr, "Packet to send not valid, error code : %d\n", validPkt);
        return -1;
    }

    ssize_t nbByteWr = write(sfd, socketWriteBuf, sizeEncode);

    // Put packet in buffer packet
    pktBuf[pkt_get_seqnum(pktToSend) % pktBufSize] = pktToSend;
    if(nbByteWr != nbByteRe + HEADER_SIZE) {
        fprintf(stderr, "Error on write from file in socket\n");
        return -1;
    }
    fprintf(stderr, "Send a packet (byte write %d)\n", (int) nbByteWr);
    return 0;
}

void init_pkt(pkt_t * pkt, char * fileReadBuf, const uint16_t nbByteRe, const uint8_t seqNum, const ptypes_t type, const uint8_t winSize, const uint32_t timestamp) {
    pkt_set_payload(pkt, fileReadBuf, nbByteRe);
    pkt_set_seqnum(pkt, seqNum);
    pkt_set_type(pkt, type);
    pkt_set_window(pkt, winSize);
    pkt_set_timestamp(pkt, timestamp);
}

void free_packet_buffer(pkt_t ** pktBuf, uint8_t ackSeqnum, int pktBufSize) {

    const uint8_t cpSeq = ackSeqnum;

    decrement_seqnum(&ackSeqnum);
    while(pktBuf[ackSeqnum % pktBufSize] != NULL) {
        uint8_t currenSeq = pkt_get_seqnum(pktBuf[ackSeqnum % pktBufSize]);
        if(currenSeq > (int)cpSeq + (MAX_SEQNUM - pktBufSize)) {
            // DEBUG
            fprintf(stderr, "I free buffer number %d, seqnum = %d\n", currenSeq % pktBufSize, currenSeq);
            pkt_del(pktBuf[currenSeq % pktBufSize]);
            pktBuf[currenSeq % pktBufSize] = NULL;
        } else {
            if(currenSeq < (int)cpSeq && currenSeq >= (int)cpSeq - pktBufSize) {
                // DEBUG
                fprintf(stderr, "I free buffer number %d, seqnum %d\n", currenSeq % pktBufSize, currenSeq);
                pkt_del(pktBuf[currenSeq % pktBufSize]);
                pktBuf[currenSeq % pktBufSize] = NULL;
            } else  {
                return;
            }
        }
        decrement_seqnum(&ackSeqnum);
    }

}

void timeout_check(const size_t sizeMaxPkt, const int sfd, pkt_t ** pktBuf, int pktBufSize, int * eof, int * timeEof) {
    uint32_t time = timestamp();
    for(int i = 0; i < pktBufSize; i++) {
        if(pktBuf[i] != NULL && time > (pkt_get_timestamp(pktBuf[i]) + TIME_OUT)) {
            char socketWriteBuf[sizeMaxPkt];

            pkt_set_timestamp(pktBuf[i], timestamp());
            size_t sizeMaxPktCp = sizeMaxPkt;
            pkt_encode(pktBuf[i], socketWriteBuf, &sizeMaxPktCp);
            int nbWrite = write(sfd, socketWriteBuf, sizeMaxPktCp);

            // DEBUG
            fprintf(stderr, "Resend packet, seqnum = %d, nb byte : %d\n", pkt_get_seqnum(pktBuf[i]), nbWrite);
            if(nbWrite < 0) {
                fprintf(stderr, "Client leave, finish Connection!\n");
                *eof = 1;
                return;
            }
            if(pkt_get_length(pktBuf[i]) == 0) {
                (*timeEof)--;
                if(*timeEof <= 0) {
                    fprintf(stderr, "Not reponse of client for end of file, finish connection!\n");
                    *eof = 1;
                }
            }
        }
    }
}

int can_send_packet(uint8_t ackSeqnum, uint8_t nextSeqnum, int winSize) {
    if((((int)ackSeqnum) + winSize) > MAX_SEQNUM) {
        if(nextSeqnum >= ackSeqnum) {
            return 0;
        }
        if((winSize - (MAX_SEQNUM - ackSeqnum)) > nextSeqnum) {
            return 0;
        }
        return 1;
    } else {
        if(nextSeqnum > ackSeqnum + winSize - 1) {
            return 1;
        }
    }
    return 0;
}
