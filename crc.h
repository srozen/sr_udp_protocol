#ifndef CRC_H_INCLUDED
#define CRC_H_INCLUDED

struct {
    uint32_t crc;
} crcFrame;

/**
* Init a crc
* @pre: r!= NULL
* @post: record_get_type(r) == 0 && record_get_length(r) == 0
*		 && record_has_footer(r) == 0
* @return: 1 en cas d'erreur, 0 sinon
*/
int crc_init(struct crcFrame * );

#endif // CRC_H_INCLUDED
