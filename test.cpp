#include <iostream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string>
#define BACKLOG 10
#define MAXBUFLEN 100
#define SIZE 500
#define MAXEVENTS 10
using namespace std;

int file_size(char *name);

class mySocket {
	protected:
		int sockfd;
		struct sockaddr_in my_addr;

	public:
		mySocket(int fd);
		~mySocket();
		virtual void handleconnection() {
			cout << "***************************" << endl;
			cout << "*handled in the base class*" << endl;
			cout << "***************************" << endl;
		}
};

class httpServer:public mySocket {
	public:
	httpServer(int port)
		:mySocket(port){}

	void handleconnection();
};

class simpleChat:public mySocket {
	public:
	simpleChat(int port)
		:mySocket(port){}

	void handleconnection();
};

class epollBroadcastServer:public mySocket {
	public:
	epollBroadcastServer(int port)
		:mySocket(port){}

	void handleconnection();
};

void simpleChat::handleconnection() {
	cout << "handles in simpleChat class " << endl;
	struct sockaddr_in their_addr;
	fd_set readfds;
	fd_set master;
	int addr_len;
	char buf[100],ar[100];
	int numbytes,s,r;
	int sockfd1;
	string str;

	while (1) {
		if ((sockfd1 = accept(sockfd,(struct sockaddr *)&their_addr,(socklen_t *)&addr_len)) == -1) {
			cerr<< "accept err : " << strerror(errno) <<endl;
			exit(1);
		}
		addr_len = sizeof(struct sockaddr);
		FD_ZERO(&readfds);
		FD_ZERO(&master);
		FD_SET(0,&master);
		FD_SET(sockfd1,&master);
		s = sockfd1+1;
		while (1) {
			readfds = master;
			r = select(s,&readfds,NULL,NULL,NULL);
			if (FD_ISSET(sockfd1,&readfds)) {
	//			cout <<"recieved"<<endl;
				if ((numbytes = recv(sockfd1,buf,sizeof(buf),0)) > 0) {
					cout << "got packet from " << inet_ntoa(their_addr.sin_addr) << endl;
					cout << "packet is " << numbytes << "long" << endl;
					buf[numbytes] = '\0';
					cout << "[message from neighbour] :" << buf << endl;
				}
			}
			else if (FD_ISSET(0,&readfds)) {
			//	cout << "[sent]" << endl;
				getline(cin,str);
		//		fgets(ar,MAXBUFLEN-1,0);
				if (send(sockfd1,str.c_str(),str.length(),0) == -1) {
					cerr << "send err "<< strerror(errno) << endl;
					exit(1);
				}
				cout << "send!!!!!!!!!!!!" << endl;
			}
		}
//		close(sockfd1);
	}
}


mySocket::mySocket(int fd) {
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) { 
		  cerr <<"sockfd error "<<strerror(errno) << endl;
		  exit(1);
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(fd);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero),'\0',8);
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		cerr << "bind err "<<strerror(errno) << endl;
		exit(1);
	}

	int yes = 1;
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		cerr << "reuser err " <<strerror(errno) << endl;
		exit(1);
	}
	
	if (listen(sockfd,BACKLOG) == -1) {
		cerr << "listen err " << strerror(errno) << endl;
		exit(1);
	}
}

mySocket::~mySocket() {
	close(sockfd);
	cout << "destroyed " << endl;
}

