#include "http_header.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int send_all(const int client_socket, char* buffer, size_t size) {
    while(size > 0) { 
        size -= write(client_socket, buffer, size);
    }
    return size;
}

static int read_local_file(const char* root_dir, const char* file, char** buffer, size_t* size) {
    // Compute full url
    char full_url[MAX_LINE_SIZE];
    strcpy(full_url, root_dir);
    strcat(full_url, file);

    FILE *fp = fopen(full_url, "r");


    *size = 0;

    if(fp == NULL) {
        return -1;
    } else {
        // Read file
        if(fseek(fp, 0L, SEEK_END) == 0) {
            long bufsize = ftell(fp);
            if(bufsize == -1) {
                // ERROR
            }

            *buffer = malloc(sizeof(char) * bufsize);
            if(fseek(fp, 0L, SEEK_SET) != 0) {
                // ERROR
            }
            *size = fread(*buffer, sizeof(char), bufsize, fp);
            if(ferror(fp) != 0) {
                // ERROR
            }
        }
        fclose(fp);

    }
    return 0;

}

static void send_local_file(const int client_socket, char *buffer, const size_t size) {
    if(buffer == NULL) {
        send_all(client_socket, "404 NOT FOUND", 13);
        return;
    }

    send_all(client_socket, buffer, size);
}

static int send_response_header(const int client_socket, http_header *header, const size_t content_size) {

    send_all(client_socket, header->version, strlen(header->version));

    if(header->method == UNSUPPORTED) {
        send_all(client_socket, " 405 Method Not Allowed\r\n", strlen(" 405 Method Not Allowed\r\n"));
        send_all(client_socket, "Allowed: GET, HEAD\r\n", strlen("Allowed: GET, HEAD\r\n"));
        send_all(client_socket, "\r\n", 2);
        return 0;
    }

    char size_str[256];
    snprintf(size_str, sizeof(size_str), "%zu", content_size);

    char content_type[1024];
    strcpy(content_type, "Content-Type: ");
    int len = strlen(header->resource);
    if(strcmp(&header->resource[len-4], ".pdf") == 0) {
        strcat(content_type, "application/pdf");
    } else if(strcmp(&header->resource[len-4], ".png") == 0) {
        strcat(content_type, "image/png");
    } else if(strcmp(&header->resource[len-4], ".jpg") == 0 
            || strcmp(&header->resource[len-5], ".jpeg") == 0) {
        strcat(content_type, "image/jpeg");
    } else if(strcmp(&header->resource[len-4], ".gif") == 0) {
        strcat(content_type, "image/gif");
    } else if(strcmp(&header->resource[len-4], ".xml") == 0) {
        strcat(content_type, "application/xml");
    } else if(strcmp(&header->resource[len-4], ".css") == 0) {
        strcat(content_type, "text/css");
    } else if(strcmp(&header->resource[len-5], ".html") == 0) {
        strcat(content_type, "text/html");
        strcat(content_type, ";charset=utf-8");
    }

    if(header->status == 200) {
        send_all(client_socket, " 200 OK\r\n", strlen(" 200 OK\r\n"));
        send_all(client_socket, content_type, strlen(content_type));
        send_all(client_socket, "\r\n", 2);
        send_all(client_socket, "Content-Length: ", 16);
        send_all(client_socket, size_str, strlen(size_str));
        send_all(client_socket, "\r\n", 2);
    } else if (header->status == 404) {
        strcpy(content_type, "Content-Type: text/html;charset=utf-8");
        send_all(client_socket, " 404 NOT FOUND\r\n", strlen(" 404 NOT FOUND\r\n"));
        send_all(client_socket, content_type, strlen(content_type));
        send_all(client_socket, "\r\n", 2);
    }
    send_all(client_socket, "\r\n", 2);
    return 1;
}


int send_http_response(const int client_socket, 
                    const char *root_dir,
                    http_header* header) {

    char *source;
    size_t size_read = 0;
    int result = read_local_file(root_dir, header->resource, &source, &size_read);

    if(result == -1) {
        source = NULL;
        header->status = 404;
        size_read = 13;
    }


    int send_body = send_response_header(client_socket, header, size_read);

    if(send_body == 1) {
        send_local_file(client_socket, source, size_read);
    }

    free(source);

    return 0;

}
        
