/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 1100 // max number of bytes we can get at once

int sockfd; //listening socket descriptor

//define packet struct
struct packet{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    unsigned char filedata[1000];
};

void fill_struct(struct packet * pac, unsigned char* buf, char* filename);
void str_clean(unsigned char* var, int size);

//Ctrl-C to kill the program
static void intHandler(int _)
{
    close(sockfd); //shuts down the socket associated with the socket descriptor socket, and frees resources allocated to the socket
}

//child process
void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning
	int saved_errno = errno; // waitpid() might overwrite errno, so we save and restore it
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	} //IPv4
	return &(((struct sockaddr_in6*)sa)->sin6_addr); //IPv6
}

int main(int argc, char *argv[]){	
    int new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];

	int rv, numbytes; 
    unsigned char buf[MAXDATASIZE];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	//if user typed other than 2 arguments, give user hint what to type in
    if (argc != 2) {
	    fprintf(stderr,"usage: server port\n");
	    exit(1);
	}


	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
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

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");
    signal(SIGINT, intHandler);
	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			break;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener 

     		//recive buf from deliver
            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            
			//decode buf for pac
			struct packet pac;
			char filename[100] = {'\0'};
			pac.filename = filename; 

            fill_struct(&pac, &buf, &filename);
 
			//create file
			FILE *fp;
			fp = fopen(filename,"w");
			for(int i= 0; i<pac.size ; i++){
				fputc(pac.filedata[i] , fp);
			}

			//loop to receive all packet
            if(pac.total_frag != 1){
                while(pac.frag_no < pac.total_frag -1){
                    //send ack
                    if (send(new_fd, "ack: packet received", 1000, 0) == -1)
				        perror("send");

                    //receive
                    if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
                        perror("recv");
                        exit(1);
                    }

                    //decode buf
                    fill_struct(&pac, &buf, &filename);

                    for(int i= 0; i<pac.size  ; i++){
                        fputc(pac.filedata[i] , fp);    
                    }
                }
            }

			if(pac.total_frag = 1){
                //send ack
                if (send(new_fd, "ack: packet received", 1000, 0) == -1)
				    perror("send");

                //receive
                if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }

                //decode buf
                fill_struct(&pac, &buf, &filename);              
            }

            printf("transfer completed\n");
			fclose(fp);

			close(new_fd);// parent doesn't need this
			exit(0);
		}
	}   
	return 0;
}

//empty string function
void str_clean(unsigned char* var, int size){
  for (int i = 0; i < size; i++) {
    var[i] = 0; 
  }
}

void fill_struct(struct packet * pac,unsigned char* buf, char* filename){
	int i=1;
    int j=0;
    unsigned char total_frag[100];

    //1            
    for( j=0 ; buf[i]!=':' ; i++){
        total_frag[j] = buf[i];
        j++;
    }
    sscanf(total_frag, "%d", &pac->total_frag);
    i++;

    //2
    str_clean(total_frag,100);
    for( j=0; buf[i]!=':' ; i++){
        total_frag[j] = buf[i];
        j++;
    }
    sscanf(total_frag, "%d", &pac->frag_no);
    i++;

    //3
    str_clean(total_frag,100);
    for( j=0;buf[i]!=':' ; i++){
        total_frag[j] = buf[i];
        j++;
    }
    sscanf(total_frag, "%d", &pac->size);
    i++;

    //4
    str_clean(total_frag,100);
    for( j=0;buf[i]!=':' ; i++){
        if(buf[i] == '.'){
            filename[j] = '2';
            j++;
        }
        filename[j] =  (char)buf[i];
        j++;
    }
    i++;
    str_clean(total_frag,100);
    for( j=0; j<pac->size ; i++){
        pac->filedata[j] = buf[i];
        j++;
    }
    i++;
    pac->filedata[j] = 0;
}