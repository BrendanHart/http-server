#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <assert.h>

#include "make_printable_address.h";

int handle_listen_socket(int socket_fd) {

    int client;
    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[INET6_ADDRSTRLEN + 32];

    while((client = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len) >= 0) {

        char* printable_address = make_printable_address(&client_addr,
                                                         client_addr_len,
                                                         buffer,
                                                         sizeof(buffer));

        service_client_socket(client, printable_address);

        free(printable_address);

    }

}
