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

    if(writing_loop(sfd, inFile) != EXIT_SUCCESS){
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
    fprintf(stderr, "Begin loop to write in socket\n");

    // Can't change
    const size_t sizeMaxPkt = MAX_PAYLOAD_SIZE + HEADER_SIZE;
    const int pktBufSize = MAX_WINDOW_SIZE + 1;
    const int infd = fileno(inFile);

    // Packets buffer
    pkt_t * pktBuffer[pktBufSize];
    for(int i = 0; i < pktBufSize; i++)
        pktBuffer[i] = NULL;

    uint8_t seqnum = 0; //First seqnum must be 0
    uint8_t winSize = 1; //Window size must be 1 at begining
    uint8_t lastSeqnum = seqnum; //Next seqnum


    int nfsd = sfd;
    if(infd > nfsd) {
        nfsd = infd;
    }
    int eof = 0;
    int timeEof = NB_LAUNCH_EOF;

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
        if(FD_ISSET(infd, &selRe) && winSize > 0 && timeEof == NB_LAUNCH_EOF) {
            // Read from inFile
            char fileReadBuf[MAX_PAYLOAD_SIZE];
            ssize_t nbByteRe = read(infd, fileReadBuf, MAX_PAYLOAD_SIZE);

            // Packet initialization + encode
            pkt_t * pktWr = pkt_new();
            init_pkt(pktWr, fileReadBuf, nbByteRe, seqnum, PTYPE_DATA, winSize, timestamp());

            if(send_new_packet(sfd, pktBufSize, sizeMaxPkt, pktBuffer, pktWr, nbByteRe) == 0) {
                winSize--;
                increment_seqnum(&seqnum);
                lastSeqnum = pkt_get_seqnum(pktWr);

                if(nbByteRe == 0) { // End of file
                    timeEof--;
                    lastSeqnum++;
                }
            } else {
                pkt_del(pktWr);
            }
        }

        // WHEN SOCKET RECEIVE A ACK
        if(FD_ISSET(sfd, &selRe)) {

            char socketReadBuf[sizeMaxPkt];

            ssize_t nbByteReSo = read(sfd, socketReadBuf, sizeMaxPkt); // Read data from SFD

            if(nbByteReSo < 0) {
                fprintf(stderr, "An error occured reading ACK in socket : %s\n", strerror(errno));
                return EXIT_FAILURE;
            }

            pkt_t * pktRe = pkt_new();
            pkt_decode(socketReadBuf, nbByteReSo, pktRe); // Create new packet from buffer
            pkt_debug(pktRe); // Debug ACK
            winSize = pkt_get_window(pktRe); // take the windows of receiver

            uint8_t seq = pkt_get_seqnum(pktRe);
            free_packet_buffer(pktBuffer, seq, pktBufSize, winSize);

            if(pkt_get_seqnum(pktRe) == lastSeqnum) {
                eof = 1;
            }
            pkt_del(pktRe);
        }
        timeout_check(sizeMaxPkt, sfd, pktBuffer, pktBufSize, &eof, &timeEof);
    }

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

void free_packet_buffer(pkt_t ** pktBuf, uint8_t seqnum, int pktBufSize, int winSize) {
    uint8_t cpSeqnum = seqnum;

    decrement_seqnum(&seqnum);
    int finish = 0;
    while(!finish && pktBuf[seqnum % pktBufSize] != NULL) {

        fprintf(stderr, "I free buffer number %d\n", seqnum % pktBufSize);
        pkt_del(pktBuf[seqnum % pktBufSize]);
        pktBuf[seqnum % pktBufSize] = NULL;
        decrement_seqnum(&seqnum);

        if(cpSeqnum - winSize > seqnum ||
                (cpSeqnum < seqnum && (MAX_SEQNUM - winSize + cpSeqnum < seqnum))) {
            finish = 1;
        }
    }
}

void timeout_check(const size_t sizeMaxPkt, const int sfd, pkt_t ** pktBuf, int pktBufSize, int * eof, int * timeEof) {
    uint32_t time = timestamp();
    for(int i = 0; i < pktBufSize; i++) {
        if(pktBuf[i] != NULL && time > (pkt_get_timestamp(pktBuf[i]) + TIME_OUT)) {
            char socketWriteBuf[sizeMaxPkt];

            size_t tmps = sizeMaxPkt;
            pkt_encode(pktBuf[i], socketWriteBuf, &tmps);
            int nbWrite = write(sfd, socketWriteBuf, tmps);
            pkt_set_timestamp(pktBuf[i], timestamp());
            fprintf(stderr, "Resend nb byte : %d\n", nbWrite);
            if (pkt_get_length(pktBuf[i]) == 0){
                (*timeEof)--;
                if (*timeEof <= 0){
                    fprintf(stderr, "Not respont of client, finish connection!\n");
                    *eof = 1;
                }
            }
            if(nbWrite < 0) {
                fprintf(stderr, "Client leave, finish Connection!\n");
                *eof = 1;
            }

        }
    }
}


