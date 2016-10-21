#include <time.h>
#include "sender.h"
#include "packet_debug.h"

int main(int argc, char * argv[]) {
    FILE * f = stdin;
    char openMode[] = "r";

    int port = 0;
    char * address = NULL;
    struct sockaddr_in6 addr;

    if(!read_args(argc, argv, &address, &port, &f, openMode)) {
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

void init_pkt(pkt_t * pkt, char * fileReadBuf, const uint16_t nbByteRe, const uint8_t seqNum, const ptypes_t type, const uint8_t winSize, const uint32_t timestamp) {
    pkt_set_payload(pkt, fileReadBuf, nbByteRe);
    pkt_set_seqnum(pkt, seqNum);
    pkt_set_type(pkt, type);
    pkt_set_window(pkt, winSize);
    pkt_set_timestamp(pkt, timestamp);
}

void writing_loop(const int sfd, FILE * inFile) {

    pkt_t * pktRe = pkt_new();
    const size_t sizeMaxPkt = MAX_PAYLOAD_SIZE + sizeof(pktRe) + 4;

    // Packets buffers
    const int moduloWindows = MAX_WINDOW_SIZE + 1;
    pkt_t * pktBuffer[moduloWindows];
    for (int i=0; i<moduloWindows;i++)
        pktBuffer[i] = NULL;

    char fileReadBuf[MAX_PAYLOAD_SIZE];
    char socketReadBuf[sizeMaxPkt];
    char socketWriteBuf[sizeMaxPkt];
    ssize_t nbByteRe = 0;
    ssize_t nbByteWr = 0;

    uint8_t seqnum = 0; //First seqnum must be 0
    uint8_t winSize = 1;
    uint8_t lastSeqnum = 0;

    int eof = 0;
    int allWritten = 1;
    fd_set selSo;

    while(!eof) {
        FD_ZERO(&selSo);
        FD_SET(fileno(inFile), &selSo);
        FD_SET(sfd, &selSo);

        int sct = select(sfd + 1, &selSo, NULL, NULL, NULL);

        if(sct < 0) {
            fprintf(stderr, "An error occured on select %s\n", strerror(errno));
            break;
        }
        if(FD_ISSET(fileno(inFile), &selSo)) {
            if(winSize > 0 && allWritten) {
                nbByteRe = read(fileno(inFile), fileReadBuf, MAX_PAYLOAD_SIZE);
                pkt_t * pktWr = pkt_new();
                // Packet initialization + encode
                init_pkt(pktWr, fileReadBuf, nbByteRe, seqnum, PTYPE_DATA, winSize, timestamp());
                size_t tmp = sizeMaxPkt;
                int validPkt = pkt_encode(pktWr, socketWriteBuf, &tmp);

                if(validPkt == 0) {
                    nbByteWr = write(sfd, socketWriteBuf, tmp);
                    fprintf(stderr, "SEND\n");
                    // put packet packet buffer
                    pktBuffer[pkt_get_seqnum(pktWr) % moduloWindows] = pktWr;
                    if(nbByteWr != nbByteRe + (int)sizeof(pktWr) + 4) {
                        fprintf(stderr, "Error on write from file\n");
                    }
                } else {
                    fprintf(stderr, "Packet not valid, error code : %d\n", validPkt);
                }

                winSize--;
                increment_seqnum(&seqnum);
                lastSeqnum = pkt_get_seqnum(pktWr);

                if(nbByteRe == 0) {
                    allWritten = 0;
                    lastSeqnum++;
                }
            } else {
                for(int i = 0; i < MAX_WINDOW_SIZE; i++){
                    if(pktBuffer[i] != NULL && time(NULL) > (pkt_get_timestamp(pktBuffer[i]) + 5)){
                        size_t tmps = sizeMaxPkt;
                        pkt_encode(pktBuffer[i], socketWriteBuf, &tmps);
                        int nbsdfsdfq = write(sfd, socketWriteBuf, tmps);
                        fprintf(stderr, "Resend nb byte : %d", nbsdfsdfq);
                        pkt_set_timestamp(pktBuffer[i], timestamp());
                    }
                }
            }
        }

        if(FD_ISSET(sfd, &selSo)) {
            nbByteRe = read(sfd, socketReadBuf, sizeMaxPkt); // Read data from SFD

            if(nbByteRe > 0) { // If something has been read from SFD
                pkt_decode(socketReadBuf, nbByteRe, pktRe); // Create new packet from buffer
                pkt_debug(pktRe); // Debug ACK
                winSize = pkt_get_window(pktRe);

                // Free packets
                int seq = pkt_get_seqnum(pktRe) - 1;
                int finish = 0;
                while(!finish && pktBuffer[seq % moduloWindows] != NULL) {

                    fprintf(stderr, "I free buffer number %d\n", seq % moduloWindows);
                    pkt_del(pktBuffer[seq % moduloWindows]);
                    pktBuffer[seq % moduloWindows] = NULL;
                    if(seq > 0) {
                        seq--;
                    } else {
                        seq = MAX_SEQNUM;
                    }
                    if(seq < (pkt_get_seqnum(pktRe) - 1 - winSize) || seq + winSize < pkt_get_seqnum(pktRe)) {
                        finish = 1;
                    }
                }

                if(pkt_get_seqnum(pktRe) == lastSeqnum) {
                    eof = 1;
                }
            }
        }
    }

    pkt_del(pktRe);
}


