#include "functions.h"

int read_args(int argc, char * argv[], char** address, int * port, FILE** f, char * openMode) {
    if(argc < 3){
        fprintf(stderr, USAGE);
        return EXIT_FAILURE;
    }
    int opt;
    char * file = NULL;
    while((opt = getopt(argc, argv, "f:")) != -1) {
        switch(opt) {
        case 'f':
            file = malloc(strlen(optarg) + 1);
            strcpy(file, optarg);
            break;
        default:
            fprintf(stderr, USAGE);
            return EXIT_FAILURE;
        }
    }
    // Assign address and port
    if(argv[optind] == NULL || argv[optind + 1] == NULL){
        fprintf(stderr, USAGE);
        return EXIT_FAILURE;
    }
    char* buff = argv[optind];
    *address = buff;
    *port = atoi(argv[optind + 1]);
    //Open file
    if(file != NULL) {
        *f = fopen(file, openMode);
    }
    free(file);
    return EXIT_SUCCESS;
}

uint32_t timestamp() {
    struct timeval now;
    gettimeofday(&now, NULL);
    long mill = (now.tv_sec*1000 + now.tv_usec/1000);
    return (uint32_t)(mill%1000000000);
}

void recompute_timeout(uint32_t timeout, uint32_t timestamp, uint32_t timenow){
  fprintf(stderr, "Actual timeout : %d\n", timeout);
  fprintf(stderr, "Should be actualized to : %d\n", (timestamp - timenow) + 100);
  //&timeout = (timestamp - timenow) + 100;
}

void release_all_buffers(pkt_t ** pktBuf, int pktBufSize){
  for(int i = 0; i < pktBufSize; i++){
    if(pktBuf[i] != NULL)
      pkt_del(pktBuf[i]);
  }
}
