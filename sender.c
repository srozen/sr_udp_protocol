#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "functions.h"

int main(int argc, char * argv[]){
    fprintf(stderr,"Sender Launch : number args '%d', args '%s'\n", argc, argv[0]);

    int inputFile = STDIN_FILENO;
    int * port = NULL;
    char * address = NULL;

    readArgs(argc, argv, address, port,&inputFile);

    return (EXIT_SUCCESS);
}
