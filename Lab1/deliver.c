/*
** deliver.c -- a datagram "client" demo
reference: Beej's Guide to Network Programming
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

//#define SERVERPORT "4950" // the port users will be connecting to
#define MAXBUFLEN 100 // max number of bytes we can get at once 

int main(int argc, char *argv[]){
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int numbytes;
  const char* SERVERPORT;
  //struct sockaddr_storage their_addr;
  char buf[MAXBUFLEN];
  char *file[MAXBUFLEN];
  //socklen_t addr_len;
  char s[INET6_ADDRSTRLEN];
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if (argc != 3) {
    fprintf(stderr,"deliver <server address> <server port number>\n");
    exit(1);
  }
  SERVERPORT = argv[2];
  if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "deliver <server address> <server port number>\n", gai_strerror(rv));
    return 1;
  }
  // loop through all the results and make a socket
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("deliver: socket");
      continue;
    }
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("deliver: connect");
			close(sockfd);
			continue;
    }
    break;
  }
  if (p == NULL) {
    fprintf(stderr, "deliver: failed to create socket\n");
    return 2;
  }
  
	// inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	// 		s, sizeof s);
	// printf("client: connecting to %s\n", s);

	// freeaddrinfo(servinfo);
  // if ((numbytes = sendto(sockfd, "ddd", strlen(argv[2]), 0, p->ai_addr, p->ai_addrlen)) == -1) {
  //   perror("deliver: sendto");
  //   exit(1);
  // }
  //freeaddrinfo(servinfo);

  printf("input a message as follows:\nftp <file name>\n");
  scanf("%s%s", file[0], file[1]); 

  while(strcmp(file[0], "ftp")!=0){
    printf("input a message as follows:\nftp <file name>\n");
    scanf("%s%s", file[0],file[1]); 
  }

  if(access(file[1], F_OK) == 0){
    if ((numbytes = sendto(sockfd, "ftp", strlen(argv[2]), 0, p->ai_addr, p->ai_addrlen)) == -1) {
    perror("deliver: sendto");
    exit(1);
    }
    freeaddrinfo(servinfo);
  }else{
    return 0;
  }

  //receive responde
  // if ((numbytes = recv(sockfd, buf, MAXBUFLEN-1, 0, p->ai_addr, p->ai_addrlen)) == -1) {
  //   perror("recv");
  //   exit(1);
  //   }
  //   freeaddrinfo(servinfo);
	// printf("client: received '%s'\n",buf);

  //receive data from the server
  //recv(sockfd, &buf, sizeof(buf),0);
  //receive responde
  if ((numbytes = recv(sockfd, buf, MAXBUFLEN-1, 0)) == -1) {
	  perror("recv");
	  exit(1);
	}
	buf[numbytes] = '\0';
	printf("client: received '%s'\n",buf);

  if(strcmp(buf, "yes")==0){
    printf("A file transfer can start.");
  }else{
    return 0;
  }

  printf("deliver: sent %s bytes to %s\n", file[1], argv[1]);
  close(sockfd);
  return 0;
}