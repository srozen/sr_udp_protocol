#include "crc.h"
#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>

int crc_compare(struct frame *f){
    if(f->crc->value != crc_compute(f)){
        fprintf(stderr, "Error on CRC comparison occurred.\n"); // DEBUG
        return EXIT_FAILURE;
    }
    else
        return EXIT_SUCCESS;
}

// TODO : Must use CRC32 to compute CRC, check Zlib
int crc_compute(struct frame *f){

}