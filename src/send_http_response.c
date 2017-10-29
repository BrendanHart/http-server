#include "http_header.h"
#include "send_http_response.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

static int send_all(SSL *ssl, char *buffer, size_t size) {
    // Send data until there's no bytes left to be sent
    while(size > 0) { 
        int written = SSL_write(ssl, buffer, size);
        if(written < 0)
            return -1;
        size -= written;
    }
    return size;
}

static int read_local_file(const char *root_dir, const char *file, char **buffer, size_t* size) {
    if(file == NULL)
        return -1;

    // Compute full url
    char full_url[MAX_LINE_SIZE];
    strcpy(full_url, root_dir);
    strcat(full_url, file);

    FILE *fp = fopen(full_url, "r");

    *size = 0;

    if(fp == NULL) {
        // File not found
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
            // Read the file into the buffer and put
            // the amount of bytes into the size variable
            *size = fread(*buffer, sizeof(char), bufsize, fp);
            if(ferror(fp) != 0) {
                // ERROR
            }
        }
        fclose(fp);

    }
    return 0;

}

static char* send_response_header(SSL* ssl, http_header *header, const size_t content_size) {

    // Make the content size into a string
    char size_str[256];
    snprintf(size_str, sizeof(size_str), "%zu", content_size);

    // Determine the content type from the file name
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
        } else {
            strcat(content_type, "text/plain");
        }
    }

    // Allows for HTTP/1.0 responses as well as HTTP/1.1
    if(header->version)
        send_all(ssl, header->version, strlen(header->version));
    else
        send_all(ssl, "HTTP/1.1", strlen("HTTP/1.1"));

    char * bad_version = " 505 HTTP Version Not Supported";
    char * not_implemented = " 501 Not Implemented";
    char * bad_request = " 400 Bad Request";
    char * forbidden = " 403 Forbidden";

    // Check the status and send the appopriate headers
    if(header->status == 400) {
        send_all(ssl, bad_request, strlen(bad_request));
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "Content-Type: text/plain", 24);
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "Content-Length: ", 16);
        send_all(ssl, "15", 2);
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "\r\n", 2);
        return "400 Bad Request";
    } else if(header->status == 403) {
        send_all(ssl, forbidden, strlen(forbidden));
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "Content-Type: text/plain", 24);
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "Content-Length: ", 16);
        send_all(ssl, "13", 2);
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "\r\n", 2);
        return "403 Forbidden";
    } else if(header->status == 505) {
        send_all(ssl, bad_version, strlen(bad_version));
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "Content-Type: text/plain", 24);
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "Content-Length: ", 16);
        send_all(ssl, "30", 2);
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "\r\n", 2);
        return "505 HTTP Version Not Supported";
    } if(header->status == 200) {
        send_all(ssl, " 200 OK\r\n", strlen(" 200 OK\r\n"));
        send_all(ssl, content_type, strlen(content_type));
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "Content-Length: ", 16);
        send_all(ssl, size_str, strlen(size_str));
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "\r\n", 2);
    } else if (header->status == 404) {
        strcpy(content_type, "Content-Type: text/html;charset=utf-8");
        send_all(ssl, " 404 NOT FOUND\r\n", strlen(" 404 NOT FOUND\r\n"));
        send_all(ssl, "Content-Type: text/plain", 24);
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "Content-Length: ", 16);
        send_all(ssl, "13", 2);
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "\r\n", 2);
        return "404 Not Found";
    } else if(header->status == 501) {
        send_all(ssl, not_implemented, strlen(not_implemented));
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "Content-Type: text/plain", 24);
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "Content-Length: ", 16);
        send_all(ssl, "19", 2);
        send_all(ssl, "\r\n", 2);
        send_all(ssl, "\r\n", 2);
        return "501 Not Implemented";
    }

    return NULL;
}


int send_http_response(SSL* ssl, 
                    const char *root_dir,
                    http_header* header) {

    // Get the file contents
    char *source;
    size_t size_read = 0;
    int result = read_local_file(root_dir, header->resource, &source, &size_read);
    if(result == -1) {
        source = NULL;
        if(header->status == 200)
            header->status = 404;
        size_read = 13;
    }


    // Send the header response
    char *body = send_response_header(ssl, header, size_read);

    // Sometimes we might want to send an error string instead of
    // the local file, so check if body is NULL, meaning
    // send_response_header does not request to override the
    // sending of the local file.
    if(body == NULL) {
        send_all(ssl, source, size_read);
    } else {
        send_all(ssl, body, strlen(body));
    }

    // Free the buffer and return
    free(source);

    return 0;

}
        
