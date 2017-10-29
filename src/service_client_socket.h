#ifndef SERVICE_CLIENT_SOCKET_H
#define SERVICE_CLIENT_SOCKET_H

#include <openssl/ssl.h>
int service_client_socket(SSL* ssl, const char *const printable_address, const char *root_dir);

#endif /* SERVICE_CLIENT_SOCKET_H */
