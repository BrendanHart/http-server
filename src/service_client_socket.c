#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http_header.h"
#include "send_http_response.h"

static int process_http_header_line(char* buffer, size_t length, http_header *header) {
    header->status = 200;

    if(header->resource == NULL) {
        // No resource in header struct, so this must be the first line.
        // Check for method
        if(length >= 4 && strncmp(buffer, "GET ", 4) == 0) {
            header->method = GET;
        } else if(length >= 4 && strncmp(buffer, "HEAD ", 4) == 0) {
            header->method = HEAD;
        } else {
            header->method = UNSUPPORTED;
        }

        char *version, *resource;
        strtok(buffer, " ");
        resource = strtok(NULL, " ");
        version = strtok(NULL, " ");


        // Malformed HTTP request
        if(resource == NULL) {
            header->status = 400;
            return -1;
        } else {
            if(strcmp(resource, "/") == 0) {
                resource = "/index.html";
            }
            header->resource = calloc(strlen(resource) + 1, sizeof(char));
            strcpy(header->resource, resource);
        }

        if(version == NULL) {
            header->status = 400;
            header->version = calloc(strlen("HTTP/1.1") + 1, sizeof(char));
            strcpy(header->version, "HTTP/1.1");
            return -1;
        } else if(strcmp(version, "HTTP/1.1") != 0 && strcmp(version, "HTTP/1.0") != 0) {
            header->status = 505;
            header->version = calloc(strlen("HTTP/1.1") + 1, sizeof(char));
            strcpy(header->version, "HTTP/1.1");
            return -1;
        } else {
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
    http_header* header = malloc(sizeof(http_header));
    header->resource = NULL;
    header->version = NULL;
    header->method = UNSUPPORTED;
    header->user_agent = NULL;
    header->status = -1;
    return header;
}

static int read_header_lines(const int client_socket, http_header *header) {
    size_t bytes_read;
    size_t last_index = 0;
    char buffer[MAX_LINE_SIZE];
    int recent_crlf = 0;

    while((bytes_read = read(client_socket, &buffer[last_index], 1)) == 1) {
        
        if(last_index > 0 && buffer[last_index-1] == '\r' && buffer[last_index] == '\n') {
            if(recent_crlf == 1)
                return 0;
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


int service_client_socket(const int client_socket, const char *const printable_address, const char *root_dir) {

    int success;

    printf("%s: Connected.\n", printable_address);
    
    http_header *header = new_header();

    success = read_header_lines(client_socket, header);
    if(success != 0) {
        header->status = 400;
        header->version = calloc(strlen("HTTP/1.1") + 1, sizeof(char));
        strcpy(header->version, "HTTP/1.1");
    } else if(header->method == UNSUPPORTED)
        header->status = 501;

    success = send_http_response(client_socket, root_dir, header);
    if(success != 0)
        printf("%s: Error sending http response.", printable_address);

    free(header->version);
    free(header->resource);
    free(header);
    printf("%s: Disconnected.\n", printable_address);

    //shutdown(client_socket, 2);
    close(client_socket);

    return 0;

}
