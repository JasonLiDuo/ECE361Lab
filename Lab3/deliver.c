/*
** deliver.c -- a stream socket deliver demo
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
#include <stdbool.h>
#include <math.h>

#define PORT "3501" // the port deliver will be connecting to 
#define MAXDATASIZE 1100 // max number of bytes we can get at once 
#define ALPHA 0.125
#define BETA 0.25

void str_copy(unsigned char* dest,unsigned char* start, int size);
void str_clean(unsigned char* var, int size);
void str_cat(unsigned char* dest,unsigned char* start);


clock_t start, end, timerstart, currenttime;

double RTT,SRTT, RTTVAR, RTO;

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
	}//IPv4
	return &(((struct sockaddr_in6*)sa)->sin6_addr);//IPv6
}

int main(int argc, char *argv[]){
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
    char file_name[MAXDATASIZE];

  //if user typed other than 3 arguments, give hints
	if (argc != 3){
	    fprintf(stderr,"usage: deliver server_address port\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("deliver: socket");
			continue;
		}
		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			perror("deliver: connect");
			close(sockfd);
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "deliver: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("deliver: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

  printf("please enter file name in format：ftp <file name>\nftp ");
  scanf("%s", file_name);
  printf("you enter: %s ",file_name);
  printf("\n");

  unsigned char buffer[1100];
  FILE *fp;
  fp = fopen(file_name, "r"); // read mode
  if (fp == NULL){
    perror("Error while opening the file.\n");
    return(0);
  }

  fseek(fp, 0L, SEEK_END); 
  
  // calculating the size of the file 
  long int size_of_file = ftell(fp); 
  close(fp);

  fp = fopen(file_name, "r"); // read mode
  if(fp == NULL){
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

  for(int i= 0; i<size_of_file ; i++){
    ch = read_buf[i];
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

      str_cat(&buffer, &pac.filedata);

      RTO = 1;
      
      start = clock();
      if (send(sockfd, buffer, 1100-1, 0) == -1){
				perror("send");
      }
      printf("%Lf", (long double)(start));

      while(clock()<(start+RTO*CLOCKS_PER_SEC)){
        //wait for ack
        if ((numbytes = recv(sockfd, ack_buf, 1000, 0)) == -1) {      
          perror("recv");
          exit(1);
        }  
      }
      end = clock();
      if(strcmp(ack_buf,"ack: packet received")!=0){
        start = clock();
        if (send(sockfd, buffer, 1100-1, 0) == -1){
           perror("send");
        }
  
        while(clock()<(start+RTO*CLOCKS_PER_SEC)){
            //wait for ack
           if ((numbytes = recv(sockfd, ack_buf, 1000, 0)) == -1) {      
              perror("recv");
              exit(1);
           }          
        }
        end = clock();
        if(strcmp(ack_buf,"ack: packet received")!=0){
          start = clock();
          if (send(sockfd, buffer, 1100-1, 0) == -1){
            perror("send");
          }
  
            while(clock()<(start+RTO*CLOCKS_PER_SEC)){
              //wait for ack
              if ((numbytes = recv(sockfd, ack_buf, 1000, 0)) == -1) {      
                perror("recv");
                exit(1);
              }
            }
            if(strcmp(ack_buf,"ack: packet received")!=0){
              printf("Connection lost");
              return 0;
            }
          } 
        }

          buf[numbytes] = '\0';
          printf("deliver: received '%s'\n",ack_buf);
          RTT = (double) (end - start) / CLOCKS_PER_SEC;
          printf("RTT: %f\n", RTT);
          SRTT = RTT;
          printf("SRTT: %f\n", SRTT);
          RTTVAR = RTT/2;
          printf("RTTVAR: %f\n", RTTVAR);
          RTO = SRTT + 4*RTTVAR;
          if(RTO<1){
            RTO = 1;
          }
          printf("ddd");
          printf("RTO: %f\n", RTO);
        }
        //increment
        frag_no++;
        str_clean(&packet_file_data, 1000);
        str_clean(&buffer, 1100);
      
      //   start = clock();
      //   if (send(sockfd, buffer, 1100-1, 0) == -1){
			// 	  perror("send");
      //   }
      //   while(true){
      //     if(isRetransmission==false){
      //       fprintf(stderr,"aaa\n");
        
      //       while(clock()<(start+RTO*CLOCKS_PER_SEC)){
      //         //wait for ack
      //         if ((numbytes = recv(sockfd, ack_buf, 1000, 0)) == -1) {      
      //           perror("recv");
      //           exit(1);
      //         }
      //         fprintf(stderr,"aff\n");
      //       }
            
      //       fprintf(stderr,"swaaff\n");
      //       isRetransmission = true;
      //    }
      //    end = clock();
      //     if(isRetransmission==true){
      //       start = clock();
      //       if (send(sockfd, buffer, 1100-1, 0) == -1){
			// 	      perror("send");
      //         isRetransmission = false;
      //       }
      //     }
      //     frag_no++;
      //     str_clean(&packet_file_data, 1000);
      //     str_clean(&buffer, 1100);
      //   }
      //   buf[numbytes] = '\0';
      //   printf("deliver: received '%s'\n",ack_buf);
      //   // RTT calculated
      //   RTT = (double) (end - start) / CLOCKS_PER_SEC;
      //   printf("RTT: %f\n", RTT);
      //   RTTVAR = (1-BETA)*RTTVAR + BETA*fabs(SRTT - RTT);
      //   printf("RTTVAR: %f\n", RTTVAR);
      //   SRTT = (1-ALPHA)*SRTT + ALPHA*RTT;
      //   printf("SRTT: %f\n", SRTT);
      //   RTO = SRTT + 4*RTTVAR;
      //   if(RTO<1){
      //     RTO = 1;
      //   }
      //   printf("RTO: %f\n", RTO); 
      //   }
      //   while(strcmp(ack_buf,"ack: packet received")!=0){}
      // }
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

  str_cat(&buffer, &pac.filedata);

  if (send(sockfd, buffer, 1100-1, 0) == -1){
    start = clock();
		perror("send");
  }

  //wait for ack
  if ((numbytes = recv(sockfd, ack_buf, 1000, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  end = clock();
  buf[numbytes] = '\0';
  printf("deliver: received '%s'\n",ack_buf);
  // RTT calculated
  double RTT = (double) (end - start) / CLOCKS_PER_SEC;
  printf("RTT: %f\n", RTT);
  fclose(fp);
  
  close(sockfd);
	return 0;
}

//copy string function
void str_copy(unsigned char* dest,unsigned char* start, int size){
  for (int i = 0; i < size; i++) {
    dest[i] = start[i];
  }
}

//clear string function
void str_clean(unsigned char* var, int size){
  for (int i = 0; i < size; i++) {
    var[i] = 0;
  }
}

//concatenate 2 strings function
void str_cat(unsigned char* dest,unsigned char* start){
  int i=0;
  for(i=0;dest[i]!='\0';i++){
  }
  for(int j=0;j<1000;j++) {
    dest[i] = start[j];
    i++;
  }
  dest[i] = 0;
}