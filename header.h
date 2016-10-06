#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

struct {
    uint32_t type : 3,
             windows : 5,
             seqNum: 8,
             lenght: 16;
    uint32_t timestamp;
} headerFrame;

/**
* Init a header of frame
* @pre: r!= NULL
* @post: record_get_type(r) == 0 && record_get_length(r) == 0
*		 && record_has_footer(r) == 0
* @return: 1 en cas d'erreur, 0 sinon
*/
int header_init(struct headerFrame * );



#endif // HEADER_H_INCLUDED
