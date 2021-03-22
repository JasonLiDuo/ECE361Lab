/*
** talker.c -- a datagram "client" demo
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

int main(int argc, char *argv[]){
  //create a socket
  int deliver_socket;
  deliver_socket = socket(AF_INET, SOCK_STREAM, 0); //using TCP
  
  //specify an address for the socket
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(9002);
  server_address.sin_addr.s_addr = INADDR_ANY;

  int connection_status = connet(deliver_socket, (struct sockaddr *) &server.address, sizeof(server_address));
  //check connection error
  if (connection_status == -1){
    printf("connection wrong");
  }
  //receive data from the server
  char server_response[256];
  recv(deliver_socket, &server_response, sizeof(server_response),0);

  //print out server's response
  printf("The server sent the data: %s\n", server_response);

  //then close the socket
  close(deliver_socket);
  return 0; 
}