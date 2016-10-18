#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "functions.h"

int main(int argc, char * argv[]){
    fprintf(stderr,"Sender Launch : number args '%d', args '%s'\n", argc, argv[0]);

    //int inputFile = STDIN_FILENO;
    FILE * f = NULL;
    int port = 0;
    char * address = NULL;
    char openMode[] = "r";

    readArgs(argc, argv, address, &port, f, openMode);

    return (EXIT_SUCCESS);
}
