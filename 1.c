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
#define MYPORT 4950
#define MAXBUFLEN 100

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in their_addr;
	struct hostent *he;
	int numbytes;
	char ar[100];
	char buf[MAXBUFLEN];
	int addr_len;
	if (argc != 2) {
		fprintf(stderr,"usage : talker hostname \n");
		exit(1);
	}
	if ((he=gethostbyname(argv[1])) == NULL) {
		perror("gethostbyname");
		exit(1);
	}
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		perror("socket");
		exit(1);
	}
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(MYPORT);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0',8);
	addr_len = sizeof(struct sockaddr);
	if(connect(sockfd,(struct sockaddr *)&their_addr,sizeof(struct sockaddr))== -1) {
		perror("connect");
		exit(1);
	}
	while (1) {
		printf("[Type the message to send] : ");
		gets(ar);
		if ((numbytes=sendto(sockfd,ar,strlen(ar),0,(struct sockaddr *)&their_addr,sizeof(struct sockaddr))) == -1) {
			perror("sendto");
			exit(1);
		}
		printf("sent %d bytes to %s\n",numbytes,inet_ntoa(their_addr.sin_addr));
		if ((numbytes = recvfrom(sockfd,buf,MAXBUFLEN-1,0,(struct sockaddr *)&their_addr,&addr_len)) == -1) {
			perror("reccl");
			exit(1);
		}
		buf[numbytes] = '\0';
		printf("[inbox] = %s\n",buf);
	}
	close(sockfd);
	return 0;
}

