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
            return -1;
        }

        char *version, *resource;
        strtok(buffer, " ");
        resource = strtok(NULL, " ");
        version = strtok(NULL, " ");

        // Malformed HTTP request
        if(version == NULL || resource == NULL) {
            header->status = 400;
            return -1;
        } else if(strcmp(version, "HTTP/1.1") != 0 && strcmp(version, "HTTP/1.0") != 0) {
            header->status = 505;
            return -1;
        } 

        if(strcmp(resource, "/") == 0) {
            resource = "/index.html";
        }

        header->resource = calloc(strlen(resource) + 1, sizeof(char));
        header->version = calloc(strlen(version) + 1, sizeof(char));
        strcpy(header->resource, resource);
        strcpy(header->version, version);
    } else {

        printf("%s\n", buffer);
        // We've already processed the resource line here.
        // What we have to process now is the rest of the headers.

    }

    return 0;
}




int service_client_socket(const int client_socket, const char *const printable_address) {

    char buffer[MAX_LINE_SIZE];
    size_t bytes_read;
    size_t last_index = 0;
    int headers_finished = 0;

    printf("%s: Connected.\n", printable_address);
    
    http_header header;
    while((bytes_read = read(client_socket, &buffer[last_index], 1)) == 1) {

        if(buffer[last_index] == '\n') {
            buffer[last_index-1] = '\0';

            process_http_header_line(buffer, last_index, &header);

            // Test that headers are finished here, for now just read
            // the first line.
            headers_finished = 1;

            if(headers_finished) {

                int success = send_http_response(client_socket, "/home/brendan/work/http-server/www", &header);
                if(success == -1)
                    printf("%s: Error sending http response.", printable_address);
                break;
            }

            last_index = 0;
        } else {
            last_index++;
            if(last_index >= MAX_LINE_SIZE) {
                fprintf(stderr, "Length of line in header too long.");
                return -1;
            }

        }

    }

    printf("%s: Disconnected.\n", printable_address);

    shutdown(client_socket, 2);
    close(client_socket);

    return 0;

}
