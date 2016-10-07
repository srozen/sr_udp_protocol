#ifndef CRC_H_INCLUDED
#define CRC_H_INCLUDED
#include <stdint.h>
#include "frame.h"

struct crc{
    uint32_t value;
};

/**
* Init a crc
* @pre: r!= NULL
* @post: record_get_type(r) == 0 && record_get_length(r) == 0
*		 && record_has_footer(r) == 0
* @return: 1 en cas d'erreur, 0 sinon
*/
int crc_init(struct crc *f);


/**
 * Compute a CRC
 * @pre: frame != null
 * @post: crc is computed with header & payload (if payload != NULL)
 * @return: CRC value
 */
int crc_compute(struct frame *f);

/**
 * Compare CRC
 * @pre: frame!= null
 * @return: 1 if not equals, 0 else
 */
int crc_compare(struct frame *f);

#endif // CRC_H_INCLUDED
