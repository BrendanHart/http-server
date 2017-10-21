#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>

#include "get_listen_socket.h"
#include "handle_listen_socket.h"

char *ROOT_DIR;

int main(int argc, char **argv) {

    int socket_fd, port;
    char *endp, *program_name;

    assert(argv[0] && *argv[0]);
    program_name = argv[0];

    if(argc != 5) {
        fprintf(stderr, "%s: usage: %s -p <port> -d <web_root>\n", program_name, program_name);
        exit(1);
    }

    assert(argv[1] && *argv[1]);
    assert(argv[2] && *argv[2]);
    assert(argv[3] && *argv[3]);
    assert(argv[4] && *argv[4]);

    int i;
    for(i = 1; i < argc; i++) {
        if(i % 2 == 0) {
            if(strcmp(argv[i-1], "-p") == 0) {
                port = strtol(argv[i], &endp, 10);
                if(*endp != '\0') {
                    fprintf(stderr, "%s: %s is not a number.\n", program_name, argv[1]);
                    exit(1);
                }
            } else if(strcmp(argv[i-1], "-d") == 0) {
                ROOT_DIR = argv[i];
            }

        }
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
