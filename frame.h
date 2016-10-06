#ifndef FRAME_H_INCLUDED
#define FRAME_H_INCLUDED


struct {
    struct headerFrame *header;
    char * payload;
    struct crcFrame *crc;
} frame;

/**
* Init a frame
* @pre: r!= NULL
* @post: record_get_type(r) == 0 && record_get_length(r) == 0
*		 && record_has_footer(r) == 0
* @return: 1 en cas d'erreur, 0 sinon
*/
int frame_init(struct frame * );





#endif // FRAME_H_INCLUDED
