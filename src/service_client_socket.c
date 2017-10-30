#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/err.h>

#include "http_header.h"
#include "send_http_response.h"
#include "service_client_socket.h"

static int process_http_header_line(char* buffer, size_t length, http_header *header) {
    printf("%s\n", buffer);

    if(header->resource == NULL) {
        header->status = 200;
        // No resource in header struct, so this must be the first line.
        // Check for method
        if(length >= 3 && strncmp(buffer, "GET", 3) == 0) {
            header->method = GET;
        } else if(length >= 4 && strncmp(buffer, "HEAD", 4) == 0) {
            header->method = HEAD;
        } else {
            header->method = UNSUPPORTED;
        }

        // Read the version and resource
        char *version, *resource;
        strtok(buffer, " ");
        resource = strtok(NULL, " ");
        version = strtok(NULL, " ");


        if(resource == NULL) {
            // Malformed HTTP request
            header->status = 400;
            return -1;
        } else {
            if(strstr(resource, "../") != NULL) {
                // Forbidden to prevent access outside the web files root
                header->status = 403;
                return -1;
            } else if(resource[strlen(resource)-1] == '/') {
                // If the last character is a slash,
                // the user must be trying to access a directory. As such,
                // try return the index.html page for that directory.
                header->resource = calloc(strlen(resource) + strlen("index.html") + 1,
                                            sizeof(char));
                strcpy(header->resource, resource);
                strcat(header->resource, "index.html");
            } else {
                // Just put the resource insid
                header->resource = calloc(strlen(resource) + 1, sizeof(char));
                strcpy(header->resource, resource);
            }
        }

        if(version == NULL) {
            // Bad request
            header->status = 400;
            header->version = calloc(strlen("HTTP/1.1") + 1, sizeof(char));
            strcpy(header->version, "HTTP/1.1");
            return -1;
        } else if(strcmp(version, "HTTP/1.1") != 0 && strcmp(version, "HTTP/1.0") != 0) {
            // Incompatible HTTP version
            header->status = 505;
            header->version = calloc(strlen("HTTP/1.1") + 1, sizeof(char));
            strcpy(header->version, "HTTP/1.1");
            return -1;
        } else {
            // Copy the version string into the http_header struct
            header->version = calloc(strlen(version) + 1, sizeof(char));
            strcpy(header->version, version);
        }



    } else {

        // We've already processed the resource line here.
        // What we have to process now is the rest of the headers.

    }

    return 0;
}

static http_header* new_header() {
    // Create a new http_header with default values
    http_header* header = malloc(sizeof(http_header));
    header->resource = NULL;
    header->version = NULL;
    header->method = UNSUPPORTED;
    header->user_agent = NULL;
    header->status = -1;
    return header;
}

static int read_header_lines(SSL* ssl, http_header *header) {
    size_t bytes_read;
    size_t last_index = 0;
    char buffer[MAX_LINE_SIZE];
    int recent_crlf = 0;

    // Read each character at a time
    while((bytes_read = SSL_read(ssl, &buffer[last_index], 1)) == 1) {
        
        if(last_index > 0 && buffer[last_index-1] == '\r' && buffer[last_index] == '\n') {
            // CRLF detected
            // If the two bytes before this CRLF were also a CRLF, we can
            // return as we've processed the header lines
            if(recent_crlf == 1)
                return 0;
            // Otherwise, we call process_http_header_line
            // to extract the information from this header line
            buffer[last_index-1] = '\0';
            process_http_header_line(buffer, last_index, header);
            buffer[last_index-1] = '\r';
            recent_crlf = 1;
            last_index = 0;
        } else {
            last_index++;
            if(last_index >= MAX_LINE_SIZE) {
                fprintf(stderr, "Length of line in header too long.");
                printf("ERROR\n");
                return -1;
            }
        }

        if(last_index > 1)
            recent_crlf = 0;

    }
    return -1;

}

int service_client_socket(SSL* ssl, const char *const printable_address, const char *root_dir) {

    int success;

    printf("%s: Connected.\n", printable_address);

    if(SSL_accept(ssl) == -1) {
        ERR_print_errors_fp(stderr);
    } else {
        // SSL_accept succeeded in handling the SSL protocol,
        // now we can just make calls to SSL_write and SSL_read
        // to send and receive data.

        // Read the header lines
        http_header *header = new_header();
        success = read_header_lines(ssl, header);
        if(success != 0) {
            header->status = 400;
            header->version = calloc(strlen("HTTP/1.1") + 1, sizeof(char));
            strcpy(header->version, "HTTP/1.1");
        } else if(header->method == UNSUPPORTED)
            header->status = 501;

        // Send the HTTP response
        success = send_http_response(ssl, root_dir, header);
        if(success != 0)
            printf("%s: Error sending http response.", printable_address);

        // Free the memory used
        free(header->version);
        free(header->resource);
        free(header);
        printf("%s: Disconnected.\n", printable_address);
    }

    SSL_free(ssl);

    return 0;

}
