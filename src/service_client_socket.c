#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
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

static int process_http_request_line(char* buffer, size_t length, http_header *header) {
    // Check for method
    if(length >= 4 && strncmp(buffer, "GET ", 4) == 0) {
        header->method = GET;
        printf("GET REQUEST\n");
    } else if(length >= 4 && strncmp(buffer, "HEAD ", 4) == 0) {
        header->method = HEAD;
        printf("HEAD REQUEST\n");
    } else if(length >= 5 && strncmp(buffer, "POST ", 5) == 0) {
        header->method = POST;
        printf("POST REQUEST\n");
    } else {
        header->method = UNSUPPORTED;
        return -1;
    }

    char *version;
    strtok(buffer, " ");
    header->resource = strtok(NULL, " ");
    version = strtok(NULL, " ");

    // Malformed HTTP request
    if(version == NULL) {
        header->status = 400;
        return -1;
    } else if(strcmp(version, "HTTP/1.1") != 0 || strcmp(version, "HTTP/1.0") != 0) {
        header->status = 505;
        return -1;
    }
    
    if(header->resource == NULL) {
        header->status = 505;
        return -1;
    }

    if(strcmp(header->resource, "/")) {
        header->resource = "/index.html";
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
            process_http_request_line(buffer, last_index, &header);
            printf("%s\n\n", header.resource);

            // Send the file
            FILE *fp = fopen("/home/brendan/work/http-server/www/index.html", "r");
            while(!feof(fp)) {
                char c = fgetc(fp);
                write(client_socket, &c, 1);
            }
            //if(send_all(client_socket, buffer, last_index) != 0) {
            //    perror("send_all");
            //}       
            last_index = 0;
        } else {
            if(last_index >= MAX_LINE_SIZE) {
                fprintf(stderr, "Length of line in header too long.");
                return -1;
            }

            last_index++;
        }

    }

    printf("%s: Disconnected.\n", printable_address);

    close(client_socket);

    return 0;

}
