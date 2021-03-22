/*
** server.c -- a datagram sockets "server" demo
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

//#define MYPORT "4950" // the port users will be connecting to
#define MAXBUFLEN 100
#define BACKLOG 10
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){
    int client_socket;
    int sockfd;
    int yes=1;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    const char* MYPORT;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if (argc != 2) {
        fprintf(stderr,"server <UDP listen port>\n");
        exit(1);
    }

    MYPORT = argv[1];
    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "server <UDP listen port>\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        perror("server: socket");
        continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        exit(1);
    }

    printf("server: waiting to recvfrom...\n");
    while(true){
        addr_len = sizeof their_addr;
        client_socket = accept(sockfd, (struct sockaddr *)&their_addr, &addr_len);
        
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
        if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            //printf("server: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
            perror("recvfrom");
            exit(1);
        }
        //printf("server: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
        printf("server: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        printf("server: packet contains \"%s\"\n", buf);
        if(strcmp(buf, "ftp")==0){
            //send the message
            //send respond to client
            if (send(client_socket, "yes", 13, 0) == -1)
			   perror("send");
            //close the socket
            close(client_socket);
            printf("yes\n");
            break;
        }else{
            //send the message
            //send respond to client
            if (send(client_socket, "no", 13, 0) == -1)
				perror("send");
            //close the socket
            close(client_socket);
            printf("no\n");
            break;
        }
    }
    return 0;
}