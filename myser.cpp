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
#define BACKLOG 10
#define MAXBUFLEN 100
#define SIZE 500
using namespace std;

int file_size(char *name);

class mySocket {
	protected:
		int sockfd;
		struct sockaddr_in my_addr;

	public:
		mySocket(int fd);
		~mySocket();
		void handleconnection();
};

class httpServer:public mySocket {
	public:
	httpServer(int port)
		:mySocket(port){}

	void handleconnection();
};

void mySocket::handleconnection() {
	cout << "handles in base class " << endl;
	struct sockaddr_in their_addr;
	fd_set readfds;
	fd_set master;
	int addr_len;
	char buf[100],ar[100];
	int numbytes,s,r;
	int sockfd1;
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
				cin >> ar;
				if (send(sockfd1,ar,numbytes,0) == -1) {
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
/*
class httpServer:public mySocket {
	public:
	httpServer(int port)
		:mySocket(port){}

	virtual void handleconnection();
};
*/
void httpServer::handleconnection() {
	int sin_size,numbytes,new_fd,fd,n,len,pid;
	struct sockaddr_in their_addr;
	char buf[SIZE];
	char get[5],path[50];
	char *sendbuf = NULL;
	char *tempbuf = NULL;
	char *spr = NULL;

	cout << "handles in derived class " << endl;
	sin_size = sizeof(struct sockaddr_in);
	while (1) {
		sleep(1);
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
			cout << buf << endl;//request from client
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

			}

			close(new_fd);
		}
		else {
		wait();
		}
		cout << "fork done" << endl;
		close(new_fd);
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

int main(int argc,char **argv) {
	if(argc != 2) {
		cerr << "check usage : port" << endl;
		exit(1);
	}
	int port = atoi(argv[1]);
//	mySocket one(port);
//	one.handleconnection();
	httpServer two(port);
//	mySocket *p = new httpServer;
//	p->handleconnection();
	two.handleconnection();

}



