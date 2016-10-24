#include <time.h>
#include "sender.h"
#include "packet_debug.h"

int main(int argc, char * argv[]) {
    fprintf(stderr, "Sender Launch : number args '%d'\n", argc);

    FILE * f = stdin;
    char openMode[] = "r";

    int port = 0;
    char * address = NULL;
    struct sockaddr_in6 addr;

    if(read_args(argc, argv, &address, &port, &f, openMode) != 0) {
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
    writing_loop(sfd, f);

    close(sfd);
    fprintf(stderr, "Exiting with success! (Sender)\n");
    return EXIT_SUCCESS;
}



void writing_loop(const int sfd, FILE * inFile) {
    fprintf(stderr, "Begin loop to write in socket\n");

    pkt_t * pktRe = pkt_new();
    const size_t sizeMaxPkt = MAX_PAYLOAD_SIZE + HEADER_SIZE;

    // Packets buffers
    const int pktBufSize = MAX_WINDOW_SIZE + 1;
    pkt_t * pktBuffer[pktBufSize];
    for(int i = 0; i < pktBufSize; i++)
        pktBuffer[i] = NULL;

    char fileReadBuf[MAX_PAYLOAD_SIZE];
    char socketReadBuf[sizeMaxPkt];
    char socketWriteBuf[sizeMaxPkt];

    uint8_t seqnum = 0; //First seqnum must be 0
    uint8_t winSize = 1; //Window size must be 1 at begining
    uint8_t lastSeqnum = seqnum;

    int infd = fileno(inFile);
    int eof = 0;

    int allWritten = 1;

    fd_set selRe;
    fd_set selWri;

    FD_ZERO(&selRe);
    FD_ZERO(&selWri);

    while(!eof) {

        FD_SET(infd, &selRe);
        FD_SET(sfd, &selRe);
        FD_SET(sfd, &selWri);

        if((select(sfd + 1, &selRe, &selWri, NULL, NULL)) < 0) {
            fprintf(stderr, "An error occured on select %s (writing_loop)\n", strerror(errno));
            break;
        }

        if(FD_ISSET(infd, &selRe)) {
            if(winSize > 0 && allWritten) {
                ssize_t nbByteReFi = read(fileno(inFile), fileReadBuf, MAX_PAYLOAD_SIZE);
                pkt_t * pktWr = pkt_new();
                // Packet initialization + encode
                init_pkt(pktWr, fileReadBuf, nbByteReFi, seqnum, PTYPE_DATA, winSize, timestamp());
                size_t tmp = sizeMaxPkt;
                int validPkt = pkt_encode(pktWr, socketWriteBuf, &tmp);

                if(validPkt == 0) {
                    ssize_t nbByteWr = write(sfd, socketWriteBuf, tmp);
                    fprintf(stderr, "SEND\n");
                    // put packet packet buffer
                    pktBuffer[pkt_get_seqnum(pktWr) % pktBufSize] = pktWr;
                    if(nbByteWr != nbByteReFi + (int)sizeof(pktWr) + 4) {
                        fprintf(stderr, "Error on write from file\n");
                    }
                } else {
                    fprintf(stderr, "Packet not valid, error code : %d\n", validPkt);
                }

                winSize--;
                increment_seqnum(&seqnum);
                lastSeqnum = pkt_get_seqnum(pktWr);

                if(nbByteReFi == 0) {
                    allWritten = 0;
                    lastSeqnum++;
                }
            }
        }

        // WHEN SOCKET RECEIVE A ACK
        if(FD_ISSET(sfd, &selRe)) {
            ssize_t nbByteReSo = read(sfd, socketReadBuf, sizeMaxPkt); // Read data from SFD

            if(nbByteReSo < 0) {
                fprintf(stderr, "An error occured reading ACK in socket : %s\n", strerror(errno));
            }
            pkt_decode(socketReadBuf, nbByteReSo, pktRe); // Create new packet from buffer
            pkt_debug(pktRe); // Debug ACK
            winSize = pkt_get_window(pktRe);

            uint8_t seq = pkt_get_seqnum(pktRe);
            free_packet_buffer(pktBuffer, seq, pktBufSize, winSize);

            if(pkt_get_seqnum(pktRe) == lastSeqnum) {
                eof = 1;
            }
        }

        timeout_check(pktBuffer, sfd, pktBufSize, &eof);
    }

    pkt_del(pktRe);
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

void timeout_check(pkt_t ** pktBuf, const int sfd, int pktBufSize, int * eof) {
    for(int i = 0; i < pktBufSize; i++) {
        if(pktBuf[i] != NULL && timestamp() > (pkt_get_timestamp(pktBuf[i]) + TIME_OUT)) {
            const size_t sizeMaxPkt = MAX_PAYLOAD_SIZE + HEADER_SIZE;
            char socketWriteBuf[sizeMaxPkt];

            size_t tmps = sizeMaxPkt;
            pkt_encode(pktBuf[i], socketWriteBuf, &tmps);
            int nbWrite = write(sfd, socketWriteBuf, tmps);
            fprintf(stderr, "Resend nb byte : %d\n", nbWrite);
            pkt_set_timestamp(pktBuf[i], timestamp());

            if(nbWrite < 0) {
                fprintf(stderr, "Client leave, finish Connection!\n");
                *eof = 1;
            }
        }
    }
}


