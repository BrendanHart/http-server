#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

enum request_method {
    GET, HEAD, UNSUPPORTED
};

#define MAX_LINE_SIZE 1024

typedef struct http_header {
    enum request_method method;
    char* user_agent;
    char* resource;
    char* version;
    int status;
} http_header;

#endif /* HTTP_HEADER_H */
