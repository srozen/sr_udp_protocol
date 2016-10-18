#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "functions.h"

int main(int argc, char * argv[]) {
    fprintf(stderr, "Receiver Launch : number args '%d', args '%s'\n", argc, argv[0]);

    FILE* f = NULL;
    int port = 0;
    char * address = NULL;
    char openMode[] = "w+";


    if(readArgs(argc, argv, address, &port, &f, openMode)){
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
