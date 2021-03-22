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
#define PORT "3501" // the port client will be connecting to 

#define MAXDATASIZE 1100 // max number of bytes we can get at once 

void str_copy(unsigned char* dest,unsigned char* start, int size);
void str_clean(unsigned char* var, int size);
void str_cat(unsigned char* dest,unsigned char* start);

struct packet{
      unsigned int total_frag;
      unsigned int frag_no;
      unsigned int size;
      char* filename;
      unsigned char filedata[1000];
    };

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
  char file_name[MAXDATASIZE];

	//time
	struct timespec ts;
	struct timespec ts_end;


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
    scanf("%s", file_name);
    printf("you enter: %s ",file_name);
    printf("\n");

	// struct dirent *de;
    //  DIR *dr = opendir(".");
    //  if(dr == NULL)
    //  {
    //     printf("Could not open current directory" );
    //  }
	
	// int file_matched = 0;
    // while ((de = readdir(dr)) != NULL) {
    // 	//printf("%s\n", de->d_name); 
	// 	if(!strcmp(file, de->d_name)){
		
	// 		// timespec_get(&ts, TIME_UTC);
	// 		// char buff[100];
	// 		// strftime(buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec));
	// 		//printf("Current time: %s.%09ld UTC\n", buff, ts.tv_nsec);

	// 		char send_buf[1100] = "4:2:3500:foobar.txt:bbcccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc";
	// 		if (send(sockfd, send_buf, 5000, 0) == -1)
	// 			perror("send");
	// 		file_matched = 1;
	// 	}
	// }
	// if(!file_matched){
	// 	return 0;
	// }
    // closedir(dr);     

	///////////////////////////////////////////////////////////
	
 
    unsigned char buffer[1100];
    FILE *fp;
   fp = fopen(file_name, "r"); // read mode
   if (fp == NULL)
   {
      perror("Error while opening the file.\n");
      return(0);
   }

  //  printf("The contents of %s file are:\n", file_name);
  fseek(fp, 0L, SEEK_END); 
  
  // calculating the size of the file 
  long int size_of_file = ftell(fp); 
  close(fp);

  fp = fopen(file_name, "r"); // read mode
   if (fp == NULL)
   {
      perror("Error while opening the file.\n");
      return(0);
   }

  int count=0; //for packet data size <1000
  int frag_no=0;
  unsigned char packet_file_data[1000];
  unsigned char ch;
  unsigned char int_buf[1000], ack_buf[1000];
  struct packet pac;
  pac.total_frag = size_of_file/1000 + 1;
  
  pac.filename = file_name;
 
 unsigned char read_buf[1000000];
  int size = fread(&read_buf, 1, size_of_file, fp);


  //  while((ch = fgetc(fp)) != EOF){
  for(int i= 0; i<size_of_file ; i++){
      ch = read_buf[i];   

      printf(" %c",(char *)ch);
      packet_file_data[count] = ch;
      count = count +1;

      if(count == 1000){
        packet_file_data[count] = 0;

        //send a packet
        count = 0;

		//fill up the struct
        pac.frag_no = frag_no;
        pac.size = 1000;
        str_copy(&pac.filedata , &packet_file_data, 1000);

        //forming buf for send
        buffer[0] = 'a';
        sprintf(int_buf, "%d", pac.total_frag);
        strcat(buffer, int_buf);
        strcat(buffer,":");

        sprintf(int_buf, "%d", pac.frag_no);
        strcat(buffer, int_buf);
        strcat(buffer,":");

        sprintf(int_buf, "%d", pac.size);
        strcat(buffer, int_buf);
        strcat(buffer,":");
      
        strcat(buffer, pac.filename);
        strcat(buffer,":");

        // printf("loop: %s \n\n", buf);
        str_cat(&buffer, &pac.filedata);

        // printf("loop2: %s \n\n", buffer);
        // printf("%s",pac.filedata);

        // buffer = "4:0:3500:foo.txt:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa ";
		    // buffer = "hello ";
        //printf("testaaaa\n");
        printf("loop33333333333333333:%s \n\n", buffer);
        if (send(sockfd, buffer, 1100-1, 0) == -1)
				perror("send");
        //increment
        frag_no++;
        str_clean(&packet_file_data, 1000);
        str_clean(&buffer, 1100);


        //wait for ack
        if ((numbytes = recv(sockfd, ack_buf, 1000, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';
        printf("client: received '%s'\n",ack_buf);
        while(strcmp(ack_buf,"ack: packet received")!=0){}

      }
  }

   //last PACKET

    pac.frag_no = frag_no;
    pac.size = size_of_file - (pac.total_frag-1)*1000;
    str_copy(&pac.filedata , &packet_file_data, pac.size);

        //forming buf for send
    buffer[0] = '0';
    sprintf(int_buf, "%d", pac.total_frag);
    strcat(buffer, int_buf);
    strcat(buffer,":");

    sprintf(int_buf, "%d", pac.frag_no);
    strcat(buffer, int_buf);
    strcat(buffer,":");

    sprintf(int_buf, "%d", pac.size);
    strcat(buffer, int_buf);
    strcat(buffer,":");
     
    strcat(buffer, pac.filename);
    strcat(buffer,":");

        // printf("loop: %s \n\n", buf);
    str_cat(&buffer, &pac.filedata);


    // str_clean(buffer,1100);
    // char buffer2[1100] = "1:0:51:foo.txt:helow";
    // printf("2last:");
  //  printf("last: %s \n\n", buffer);

   if (send(sockfd, buffer, 1100-1, 0) == -1)
				perror("send");
   fclose(fp);
	////////////////////////////////////////////////////////////
    //receive responde
    // if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	//     perror("recv");
	//     exit(1);
	// }
	// buf[numbytes] = '\0';
	// printf("client: received '%s'\n",buf);
	
	
    // timespec_get(&ts_end, TIME_UTC);
    // char buff[100];
    // strftime(buff, sizeof buff, "%D %T", gmtime(&ts_end.tv_sec));
    //printf("Current time: %s.%09ld UTC\n", buff, ts_end.tv_nsec);
	// printf("RRT: 0.%09ld second\n", ts_end.tv_nsec - ts.tv_nsec);

	// if(!strcmp(buf, "yes")){
	// 	printf("A file transfer can start.\n");
		
		
	// }
  // fclose(fp2);

	close(sockfd);

	return 0;
}

void str_copy(unsigned char* dest,unsigned char* start, int size){
  // int i;
  // for (i = 0; start[i] != 0; i++) {
    // printf("copy:size:%d  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",size);
  for (int i = 0; i < size; i++) {
    dest[i] = start[i];
    // printf(" %u", dest[i]);
  
  }
  //  printf("\n\n");
  // *size = i+1;
}

void str_clean(unsigned char* var, int size){

  for (int i = 0; i < size; i++) {
    var[i] = 0;
    // printf("%s\n", start[i]);
  
  }
}

void str_cat(unsigned char* dest,unsigned char* start){
  int i;
  for ( i =0; dest[i] != '\0'; i++){
      // printf("%d ", i);
  }

  for (int j = 0; j < 1000; j++) {
    dest[i] = start[j];
    i++;
    // printf("%s\n", start[i]);
  }
  dest[i] = 0;
}