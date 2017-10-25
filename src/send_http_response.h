#ifndef SEND_HTTP_RESPONSE_H
#define SEND_HTTP_RESPONSE_H 

#include "http_header.h"
#include <openssl/ssl.h>

int send_http_response(SSL* ssl, const char *root_dir, http_header *header);

#endif /* SEND_HTTP_RESPONSE_H */
