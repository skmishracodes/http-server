#ifndef SOCKET_DEFS_H
#define SOCKET_DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

int srv_init(char *server_addr, int port, char *error_msg){
    int s;
    struct sockaddr_in srv;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0){
        error_msg = "socket() error\n";
        return 0;
    }

    // server struct values
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = inet_addr(server_addr);

    if(bind(s, (struct sockaddr *)&srv, sizeof(srv))){
        close(s);
        error_msg = "bind() error\n";
        return 0;
    }
    if(listen(s, 5)){
        close(s);
        error_msg =  "listen() error\n";
        return 0;
    };
    return s;
}

int client_accept(int s, char *error_msg){
    int client;
    struct sockaddr_in clt;
    socklen_t addrlen = 0;
    memset(&clt, 0, sizeof(clt));

    client = accept(s, (struct sockaddr*)&clt, &addrlen);
    if(client < 0){
        error_msg = "accept() error\n";
        return 0;
    }
    return client;
}

#endif