void httpServer::handleconnection() {
	int sin_size,numbytes,new_fd,fd,n,len,pid;
	struct sockaddr_in their_addr;
	char buf[SIZE];
	char get[5],path[50];
	char *sendbuf = NULL;
	char *tempbuf = NULL;
	char *spr = NULL;

	cout << "handles in httpServer class " << endl;
	sin_size = sizeof(struct sockaddr_in);
	while (1) {
//		sleep(1);
		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr,(socklen_t *)&sin_size)) == -1) {
			cerr << "accept err " << strerror(errno) << endl;
			exit(0);
			cout <<"exit " <<endl;
			
		}
		cout << endl;
	
		cout << "new_fd is " << new_fd << endl;
		cout << "conncetion from :: " << inet_ntoa(their_addr.sin_addr) << endl;
		cout << "forking started " << endl;
		
		pid = fork();
		if (pid == 0) {
			close(sockfd);
			if ((numbytes = recv(new_fd,buf,SIZE-1,0)) == -1)
				cerr << "recv err " << strerror(errno) << endl;
			buf[numbytes] = '\0';
			cout << "REQUEST :: " << buf << endl;//request from client
			sscanf(buf,"%s%s",get,path);
			if (strcmp(path,"/index.html") == 0) {
				fd = open("index.html",O_RDONLY);
				len = file_size((char *)"index.html");
				sendbuf = new char[len+1];
				n = read(fd,sendbuf,len);
				spr = new char[60+len];
				sprintf(spr,"HTTP/1.1 200 OK\nContent-length: %d\nContent-Type: text/html\n\n",len);
				bcopy(sendbuf,spr+60,len);

				close(fd);
				if (send(new_fd,spr,strlen(spr),0) == -1)
					cerr << "send err :" << strerror(errno) << endl;

				delete [] sendbuf;
				delete [] spr;
				cout << "deleted " << endl;

			}

			close(new_fd);
		}
		else {
			wait();
			close(new_fd);
			cout << "parent closed " << endl;
		}
//		close(new_fd);
		cout << "fork done" << endl;
//		close(new_fd);
	}
}

int file_size(char *name)
{
	struct stat t;
	if (stat(name,&t) == -1) {
		cerr << "file open err "<< strerror(errno) << endl;;
		exit(1);
	}
	return t.st_size;
}


void epollBroadcastServer::handleconnection() {
	int epollfd,addr_len,nfds,client_fd,conn_sock,temp_fd;
	struct epoll_event ev;
	struct epoll_event events[MAXEVENTS];
	int k = 0,n,j,numbytes;
	int ar[MAXEVENTS];
	struct sockaddr_in their_addr;
	char buf[MAXBUFLEN];

	memset(ar,0,sizeof(ar));

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
				printf("can accept new conn\n");
				conn_sock = accept(sockfd,(struct sockaddr *)&their_addr,(socklen_t *)&addr_len);
				if (conn_sock == -1) {
					perror("accept ");
					exit(1);
				}
				ar[k++] = conn_sock;
				printf("new connection is going on\n");

				ev.events = EPOLLIN | EPOLLET;
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
				if (events[n].events & EPOLLIN) {
					printf("recive part\n");
					if ((numbytes = recv(client_fd,buf,sizeof(buf),0)) > 0) {
						buf[numbytes] = '\0';
						
						printf("============= %s\n",buf);
						printf("*********the client_fd %d\n",client_fd);
						temp_fd = client_fd;
						for(j = 0; j < k ;j++) {
							printf("===j val %d\n",j);
							if (ar[j] != temp_fd)
								if (send(ar[j],buf,numbytes,0) == -1) {
									perror("send error");
								}
						}
//						if (epoll_ctl(epollfd,EPOLL_CTL_DEL,client_fd,&ev) < 0) {
//							perror("epoll ctlll:");
//						}
//						close(client_fd);
					
					}
				}
					else {
						printf("type to send\n");
						if (send(client_fd,buf,numbytes,0) == -1) {
							perror("send error");
						}
					}
			//	}
			}
		}
	}
}




int main(int argc,char **argv) {
	if(argc != 2) {
		cerr << "check usage : port" << endl;
		exit(1);
	}
	int port = atoi(argv[1]);
//	mySocket one(port);
//	one.handleconnection();
//	httpServer two(port);
//	mySocket *p = new httpServer;
//	p->handleconnection();
//	two.handleconnection();

//	mySocket *p = new simpleChat(port);
//	mySocket *p = new httpServer(port);
	mySocket *p = new epollBroadcastServer(port);
	p->handleconnection();

}



