//
// Created by srozen on 10.10.16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "packet_interface.h"

int main() {
    pkt_t* packet = pkt_new();
    int buffSize = 20;
    char * buff = malloc(sizeof(char) * buffSize);
    char string[20] = "Dude";
    memcpy(buff, string, buffSize);


    pkt_set_type(packet, PTYPE_DATA);
    pkt_set_window(packet, 2);
    pkt_set_seqnum(packet, 200);
    pkt_set_timestamp(packet, 2000);
    pkt_set_length(packet, (uint16_t)buffSize);
    pkt_set_payload(packet, buff, (uint16_t)buffSize);

    uint32_t crc = compute_crc(packet);
    pkt_set_crc(packet, crc);

    fprintf(stderr, "Type is : %d\n", pkt_get_type(packet));
    fprintf(stderr, "Window is : %d\n", pkt_get_window(packet));
    fprintf(stderr, "Seqnum is : %d\n", pkt_get_seqnum(packet));
    fprintf(stderr, "Timestamp is : %d\n", pkt_get_timestamp(packet));
    fprintf(stderr, "Length is : %d\n", pkt_get_length(packet));
    fprintf(stderr, "CRC is : %d\n", pkt_get_crc(packet));
    const char * copyBuff = pkt_get_payload(packet);
    fprintf(stderr, "Payload is : %s\n", copyBuff);

    fprintf(stderr, "------ CODED - DECODED -------\n");

    size_t sbl = 1024;
    char * sb = malloc(sbl * sizeof(char));

    pkt_encode(packet, sb, &sbl);

    // Write to file
    FILE* f = fopen("frame", "w+");
    fwrite(sb, sbl, 1, f);

    pkt_t* packut = pkt_new();
    pkt_decode(sb, sbl, packut);

    fprintf(stderr, "Type is : %d\n", pkt_get_type(packut));
    fprintf(stderr, "Window is : %d\n", pkt_get_window(packut));
    fprintf(stderr, "Seqnum is : %d\n", pkt_get_seqnum(packut));
    fprintf(stderr, "Timestamp is : %d\n", pkt_get_timestamp(packut));
    fprintf(stderr, "Length is : %d\n", pkt_get_length(packut));
    fprintf(stderr, "CRC is : %d\n", pkt_get_crc(packut));
    const char * bf = pkt_get_payload(packut);
    fprintf(stderr, "Payload is : %s\n", bf);
    free(buff);
    pkt_del(packet);
    return 0;
}
