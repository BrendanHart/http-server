#include <stdio.h>

#include <sys/types.h>
#include <sys.socket.h>

#include <assert.h>

int handle_listen_socket(int socket_fd) {

    int client;
    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[INET6_ADDRSTRLEN + 32];

    while((client = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len) >= 0) {

        client_socket = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len);

    }

}
