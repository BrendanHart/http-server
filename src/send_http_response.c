#include "http_header.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

static int send_all(const int client_socket, char* buffer, size_t size) {
    while(size > 0) { 
        int written = write(client_socket, buffer, size);
        if(written < 0)
            return -1;
        size -= written;
    }
    return size;
}

static int read_local_file(const char* root_dir, const char* file, char** buffer, size_t* size) {
    // Compute full url
    if(file == NULL)
        return -1;

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

    char size_str[256];
    snprintf(size_str, sizeof(size_str), "%zu", content_size);

    char content_type[1024];
    strcpy(content_type, "Content-Type: ");
    if(header->resource) {
        int len = strlen(header->resource);
        if(len >= 4 && strcmp(&header->resource[len-4], ".pdf") == 0) {
            strcat(content_type, "application/pdf");
        } else if(len >= 4 && strcmp(&header->resource[len-4], ".png") == 0) {
            strcat(content_type, "image/png");
        } else if((len >= 4 && strcmp(&header->resource[len-4], ".jpg") == 0)
                || (len >= 5 && strcmp(&header->resource[len-5], ".jpeg") == 0)) {
            strcat(content_type, "image/jpeg");
        } else if(len >= 4 && strcmp(&header->resource[len-4], ".gif") == 0) {
            strcat(content_type, "image/gif");
        } else if(len >= 4 && strcmp(&header->resource[len-4], ".xml") == 0) {
            strcat(content_type, "application/xml");
        } else if(len >= 4 && strcmp(&header->resource[len-4], ".css") == 0) {
            strcat(content_type, "text/css");
        } else if(len >= 5 && strcmp(&header->resource[len-5], ".html") == 0) {
            strcat(content_type, "text/html");
            strcat(content_type, ";charset=utf-8");
        }
    }

    if(header->version)
        send_all(client_socket, header->version, strlen(header->version));
    else
        send_all(client_socket, "HTTP/1.1", strlen("HTTP/1.1"));

    char * bad_version = " 505 HTTP Version Not Supported";
    char * not_implemented = " 501 Not Implemented";

    if(header->status == 400) {
        if(send_all(client_socket, " 400 Bad Request", strlen(" 400 Bad Request")) < 0
        || send_all(client_socket, "\r\n", 2) < 0
        || send_all(client_socket, "\r\n", 2) < 0)
            return -1;
        return 0;
    } else if(header->status == 505) {
        if(send_all(client_socket, bad_version, strlen(bad_version)) < 0
        || send_all(client_socket, "\r\n", 2) < 0
        || send_all(client_socket, "\r\n", 2) < 0)
            return -1;
        return 0;
    } if(header->status == 200) {
        if(send_all(client_socket, " 200 OK\r\n", strlen(" 200 OK\r\n")) < 0
        || send_all(client_socket, content_type, strlen(content_type)) < 0
        || send_all(client_socket, "\r\n", 2) < 0
        || send_all(client_socket, "Content-Length: ", 16) < 0
        || send_all(client_socket, size_str, strlen(size_str)) < 0
        || send_all(client_socket, "\r\n", 2) < 0
        || send_all(client_socket, "\r\n", 2) < 0)
            return -1;
    } else if (header->status == 404) {
        strcpy(content_type, "Content-Type: text/html;charset=utf-8");
        if(send_all(client_socket, " 404 NOT FOUND\r\n", strlen(" 404 NOT FOUND\r\n")) < 0
        || send_all(client_socket, content_type, strlen(content_type)) < 0
        || send_all(client_socket, "\r\n", 2) < 0
        || send_all(client_socket, "\r\n", 2))
            return -1;
    } else if(header->status == 501) {
        if(send_all(client_socket, not_implemented, strlen(not_implemented))
        || send_all(client_socket, "\r\n", 2) < 0)
            return -1;
        return 0;
    }

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
        if(header->status == 200)
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
        
