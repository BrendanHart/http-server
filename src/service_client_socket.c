#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024

enum request_method {
    POST, GET, HEAD, UNSUPPORTED
};

typedef struct http_header {
    enum request_method method;
    char* user_agent;
    char* resource;
    int status;
} http_header;

//static int send_all(const int client_socket, char* buffer, size_t size) {
//    while(size > 0) { 
//        size -= write(client_socket, buffer, size);
//    }
//    return size;
//}

static int process_http_header_line(char* buffer, size_t length, http_header *header) {
    if(header->resource == NULL) {
        // No resource in header struct, so this must be the first line.
        // Check for method
        if(length >= 4 && strncmp(buffer, "GET ", 4) == 0) {
            header->method = GET;
        } else if(length >= 4 && strncmp(buffer, "HEAD ", 4) == 0) {
            header->method = HEAD;
        } else if(length >= 5 && strncmp(buffer, "POST ", 5) == 0) {
            header->method = POST;
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
        strcpy(header->resource, resource);
    } else {

        // We've already processed the resource line here.
        // What we have to process now is the rest of the headers.
        

    }

    return 0;
}

int service_client_socket(const int client_socket, const char *const printable_address) {

    char buffer[MAX_LINE_SIZE];
    size_t bytes_read;
    size_t last_index = 0;

    printf("%s: Connected.\n", printable_address);
    
    http_header header;
    while((bytes_read = read(client_socket, &buffer[last_index], 1)) == 1) {

        if(buffer[last_index] == '\n') {
            buffer[last_index-1] = '\0';
            process_http_header_line(buffer, last_index, &header);

            char full_url[MAX_LINE_SIZE];
            strcpy(full_url, "/home/students/bxh538/work/http-server/www");
            strcat(full_url, header.resource);

            FILE *fp = fopen(full_url, "r");
            fseek(fp, 0L, SEEK_END);
            size_t sz = ftell(fp);
            rewind(fp);
            printf("SIZE: %d\n", (int)sz);
            write(client_socket, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"));
            write(client_socket, "Content-Type: text/html\r\n", 25);
            write(client_socket, "\r\n", 2);
            int count = 1;
            char c;
            do {
                c = (char)fgetc(fp);
                printf("char: %x\n", c);
                write(client_socket, &c, 1);
                count += 1;
            }while(count < sz);

            write(client_socket, "\r\n", 2);
            break;

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
