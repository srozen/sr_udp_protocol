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

void init_pkt(pkt_t * pkt, char * fileReadBuf, const uint16_t nbByteRe, const uint8_t seqNum, const ptypes_t type, const uint8_t winSize, const uint32_t timestamp){
    pkt_set_payload(pkt, fileReadBuf, nbByteRe);
    pkt_set_seqnum(pkt, seqNum);
    pkt_set_type(pkt, type);
    pkt_set_window(pkt, winSize);
    pkt_set_timestamp(pkt, timestamp);
}

void writing_loop(const int sfd, FILE * inFile) {
    pkt_t * pktWr = pkt_new();
    pkt_t * pktRe = pkt_new();
    const size_t sizeMaxPkt = MAX_PAYLOAD_SIZE + sizeof(pktWr) + 4;

    // Packets buffers
    const int moduloWindows = MAX_WINDOW_SIZE + 1;
    pkt_t * pktBuffer[moduloWindows];

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

    while(!eof){
        FD_ZERO(&selSo);
        FD_SET(fileno(inFile), &selSo);
        FD_SET(sfd, &selSo);

        int sct = select(sfd+1, &selSo, NULL, NULL, NULL);

        if(sct < 0){
            fprintf(stderr, "An error occured on select %s\n", strerror(errno));
            break;
        }

        if(winSize > 0 && allWritten){
            if(FD_ISSET(fileno(inFile), &selSo)){
                nbByteRe = read(fileno(inFile), fileReadBuf, MAX_PAYLOAD_SIZE);

                // Packet initialization + encode
                init_pkt(pktWr, fileReadBuf, nbByteRe, seqnum, PTYPE_DATA, winSize, timestamp());
                size_t tmp = sizeMaxPkt;
                int validPkt = pkt_encode(pktWr, socketWriteBuf, &tmp);

                if(validPkt == 0){
                    nbByteWr = write(sfd, socketWriteBuf, tmp);
                    pktBuffer[pkt_get_seqnum(pktWr) % moduloWindows] = pktWr;
                    if(nbByteWr != nbByteRe + (int)sizeof(pktWr) + 4){
                        fprintf(stderr, "Error on write from file\n");
                    }
                } else {
                    fprintf(stderr, "Packet not valid, error code : %d\n", validPkt);
                }

                winSize--;
                increment_seqnum(&seqnum);
                lastSeqnum = pkt_get_seqnum(pktWr);

                if(nbByteRe == 0){
                    allWritten = 0;
                    lastSeqnum++;
                }
            }
        }
        if(FD_ISSET(sfd, &selSo)){
            nbByteRe = read(sfd, socketReadBuf, sizeMaxPkt); // Read data from SFD
            if(nbByteRe > 0){ // If something has been read from SFD
                pkt_decode(socketReadBuf, nbByteRe, pktRe); // Create new packet from buffer
                pkt_debug(pktRe); // Debug ACK
                winSize = pkt_get_window(pktRe);

                // Free packets
                int seq = pkt_get_seqnum(pktRe);
                int count = 0;// DEBUG
                for(int i = MAX_WINDOW_SIZE; i > 0; i--){
                    pktBuffer[seq % moduloWindows] = NULL;
                    seq = decrement_seqnum(seq);
                    count ++;//DEBUG
                }
                fprintf(stderr, "Packet been free : %d\n", count); //DEBUG

                if(pkt_get_seqnum(pktRe) == lastSeqnum){
                    eof = 1;
                }
            }
        }
    }
}


