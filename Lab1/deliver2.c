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

#define MAXBUFLEN 100
#define BACKLOG 10

int main(int argc, char *argv[]){
  //create a socket
  int deliver_socket;
  char file[MAXBUFLEN];
  char deliver_message[MAXBUFLEN] = "ftp";
  deliver_socket = socket(AF_INET, SOCK_STREAM, 0); //using TCP
  //const char* SERVERPORT;
  // if (argc != 3) {
  //   fprintf(stderr,"deliver <server address> <server port number>\n");
  //   exit(1);
  // }
  
  //SERVERPORT = argv[2];
  //specify an address for the socket
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(57353);
  server_address.sin_addr.s_addr = INADDR_ANY;

  int connection_status = connect(deliver_socket, (struct sockaddr *) &server_address, sizeof(server_address));
  //check connection error
  if (connection_status == -1){
    printf("connection wrong");
  }

  printf("input a message as follows:\nftp <file name>\nftp ");
  scanf("%s", &file);
  
  if(access(file, F_OK) == 0){
    int server_socket;
    server_socket = accept(deliver_socket, NULL, NULL);
    //send the message to server
    send(server_socket, "ftp", sizeof("ftp"),0);
  }else{
    return 0;
  }




  // int server_socket;
  // server_socket = accept(deliver_socket, NULL, NULL);
  // printf("2\n");
  // //send the message to server
  // send(server_socket, deliver_message, sizeof(deliver_message),0);
  //receive data from the server

  char server_response[MAXBUFLEN];
  recv(deliver_socket, &server_response, sizeof(server_response),0);

  if(strcmp(server_response, "yes")==0){
    printf("A file transfer can start.");
  }else{
    return 0;
  }
  //print out server's response
  printf("The server sent the data: %s\n", server_response);

  //then close the socket
  close(deliver_socket);
  return 0; 
}