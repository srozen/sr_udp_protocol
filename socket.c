//
// Created by Samuel Monroe on 5/10/16.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port) {
    int sockfd = 0;

    if((sockfd = socket(PF_INET6, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr, "Socket error : %s\n", gai_strerror(errno));
        return -1;
    };

    if(src_port == -1) {
        // CLIENT
        dest_addr->sin6_port = htons(dst_port);
        if(connect(sockfd, (struct sockaddr*) dest_addr, sizeof(struct sockaddr_in6)) != 0) {
            fprintf(stderr, "Client connection error : %s\n", gai_strerror(errno));
            return -1;
        } else {
            return sockfd;
        }
    } else {
        // SERVER
        source_addr->sin6_port = htons(src_port);
        if(bind(sockfd, (struct sockaddr*) source_addr, sizeof(struct sockaddr_in6)) != 0) {
            fprintf(stderr, "Server socket error : %s\n", gai_strerror(errno));
            return -1;
        } else {
            return sockfd;
        }
    }
}

const char * real_address(const char *address, struct sockaddr_in6 *rval) {
    struct addrinfo hints, *res;
    const char * error;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    if(getaddrinfo(address, NULL, &hints, &res) == 0) {
        memcpy(rval, (struct sockaddr_in6 *) res->ai_addr, sizeof(*rval));
        freeaddrinfo(res);
        return NULL;
    } else {
        error = gai_strerror(errno);
        freeaddrinfo(res);
        return error;
    };
}
