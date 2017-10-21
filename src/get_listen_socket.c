#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

int get_listen_socket(int port) {

    struct sockaddr_in6 address;
    bzero((char *) &address, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(port);

    int socket_fd = socket(PF_INET6, SOCK_STREAM, 0);

    if(socket_fd < 0) {
        perror("socket");
        return -1;
    }

    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if(bind(socket_fd, (struct sockaddr *) &address, sizeof(address)) != 0) {
        perror("bind");
        return -1;
    }

    if(listen(socket_fd, 5) != 0) {
        perror("listen");
        return -1;
    }

    return socket_fd;

}
