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

int main(void){
    char server_message[256] = "ssdasdasd";

    int server_socket;
    server_socket = socket = (AF_INET, SOCK_STREAM, 0);

    //define the server address;
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //bind the socket to our specified IP and port
    bind(server_socket, (struct sockaddr *) &server.address, sizeof(server_address));
    listen(server_socket,5);

    int client_socket;
    client_socket = accept(server_socket, NULL, NULL);
    //send the message
    send(client_socket, server_message, sizeof(server_message),0);
    //close the socket
    close(server_socket);

    return 0;
    }