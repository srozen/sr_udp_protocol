#ifndef __PACKET_INTERFACE_H_
#define __PACKET_INTERFACE_H_

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintx_t */
#include <zlib.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <memory.h>
#include <stdio.h>

/* Raccourci pour struct pkt */
typedef struct pkt pkt_t;

/* Types de paquets */
typedef enum {
    PTYPE_DATA = 1,
    PTYPE_ACK = 2,
} ptypes_t;

/* Taille maximale permise pour le payload */
#define MAX_PAYLOAD_SIZE 512
/* Taille maximale de Window */
#define MAX_WINDOW_SIZE 31
/* Taille maximale du numéro de séquence */
#define MAX_SEQNUM 255
/* Taille du header */
#define HEADER_SIZE 12

uint32_t  compute_crc(const pkt_t *pkt);

/* Valeur de retours des fonctions */
typedef enum {
    PKT_OK = 0,     /* Le paquet a été traité avec succÃ¨s */
    E_TYPE,         /* Erreur liée au champs Type */
    E_LENGTH,       /* Erreur liée au champs Length  */
    E_CRC,          /* CRC invalide */
    E_WINDOW,       /* Erreur liée au champs Window */
    E_SEQNUM,       /* Numéro de séquence invalide */
    E_NOMEM,        /* Pas assez de mémoire */
    E_NOHEADER,     /* Le paquet n'a pas de header (trop court) */
    E_UNCONSISTENT, /* Le paquet est incohérent */
} pkt_status_code;

/* Alloue et initialise une struct pkt
 * @return: NULL en cas d'erreur */
pkt_t* pkt_new();
/* Libère le pointeur vers la struct pkt, ainsi que toutes les
 * ressources associées
 */
void pkt_del(pkt_t*);

/*
 * Décode des données reçues et crée une nouvelle structure pkt.
 * Le paquet reçu est en network byte-order.
 * La fonction vérifie que:
 * - Le CRC32 des données reçues est le mÃªme que celui décodé Ã  la fin
 *   du flux de données
 * - Le type du paquet est valide
 * - La longeur du paquet est valide et cohérente avec le nombre d'octets
 *   reçus.
 *
 * @data: L'ensemble d'octets constituant le paquet reçu
 * @len: Le nombre de bytes reçus
 * @pkt: Une struct pkt valide
 * @post: pkt est la représentation du paquet reçu
 *
 * @return: Un code indiquant si l'opération a réussi ou représentant
 *         l'erreur rencontrée.
 */
pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt);

/*
 * Encode une struct pkt dans un buffer, prÃªt Ã  Ãªtre envoyé sur le réseau
 * (c-Ã -d en network byte-order), incluant le CRC32 du header et payload.
 *
 * @pkt: La structure Ã  encoder
 * @buf: Le buffer dans lequel la structure sera encodée
 * @len: La taille disponible dans le buffer
 * @len-POST: Le nombre de d'octets écrit dans le buffer
 * @return: Un code indiquant si l'opération a réussi ou E_NOMEM si
 *         le buffer est trop petit.
 */
pkt_status_code pkt_encode(const pkt_t*, char *buf, size_t *len);

/* Accesseurs pour les champs toujours présents du paquet.
 * Les valeurs renvoyées sont toutes dans l'endianness native
 * de la machine!
 */
ptypes_t pkt_get_type(const pkt_t*);
uint8_t  pkt_get_window(const pkt_t*);
uint8_t  pkt_get_seqnum(const pkt_t*);
uint16_t pkt_get_length(const pkt_t*);
uint32_t pkt_get_timestamp(const pkt_t*);
uint32_t pkt_get_crc(const pkt_t*);
/* Renvoie un pointeur vers le payload du paquet, ou NULL s'il n'y
 * en a pas.
 */
const char* pkt_get_payload(const pkt_t*);

/* Setters pour les champs obligatoires du paquet. Si les valeurs
 * fournies ne sont pas dans les limites acceptables, les fonctions
 * doivent renvoyer un code d'erreur adapté.
 * Les valeurs fournies sont dans l'endianness native de la machine!
 */
pkt_status_code pkt_set_type(pkt_t*, const ptypes_t type);
pkt_status_code pkt_set_window(pkt_t*, const uint8_t window);
pkt_status_code pkt_set_seqnum(pkt_t*, const uint8_t seqnum);
pkt_status_code pkt_set_length(pkt_t*, const uint16_t length);
pkt_status_code pkt_set_timestamp(pkt_t*, const uint32_t crc);
pkt_status_code pkt_set_crc(pkt_t*, const uint32_t crc);
/* Défini la valeur du champs payload du paquet.
 * @data: Une succession d'octets représentants le payload
 * @length: Le nombre d'octets composant le payload
 * @POST: pkt_get_length(pkt) == length */
pkt_status_code pkt_set_payload(pkt_t*, const char *data, const uint16_t length);

/* Increment the seqnum for the sender.
 * If the seqnum reached 255, it must be reset to 0 for the next packet.
 */
void increment_seqnum(uint8_t * seqnum);

/*
 * Decrement seqnum
 */
void decrement_seqnum(uint8_t * seqnum);



#endif  /* __PACKET_INTERFACE_H_ */

