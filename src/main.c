#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <assert.h>

#include "get_listen_socket.h"
#include "handle_listen_socket.h"

int main(int argc, char **argv) {

    int socket_fd, port;
    char *endp, *program_name, *root_dir;

    assert(argv[0] && *argv[0]);
    program_name = argv[0];

    // Check we have the disired number of arguments
    if(argc != 5) {
        fprintf(stderr, "%s: usage: %s -p <port> -d <web_root>\n", program_name, program_name);
        exit(1);
    }

    // Make sure they all exist
    assert(argv[1] && *argv[1]);
    assert(argv[2] && *argv[2]);
    assert(argv[3] && *argv[3]);
    assert(argv[4] && *argv[4]);

    // Process -p and -d flags, in any order
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
                root_dir = argv[i];
            }

        }
    }

    // Check the port is valid and can be listened on without sudo
    if(port > 65535 || port < 1024) {
        fprintf(stderr, "%s: port number should in the range 1024 <= port < 65535.\n", program_name);
        exit(1);
    }

    // Initialise SSL
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    SSL_library_init();

    // Get the listen socket
    socket_fd = get_listen_socket(port);
    if(socket_fd < 0) {
        fprintf(stderr, "%s: error getting listen socket.", program_name);
        exit(1);
    }

    // Handle bad pipe signal
    signal(SIGPIPE, SIG_IGN);

    // Handle the listen socket
    if(handle_listen_socket(socket_fd, root_dir) != 0) {
        fprintf(stderr, "%s: error processing listen socket.", program_name);
        exit(1);
    }

    // Cleanup after server
    ERR_free_strings();
    EVP_cleanup();

    return 0;

}

