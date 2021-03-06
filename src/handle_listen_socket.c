#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <assert.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "make_printable_address.h"
#include "service_client_socket.h"


typedef struct thread_control_block {
    int client;
    struct sockaddr_in6 client_address;
    socklen_t client_address_size;
    const char *root_dir;
    SSL_CTX *ctx;
} thread_control_block_t;

SSL_CTX *create_context() {
    // Create and return the SSL context
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    method = SSLv23_server_method();
    ctx = SSL_CTX_new(method);
    if(!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    // Configure the context using the cert.pem and key.pem files
    if(SSL_CTX_use_certificate_file(ctx, "./cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if(SSL_CTX_use_PrivateKey_file(ctx, "./key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

static void * setup_client_thread(void *data) {
    thread_control_block_t *tcb_p = (thread_control_block_t *) data;

    assert(tcb_p->client_address_size == sizeof(tcb_p->client_address));

    char buffer[INET6_ADDRSTRLEN + 32];
    char *printable_address;

    // Make the address nice for printing (format: IP port PORT)
    printable_address = make_printable_address(&(tcb_p->client_address),
                                    tcb_p->client_address_size,
                                    buffer,
                                    sizeof(buffer));
    // Create an SSL socket from the original socket
    SSL *ssl;
    ssl = SSL_new(tcb_p->ctx);
    SSL_set_fd(ssl, tcb_p->client);

    // Handle the client
    service_client_socket(ssl, printable_address, tcb_p->root_dir);

    // Free any data and exit thread
    close(tcb_p->client);
    free(printable_address);
    free(data);
    pthread_exit(0);
}

int handle_listen_socket(int socket_fd, const char *root_dir) {

    // Setup the SSL context
    SSL_CTX *ctx;
    ctx = create_context();
    configure_context(ctx);

    printf("Server is now listening...\n");

    // Constantly listen for connections
    while(1) {
        thread_control_block_t *tcb_p = malloc(sizeof(thread_control_block_t));
        if(tcb_p == 0) {
            perror("malloc");
            return -1;
        }
        tcb_p->client_address_size = sizeof(tcb_p->client_address);
        tcb_p->ctx = ctx;

        int number;
        if((number = accept(socket_fd, (struct sockaddr *) &tcb_p->client_address, &(tcb_p->client_address_size))) >= 0) {
            // We have a connection, so create a new thread to handle the connection
            tcb_p->client = number;
            tcb_p->root_dir = root_dir;
            pthread_t thread;
            if(pthread_create(&thread, 0, setup_client_thread, (void *) tcb_p) != 0) {
                perror("pthread_create");
                return -1;
            }
            int error = pthread_detach(thread);
            if(error != 0) {
                fprintf(stderr, "pthread_detach failed: %s\n", strerror(error));
            }
        } else {
            perror("accept");
        }

    }

    printf("Server is closing...\n");

}
