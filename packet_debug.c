//
// Created by srozen on 19/10/16.
//

#include "packet_debug.h"

void pkt_debug(pkt_t * pkt){
    fprintf(stderr, "###### RECEIVED PACKET DEBUG ######\n");
    fprintf(stderr, "Type : %d", pkt_get_type(pkt));
    fprintf(stderr, " - Win : %d", pkt_get_window(pkt));
    fprintf(stderr, " - Seqno : %d", pkt_get_seqnum(pkt));
    fprintf(stderr, " - Timestamp : %d\n", pkt_get_timestamp(pkt));
    fprintf(stderr, "Length is : %d\n", pkt_get_length(pkt));
    const char * bf = pkt_get_payload(pkt);
    fprintf(stderr, "Payload is : %s", bf);
    fprintf(stderr, "CRC is : %d\n", pkt_get_crc(pkt));
    fprintf(stderr, "###################################\n");
}