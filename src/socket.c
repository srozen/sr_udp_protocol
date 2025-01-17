#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "socket.h"

#define FRAME_SIZE 1024


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
    int errcode;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE | AI_V4MAPPED;
    errcode = getaddrinfo(address, NULL, &hints, &res);

    if(errcode != 0) {
        return "Getaddrinfo fail";
    }
    if(res->ai_family != AF_INET6) {
        return "Not ipv6 address";
    }

    memcpy(rval, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    return NULL;
}

int wait_for_client(int sfd) {
    char msg[FRAME_SIZE];
    struct sockaddr_in6 client;
    socklen_t len = sizeof(client);
    memset(&client, 0, sizeof client);

    if(recvfrom(sfd, msg, FRAME_SIZE, MSG_PEEK, (struct sockaddr *)&client, &len) == -1) {
        fprintf(stderr, "Receive error occurred : %s\n", gai_strerror(errno));
        return -1;
    }

    if(connect(sfd, (struct sockaddr*) &client, sizeof(struct sockaddr_in6)) != 0) {
        fprintf(stderr, "Server connection to client error : %s\n", gai_strerror(errno));
        return -1;
    }
    return 0;
}
