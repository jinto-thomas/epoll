#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define MAXEVENTS 10

#define MYPORT 4950
#define MAXBUFLEN 100

static int make_socket_non_blocking(int sockfd) {
	int getFlag,setFlag;

	getFlag = fcntl(sockfd,F_GETFL,0);

	if (getFlag == -1) {
		perror("fcntl ");
		return -1;
	}

	getFlag |= O_NONBLOCK;
	setFlag = fcntl(sockfd,F_GETFL,getFlag);

	if (setFlag == -1) {
		perror ("fcntl setFlag");
		return -1;
	}
	return 0;
}

int ar[MAXEVENTS];

int main(int argc,char **argv) {
	if (argc != 2) {
		perror("usage check " );
	exit(1);
	}

	int sockfd,addr_len,numbytes,j = 0;
	char buf[MAXBUFLEN];
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	//int sfd,efd,s;
	struct epoll_event ev;
	struct epoll_event events[MAXEVENTS];
	int conn_sock,client_fd,epollfd,nfds,n;
//	int ar[MAXEVENTS];
	static int k = 0;
	int temp_fd;
	memset(ar,0,sizeof(ar));
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		perror("socket:");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset (&(my_addr.sin_zero), '\0',8);

	if (bind(sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr)) == -1) {
		perror("bind : ");
		exit(1);
	}
	

	make_socket_non_blocking(sockfd);

	if (listen(sockfd,10) == -1) {
		perror("listen ");
		exit(1);
	}


	if ((epollfd = epoll_create(10)) == -1) {
		perror("epoll_ create");
		exit(1);
	}
	printf("epollfd %d\n",epollfd);
	addr_len = sizeof(struct sockaddr);
	
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = sockfd;
	if (epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&ev) == -1) {
		perror("epoll ctl:sockfd ");
		exit(1);
	}

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = 0;
	if (epoll_ctl(epollfd,EPOLL_CTL_ADD,0,&ev) == -1) {
		perror("epoll ctl:sockfd ");
		exit(1);
	}
	//events = calloc(MAXEVENTS,sizeof ev);
//	k = 0;
	for (;;) {
	
		printf("**in for loop**\n");
		if ((nfds = epoll_wait(epollfd,events,MAXEVENTS, -1)) == -1) {
			printf("**************\n");
			perror("epoll_wait " );
			exit(1);
		}
		printf("****nfds %d\n",nfds);
		for (n = 0;n < nfds; ++n) {
			client_fd = events[n].data.fd;
			if (client_fd == sockfd) {
//				printf("can accept new conn\n");
				conn_sock = accept(sockfd,(struct sockaddr *)&their_addr,(socklen_t *)&addr_len);
//				printf("new_fd is : %d\n",conn_sock);
				temp_fd = conn_sock;
				if (conn_sock == -1) {
					perror("accept ");
					exit(1);
				}

				ev.events = EPOLLIN;// | EPOLLET;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd,EPOLL_CTL_ADD,conn_sock,&ev) == -1) {
					perror("epoll_ctl : conn_sock");
					exit(1);
				}
			}
			else {
			
				if (events[n].events & EPOLLHUP) {
					if (epoll_ctl(epollfd,EPOLL_CTL_DEL,client_fd,&ev) < 0) {
					
						perror("epoll : ctl");
					}
					close(client_fd);
					break;
				}

				if (client_fd == 0) {
						gets(buf);
						if (send(temp_fd,buf,strlen(buf),0) == -1) {
							perror("send error");
						}
				 	
				}
				if (events[n].events & EPOLLIN) {
//					printf("recive part\n");
					if ((numbytes = recv(client_fd,buf,sizeof(buf),0)) > 0) {
						buf[numbytes] = '\0';
						
						printf("============= %s\n",buf);
//						printf("*********the client_fd %d\n",client_fd);
//							if (send(client_fd,buf,numbytes,0) == -1) {
//								perror("send error");
//							}
						}
//						if (epoll_ctl(epollfd,EPOLL_CTL_DEL,client_fd,&ev) < 0) {
//							perror("epoll ctlll:");
//						}
//						close(client_fd);
					
			//		}
				}
			
				/*		printf("type to send\n");
						gets(buf);
						if (send(client_fd,buf,strlen(buf),0) == -1) {
							perror("send error");
						}
			*/	//	}
			//	}
			}
		}
	}
}

