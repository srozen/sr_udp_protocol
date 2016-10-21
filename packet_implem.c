#include "packet_interface.h"

/* Your code will be inserted here */

struct __attribute__((__packed__)) pkt {
    struct {
        uint8_t window : 5,
                type: 3;
        uint8_t seqnum;
        uint16_t length;
        uint32_t timestamp;
    } header;
    char *payload;
    uint32_t crc;
};

/* Extra code */
uint32_t compute_crc(const pkt_t *pkt) {
    uLong crc = crc32(0L, Z_NULL, 0);
    if(pkt->header.length > 0) {
        crc = crc32(crc, (Bytef *) pkt, sizeof(pkt->header));
        crc = crc32(crc, (Bytef *) pkt->payload, sizeof(char) * pkt_get_length(pkt));
    } else {
        crc = crc32(crc, (Bytef *) pkt, sizeof(pkt->header));
    }
    return (uint32_t) crc;
}

/* Your code will be inserted here */

pkt_t *pkt_new() {
    struct pkt *packet;
    if((packet = malloc(sizeof(struct pkt))) == NULL) {
        return NULL;
    }
    packet->header.length = 0;
    packet->header.window = 0;
    packet->header.seqnum = 0;
    packet->header.timestamp = 0;
    packet->crc = 0;
    packet->payload = NULL;
    return packet;
}

void pkt_del(pkt_t *pkt) {
    if(pkt != NULL) {
        if(pkt->header.length != 0) {
            free(pkt->payload);
        }
        free(pkt);
    }
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt) {

    if(len <= sizeof(pkt->header)) {
        return E_NOHEADER;
    }

    size_t readBytes = 0;

    memcpy(&pkt->header, data, sizeof(pkt->header));
    readBytes += sizeof(pkt->header);
    pkt_set_payload(pkt, data + readBytes, pkt_get_length(pkt));
    readBytes += pkt_get_length(pkt);
    memcpy(&pkt->crc, data + readBytes, sizeof(pkt->crc));
    readBytes += sizeof(pkt->crc);

    if(readBytes > len) {
        return E_NOMEM;
    }
    if(pkt_get_crc(pkt) != compute_crc(pkt)) {
        return E_CRC;
    }
    if(pkt_get_type(pkt) != PTYPE_ACK && pkt_get_type(pkt) != PTYPE_DATA) {
        return E_TYPE;
    }
    if(pkt_get_length(pkt) > MAX_PAYLOAD_SIZE) {
        return E_LENGTH;
    }
    return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t *pkt, char *buf, size_t *len) {
    size_t writtenBytes = 0;
    uint32_t crc = htonl(compute_crc(pkt));
    memcpy(buf, &pkt->header, sizeof(pkt->header));
    writtenBytes += sizeof(pkt->header) / sizeof(char);
    memcpy(buf + writtenBytes, pkt->payload, sizeof(char) * pkt_get_length(pkt));
    writtenBytes += sizeof(char) * pkt_get_length(pkt);
    memcpy(buf + writtenBytes, &crc, sizeof(pkt->crc));
    writtenBytes += sizeof(pkt->crc) / sizeof(char);
    memcpy(len, &writtenBytes, sizeof(writtenBytes));

    if(&writtenBytes > len) {
        return E_NOMEM;
    }
    return PKT_OK;
}

ptypes_t pkt_get_type(const pkt_t *pkt) {
    return (ptypes_t) pkt->header.type;
}

uint8_t pkt_get_window(const pkt_t *pkt) {
    return (uint8_t) pkt->header.window;
}

uint8_t pkt_get_seqnum(const pkt_t *pkt) {
    return pkt->header.seqnum;
}

uint16_t pkt_get_length(const pkt_t *pkt) {
    return ntohs(pkt->header.length);
}

uint32_t pkt_get_timestamp(const pkt_t *pkt) {
    return pkt->header.timestamp;
}

uint32_t pkt_get_crc(const pkt_t *pkt) {
    return ntohl(pkt->crc);
}

const char *pkt_get_payload(const pkt_t *pkt) {
    if(pkt->header.length == 0) {
        return NULL;
    }
    return pkt->payload;
}


pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type) {
    if((type != PTYPE_ACK) && (type != PTYPE_DATA)) {
        return E_TYPE;
    }
    pkt->header.type = type;
    return PKT_OK;

}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window) {
    if((window > MAX_WINDOW_SIZE)) {
        return E_WINDOW;
    }
    pkt->header.window = window;
    return PKT_OK;
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum) {
    pkt->header.seqnum = seqnum;
    return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length) {
    if(length > MAX_PAYLOAD_SIZE) {
        return E_LENGTH;
    }
    pkt->header.length = htons(length);
    return PKT_OK;
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp) {
    pkt->header.timestamp = timestamp;
    return PKT_OK;
}

pkt_status_code pkt_set_crc(pkt_t *pkt, const uint32_t crc) {
    pkt->crc = htonl(crc);
    return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt, const char *data, const uint16_t length) {
    if(pkt_set_length(pkt, length) != PKT_OK) {
        return E_LENGTH;
    }
    char *payload = malloc(sizeof(char) * length);
    memset(payload, 0, length);
    memcpy(payload, data, sizeof(char) * length);
    pkt->payload = payload;
    return PKT_OK;
}

void increment_seqnum(uint8_t * seqnum) {
    if(*seqnum == MAX_SEQNUM) {
        *seqnum = 0;
    } else {
        *seqnum += 1;
    }
}

int decrement_seqnum(int seqnum){
    if(seqnum == 0){
        return 255;
    } else {
        return seqnum -1;
    }
}

uint32_t timestamp() {
    return (uint32_t)time(NULL);
}

