/*
** listener.c -- a datagram sockets "server" demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h> 

#define BACKLOG 10
#define MAXBUFLEN 100

int main(int argc, char *argv[]){
    char server_message[MAXBUFLEN];
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // if (argc != 2) {
    //     fprintf(stderr,"server <UDP listen port>\n");
    //     exit(1);
    // }

    // const char* MYPORT;
    // MYPORT = argv[1];

    //define the server address;
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //receive data from deliver
    char deliver_response[MAXBUFLEN];
    recv(server_socket, &deliver_response, sizeof(deliver_response),0);
    
    
    // //bind the socket to our specified IP and port
    bind(server_socket, (struct sockaddr *) & server_address, sizeof(server_address));
    
    listen(server_socket, BACKLOG);

    if(strcmp(deliver_response, "ftp")==0){
        //send respond to server
        int client_socket;
        client_socket = accept(server_socket, NULL, NULL);
        //send the message
        send(client_socket, "yes", sizeof("yes"),0);
        printf("yes\n");
        break;
    }else{
        //send respond to client
        int client_socket;
        client_socket = accept(server_socket, NULL, NULL);
        //send the message
        send(client_socket, "no", sizeof("no"),0);
        printf("no\n");
        break;
    }



    // int client_socket;
    // client_socket = accept(server_socket, NULL, NULL);
    // //send the message
    // send(client_socket, server_message, sizeof(server_message),0);
    // //close the socket
    close(server_socket);

    return 0;
    }