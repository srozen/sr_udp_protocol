#include "functions.h"

int readArgs(int argc, char * argv[], char** address, int * port, FILE** f, char * openMode) {
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
                    "-s FILE Specify a file to send as data, or to store data in it.\n");
            exit(EXIT_FAILURE);
        }
    }
    // Assign address and port
    char* buff = argv[optind];
    buff = argv[optind];
    *address = buff;
    *port = atoi(argv[optind + 1]);
    //Open file
    if(file != NULL) {
        *f = fopen(file, openMode);
    }

    // READARGS - DEBUG
    fprintf(stderr, "Filedes  : %d\n", f!=NULL);
    fprintf(stderr, "Openmode : %s\n", openMode);
    fprintf(stderr, "Filename : %s\n", file);
    fprintf(stderr, "Address  : %s\n", *address);
    fprintf(stderr, "Portno   : %d\n", *port);

    return 1;

}
