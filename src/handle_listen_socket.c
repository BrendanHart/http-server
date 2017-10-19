#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#include <assert.h>

#include "make_printable_address.h"
#include "service_client_socket.h"


typedef struct thread_control_block {
    int client;
    struct sockaddr_in6 client_address;
    socklen_t client_address_size;
} thread_control_block_t;

static void * setup_client_thread(void *data) {
    thread_control_block_t *tcb_p = (thread_control_block_t *) data;

    assert(tcb_p->client_address_size == sizeof(tcb_p->client_address));

    char buffer[INET6_ADDRSTRLEN + 32];
    char *printable_address;
    printable_address = make_printable_address(&(tcb_p->client_address),
                                    tcb_p->client_address_size,
                                    buffer,
                                    sizeof(buffer));
    service_client_socket(tcb_p->client, printable_address);
    free(printable_address);
    free(data);
    pthread_exit(0);
}

int handle_listen_socket(int socket_fd) {

    while(1) {
        thread_control_block_t *tcb_p = malloc(sizeof(thread_control_block_t));
        if(tcb_p == 0) {
            perror("malloc");
            return -1;
        }
        tcb_p->client_address_size = sizeof(tcb_p->client_address);

        int number;
        if((number = accept(socket_fd, (struct sockaddr *) &tcb_p->client_address, &(tcb_p->client_address_size))) >= 0) {
            tcb_p->client = number;
            pthread_t thread;
            if(pthread_create(&thread, 0, setup_client_thread, (void *) tcb_p) != 0) {
                perror("pthread_create");
                return -1;
            }
        } else {
            perror("accept");
        }

    }

}
