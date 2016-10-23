#include "functions.h"

int read_args(int argc, char * argv[], char** address, int * port, FILE** f, char * openMode) {
    int opt;
    char * file = NULL;
    while((opt = getopt(argc, argv, "f:")) != -1) {
        switch(opt) {
        case 'f':
            file = malloc(strlen(optarg) + 1);
            strcpy(file, optarg);
            break;
        default:
            fprintf(stderr, "Usage:\n"
                    "-f FILE    Specify a file to send as data, or to store data in it (optional).\n"
                    "HOSTNAME   IPv6 address or hostname to reach.\n"
                    "PORTNUM    Port number.\n");
            return EXIT_FAILURE;
        }
    }
    // Assign address and port
    if(argv[optind] == NULL || argv[optind + 1] == NULL){
        fprintf(stderr, "Usage:\n"
                "-f FILE    Specify a file to send as data, or to store data in it (optional).\n"
                "HOSTNAME   IPv6 address or hostname to reach.\n"
                "PORTNUM    Port number.\n");
        return EXIT_FAILURE;
    }
    char* buff = argv[optind];
    *address = buff;
    *port = atoi(argv[optind + 1]);
    //Open file
    if(file != NULL) {
        *f = fopen(file, openMode);
    }
    return EXIT_SUCCESS;
}

uint32_t timestamp() {
    struct timeval now;
    gettimeofday(&now, NULL);
    long mill = (now.tv_sec*1000 + now.tv_usec/1000);
    return (uint32_t)(mill%1000000000);
}
