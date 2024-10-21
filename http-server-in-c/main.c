#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

#include "socket_defs.h"
#include "request_header.h"

#define SERVER_ADDR "0.0.0.0"

char *error_msg;

int main(int argc, char *argv[]){
    // take port as argument of the program
    char *port;
    if(argc < 2){
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    port = argv[1];
    char *server_addr = SERVER_ADDR;

    int s = srv_init(server_addr, atoi(port), error_msg);
    if(s < 0){
        fprintf(stderr, "socket error: %s", error_msg);
        exit(1);
    }

    // creating while loop for constant connection
    while(1){
        int *c = malloc(sizeof(int));
        *c = client_accept(s, error_msg);
        if(!c){
            fprintf(stderr, "connection error: %s", error_msg);
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client_request, (void *)c);
        pthread_detach(thread_id);
    }
    close(s);
    return 0;
}

