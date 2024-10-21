#ifndef REQUEST_HEADER_H
#define REQUEST_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PUBLIC_DIR "./public"

// structure to store request params like url and method for better access
struct req_mtd_url {
    char method[8];
    char url[256];
};

struct route {
    char method[8];
    char url[256];
    void (*callback)(void);
};

char *mime_type(char *file_ext){
    char *ctype;
    if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0) {
        ctype = "text/html";
        return ctype;
    } else if (strcasecmp(file_ext, "txt") == 0) {
        ctype =  "text/plain";
        return ctype;
    } else if (strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0) {
        ctype = "image/jpeg";
        return ctype;
    } else if (strcasecmp(file_ext, "png") == 0) {
        ctype= "image/png";
        return ctype;
    } else {
        ctype= "application/octet-stream";
        return ctype;
    }
}

char *get_file_ext(char *filename){
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename){
        return "";
    }
    return dot + 1;
}

void handle_files(int client_fd, char *filename, size_t contentlen){
    int file = open(filename, O_RDONLY);
    if(file < 0){
        printf("NOT FOUND\n");
        close(client_fd);
        return ;
    }
    char *file_ext = get_file_ext(filename);
    char *content_type = mime_type(file_ext);

    // send headers then
    char headers[512];
    snprintf(headers, 511,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\n\n"
             "\r\n", content_type, contentlen);

    send(client_fd, headers, strlen(headers), 0);

    char buffer[4096];
    size_t bytes_read;

    while((bytes_read = read(file, buffer, sizeof(buffer))) > 0){
        char chunk_size[256];
        snprintf(chunk_size, sizeof(chunk_size), "%zx\r\n", bytes_read);
        send(client_fd, buffer, bytes_read, 0);
    }
    send(client_fd, "0\r\n\r\n", 5, 0);

    // send file
    close(client_fd);
    close(file);
    return;
}

void handle_callback(int client_fd, struct route route){
    route.callback();
    close(client_fd);
    return;
}

void homeroute(){
    printf("HOMEROUTE \n");
}

struct route routes[1] ={
    {"GET", "/home", homeroute},
};

// Handle routes and functions
void handle_route(int client_fd, struct req_mtd_url *req_params){
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s", PUBLIC_DIR, req_params->url);
    struct stat path_stat;
    if(stat(filepath, &path_stat) == 0 && S_ISREG(path_stat.st_mode)){
        handle_files(client_fd, filepath, path_stat.st_size);
        return ;
    }else{
        int routelen = sizeof(routes) / sizeof(routes[0]);
        for (int i=0; i < routelen; i++){
            if(strcmp(routes[i].url, req_params->url) == 0 && strcmp(routes[i].method, req_params->method) == 0){
                handle_callback(client_fd, routes[i]);
                return ;
            }
        }
    }
    close(client_fd);
    return ;
}

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

void *handle_client_request(void *arg){
    int client_fd = *((int *)arg);
    struct req_mtd_url *req;
    char *req_data = read_request_data(client_fd);
    if(!req_data){
        close(client_fd);
        return NULL;
    }

    req = parse_url_method(req_data);
    if(!req){
        close(client_fd);
        free(req);
        return NULL;
    }

    handle_route(client_fd, req);

    close(client_fd);
    free(req);
    free(arg);
    return NULL;
}

#endif
