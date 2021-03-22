/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <dirent.h> 
#include <time.h>
#include <unistd.h>
//#define PORT "3501" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{

	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
    char file[MAXDATASIZE];
	
	//time
	struct timespec ts_start, ts_end;


	if (argc != 3) {
	    fprintf(stderr,"usage: client server_address port\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
    //receive hello from server
	// if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	//     perror("recv");
	//     exit(1);
	// }
	// buf[numbytes] = '\0';
	// printf("client: received '%s'\n",buf);

    //send 
    
    printf("please enter file name in format：ftp <file name>\nftp ");
    scanf("%s", file);
    printf("you enter: %s ",file);
    printf("\n");

	struct dirent *de;
     DIR *dr = opendir(".");
     if(dr == NULL)
     {
        printf("Could not open current directory" );
     }
	
	int file_matched = 0;
    while ((de = readdir(dr)) != NULL) {
    	//printf("%s\n", de->d_name); 
		if(!strcmp(file, de->d_name)){
		
			timespec_get(&ts_start, TIME_UTC);
			char buff[100];
			strftime(buff, sizeof buff, "%D %T", gmtime(&ts_start.tv_sec));
			printf("Current time: %s.%09ld UTC\n", buff, ts_start.tv_nsec);
			if (send(sockfd, "ftp", 13, 0) == -1)
				perror("send");
			file_matched = 1;
		}
	}
	if(!file_matched){
		return 0;
	}
    closedir(dr);     


    //receive responde
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}
	buf[numbytes] = '\0';
	printf("client: received '%s'\n",buf);
	
	
    timespec_get(&ts_end, TIME_UTC);
    char buff[100];
    strftime(buff, sizeof buff, "%D %T", gmtime(&ts_end.tv_sec));
    printf("Current time: %s.%09ld UTC\n", buff, ts_end.tv_nsec);
	printf("RTT: 0.%09ld \n", ts_end.tv_nsec - ts_start.tv_nsec);


	if(!strcmp(buf, "yes")){
		printf("A file transfer can start.\n");	
	}

	close(sockfd);

	return 0;
}