#ifndef REQUEST_HEADER_H
#define REQUEST_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// structure to store request params like url and method for better access
struct req_mtd_url {
    char method[8];
    char url[256];
};

char *content_type(char *filename);

void handle_route(struct req_mtd_url *req_params);

char *read_request_data(int client_fd){
    static char buffer[1024];
    memset(buffer, 0, 1024);

    if(read(client_fd, buffer, 1024) < 0){
        return 0;
    } else {
        return buffer;
    }
}

struct req_mtd_url *parse_url_method(char *data){
    struct req_mtd_url *req;
    req = malloc(sizeof(struct req_mtd_url));
    sscanf(data, "%s %s", req->method, req->url);
    return req;
}

void handle_client_request(int client_fd){
    struct req_mtd_url *req;
    //int client_fd = *((int *)args);
    char *req_data = read_request_data(client_fd);
    if(!req_data){
        close(client_fd);
        return;
    }
    req = parse_url_method(req_data);
    if(!req){
        close(client_fd);
        free(req);
        return;
    }
    handle_route(req);

    close(client_fd);
    free(req);
    return ;
}

#endif
