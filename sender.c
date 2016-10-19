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

    // Packets relevant variables
    uint8_t seqnum = 0; //First seqnum must be 0

    // TODO in loop depending the file and wait ack

    ssize_t nbByteRe = 0;
    ssize_t nbByteWr = 0;

    int eof = 0;
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

        // working on STDIN for the moment
        if(FD_ISSET(fileno(inFile), &selSo)){
            nbByteRe = read(fileno(inFile), bufRe, MAX_PAYLOAD_SIZE);

            // Packet initialization
            pkt_set_payload(pktWr,bufRe, nbByteRe);
            pkt_set_seqnum(pktWr, seqnum);
            pkt_encode(pktWr, bufWr, &sizeMaxPkt);

            nbByteWr = write(sfd, bufWr, sizeMaxPkt);
            if(nbByteWr != (int)nbByteRe + (int)sizeof(pktWr) + (int)sizeof(pkt_get_crc(pktWr))) // total size
                fprintf(stderr, "Error occured on write from stdin\n");
            if(nbByteWr == 0){
                eof = 1;
            }
            fprintf(stderr, "nb byte Send %d\n", (int)nbByteWr);
            fprintf(stderr, "nb byte Read %d\n", (int)nbByteRe);
            increment_seqnum(&seqnum);
        }

    }
    /*
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
