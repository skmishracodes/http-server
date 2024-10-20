// common
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

// networking
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


// static variables
#define LISTENADDR "0.0.0.0"

struct HttpdRequest {
    char url[128];
    char method[8];
};

struct Route {
    char method[8];
    char url[128];
    void (*callback)(int c, char *url);
};

struct rFile {
    char filename[64];
    char *fc;
    int size;
};

// defining a type for struct httpdrequest
typedef struct HttpdRequest httpreq;
typedef struct Route httproute;
typedef struct rFile httpfile;

// global variable for error
char *error;

// return 0 on error else return socket fd
int srv_init(int port){
    int s;
    struct sockaddr_in srv;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0){
        return 0;
    }

    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(LISTENADDR);
    srv.sin_port = htons(port);

    if(bind(s, (struct sockaddr *)&srv, sizeof(srv))){
        close(s);
        error = "bind() error";
        return 0;
    }

    if(listen(s, 5)){
        close(s);
        error = "listen() error";
        return 0;
    }
    return s;
};

int cli_accept(int s){
    int c;
    socklen_t addrlen;
    struct sockaddr_in client;
    addrlen = 0;
    memset(&client, 0, sizeof(client));

    c = accept(s, (struct sockaddr *)&client, &addrlen);
    if(c < 0){
        error = "accept() error";
        return 0;
    }
    return c;
}

httpreq *parse_http(char *str){
    httpreq *req;
    req = malloc(sizeof(httpreq));

    sscanf(str, "%s %s", req->method, req->url);
    return req;
}

void http_header(int c, int scode){
    char buf[512];
    int n;
    memset(buf, 0, 512);
    snprintf(buf, 511,
             "HTTP/1.1 %d OK\r\n"
             "Connection: keep-alive\r\n"
             "Server: HTTP-SERVER-C\r\n"
             "Cache-Control: no-store\r\n"
             "Content-Language: en\r\n" , scode);
    n = strlen(buf);
    write(c, buf, n);
    return;
}

void http_response(int c, char *con_type, char *data){
    char buf[512];
    int n;
    int con_len = strlen(data);

    snprintf(buf, 511,
             "Content-Type: %s\r\n"
             "Content-Length: %d\r\n"
             "\n%s\r\n", con_type, con_len, data);
    n = strlen(buf);
    write(c, buf, n);
    return;
};

httpfile *readfile(char *filename){
    char buf[512];
    char *p;
    int n, x, fd;
    httpfile about_route*f;

    fd = open(filename, O_RDONLY);
    if(fd < 0){
        return 0;
    }

    f = malloc(sizeof(httpfile));
    if(!f){
        close(fd);
        return 0;
    }

    strncpy(f->filename, filename, 63);
    f->fc = malloc(512);

    x = 0;
    while(1){
        memset(buf, 0, 512);
        n = read(fd, buf, 512);
        if(!n){
            break;
        }else if(x == -1){
            close(fd);
            free(f->fc);
            free(f);
            return 0;
        }
        strncpy(buf, (f->fc)+x, n);
        x += n;
        f->fc = realloc(f->fc, (512+x));
    }
    f->size  = x;
    close(fd);
    return f;
};

// read file and write on the packets to send
int sendfile(int c, char *ctype, httpfile *file){
    char buf[512];
    char *p;
    int n,x;
    if(!file){
        return 0;
    }
    int clen = strlen(file->fc);
    memset(buf, 0, 512);
    snprintf(buf, 511,
             "Content-Type: %s\r\n"
             "Content-Length: %d\r\n", ctype, clen);
    n = file->size;
    p = file->fc;
    while(1){
        x = write(c, p, (n < 512)?n:512);
        if(x < 1){
            return 0;
        }
        n -= x;
        if(n < 1){
            break;
        }else{
            p += x;
        }
        return 1;
    };
    return 0;
};

// routes functions
void home_route(int c, char *url){
    http_header(c, 200);
    http_response(c, "text/html", "<html><body>Hello Home</body></html>");
    return;
}
void about_route(int c, char *url){
    http_header(c, 200);
    http_response(c, "text/html", "<html><body>Hello About</body></html>");
    return;
}

void not_found(int c, char *url){
    char *response = "404 Page not found";
    int conlen = strlen(response);
    http_header(c, 404);
    http_response(c, "text/plain", "Not FOUND");
    return;
};

httproute routes[2] = {
    {"GET", "/home", home_route},
    {"GET", "/about", about_route},
    {"GET", "/image", image_route},
};
/*
void parse_url(char *method, char *url, int c, httproute routes[], int route_size){
    for(int i = 0; i < route_size; i++){
        if(!strcmp(method, routes[i].method) && !strncmp(url, "/img/", 5)){
            memset(str, 0, 96);
            snprintf(".%s", 95, url);
        }
        if(!strcmp(method, routes[i].method) && !strcmp(url, routes[i].url)){
            routes[i].callback(c, routes[i].url);
            return;
        }
    }
    not_found(c, url);
    return;
}
*/

char *cli_read(int c){
    static char buf[512];
    memset(buf, 0, 512);

    if(read(c, buf, 512) < 0){
        error = "read() error";
        return 0;
    }else{
        return buf;
    }
}

void handle_cli_req(int s, int c){
    httpreq *req;
    char *p;
    char str[96];
    httpfile *file;

    p = cli_read(c);
    if(!p){
        fprintf(stderr, "%s\n", error);
        close(c);
        return;
    }

    req = parse_http(p);
    if(!req){
        fprintf(stderr, "%s\n", error);
        close(c);
        free(req);
        return;
    }

    int routes_size = sizeof(routes) / sizeof(routes[0]);
    for(int i = 0; i < routes_size; i++){
        if(!strcmp(req->method, routes[i].method) && !strncmp(req->url, "/img/", 5)){
            memset(str, 0, 96);
            snprintf(str, 95,".%s", req->url);
            file = readfile(req->url);
            if(!file){
                not_found(c, req->url);
            }else{
                http_header(c, 200);
                if(!sendfile(c, "image/png", file)){
                    http_header(c, 500);
                    not_found(c, req->url);
                }
            }
        }
        if(!strcmp(req->method, routes[i].method) && !strcmp(req->url, routes[i].url)){
            routes[i].callback(c, routes[i].url);
            return;
        }
    }
    not_found(c, req->url);

    free(req);
    close(c);
    return;
}

int main(int argc, char *argv[]) {
    int s ;
    char *port;

    if(argc < 2){
        fprintf(stderr, "Usage: %s <listening port>\n", argv[0]);
    }

    port = argv[1];
    s = srv_init(atoi(port));
    if(!s){
        fprintf(stderr, "%s\n", error);
        return -1;
    }

    printf("Listening for connections %s:%s\n", LISTENADDR, port);
    while(1){
        int c = cli_accept(s);
        if(!c){
            fprintf(stderr, "%s\n", error);
            continue;
        }
        printf("Incoming Connection\n");
        if(!fork()){
            handle_cli_req(s, c);
        }
    }
    return -1;
};
