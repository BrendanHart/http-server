#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "get_listen_socket.h"
#include "handle_listen_socket.h"

int main(int argc, char **argv) {

    int port;
    char *endp, *program_name;

    assert(argv[0] && *argv[0]);
    program_name = argv[0];

    if(argc != 2) {
        fprintf(stderr, "%s: usage: %s <port>\n", program_name, program_name);
        exit(1);
    }

    assert(argv[1] && *argv[1]);

    port = strtol(argv[1], &endp, 10);

    if(*endp != '\0') {
        fprintf(stderr, "%s: %s is not a number.\n", program_name, argv[1]);
        exit(1);
    }

    if(port > 65535) {
        fprintf(stderr, "%s: port number should be less than 65535.\n", program_name);
        exit(1);
    }

    socket_fd = get_listen_socket(port);
    if(socket_fd < 0) {
        fprintf(stderr, "%s: error getting listen socket.", program_name);
        exit(1);
    }

    if(handle_listen_socket(socket_fd) != 0) {
        fprintf(stderr, "%s: error processing listen socket.", program_name);
        exit(1);
    }

    return 0;

}
