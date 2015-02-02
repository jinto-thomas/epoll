
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
#define MYPORT 4960
#define HISPORT 4950
#define MAXBUFLEN 100
int main(char argc,char **argv)
{
	if (argc != 2) {
		printf("usage : ip address\n");
		exit(1);
	}
	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int addr_len, numbytes;
	char buf[MAXBUFLEN];
	char ar[MAXBUFLEN];
	struct timeval tv;
	int r,s;
	fd_set readfds;
	fd_set master;
	struct hostent *he;

	if ((he = gethostbyname(argv[1])) == NULL) {
		perror("gethostbyname");
		exit(1);
	}


	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}


	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(MYPORT);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero),'\0',8);

	
	addr_len = sizeof(struct sockaddr);
	FD_ZERO(&readfds);
	FD_ZERO(&master);
	FD_SET(0,&master);
	FD_SET(sockfd,&master);
	s = sockfd+1;

	if(connect(sockfd,(struct sockaddr *)&their_addr,sizeof(struct sockaddr))== -1) {
		perror("connect");
		exit(1);
	}

	while (1) {
		readfds = master;
		r = select(s,&readfds,NULL,NULL,NULL);
		if (FD_ISSET(sockfd,&readfds)) {
			printf("recieved\n");
			if ((numbytes = recvfrom(sockfd,buf,MAXBUFLEN-1,0,(struct sockaddr *)&their_addr,&addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}
			printf("got packet from %s\n",inet_ntoa(their_addr.sin_addr));
			printf("packet is %d bytes long\n",numbytes);
			buf[numbytes] = '\0';
			printf("[message from neighbour] : \"%s\"\n",buf);
		}
		else if (FD_ISSET(0,&readfds)) {
			printf("sent\n");
			gets(ar);
			if ((numbytes = sendto(sockfd,ar,strlen(ar),0,(struct sockaddr *)&their_addr,sizeof(struct sockaddr))) == -1) {
				perror("send to client");
				exit(1);
			}
		}
	}
	close(sockfd);
	return 0;
}

