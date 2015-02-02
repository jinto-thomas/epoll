#include <iostream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#define MYPORT 8080
#define BACKLOG 10
#define SIZE 500

using namespace std;

int file_size(char *name);

class mySocket {
	int sockfd;
	struct sockaddr_in my_addr;

	public:
	mySocket();
	void binder();
	void reuser(int &yes);
	void listener();
	void newconnection();
	~mySocket();
};

mySocket::mySocket() {
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		cerr <<"sockfd error "<<strerror(errno) << endl;
		exit(1);
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero),'\0',8);

}

mySocket::~mySocket() {
	close(sockfd);
	cout << "destructor" << endl;
}

void mySocket::binder() {
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		cerr << "bind err "<<strerror(errno) << endl;
		exit(1);
	}
}

void mySocket::reuser(int &yes) {
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		cerr << "reuser err " <<strerror(errno) << endl;
		exit(1);
	}
}

void mySocket::listener() {
	if (listen(sockfd,BACKLOG) == -1) {
		cerr << "listen err " << strerror(errno) << endl;
		exit(1);
	}
}

void mySocket::newconnection() {
	int sin_size,numbytes,new_fd,fd,n,len,pid;
	struct sockaddr_in their_addr;
	char buf[SIZE];
	char get[5],path[50];
	char *sendbuf = NULL;
	char *tempbuf = NULL;
	char *spr = NULL;
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

int main()
{
	int yes = 1;
	mySocket one;
	one.binder();
	one.reuser(yes);
	one.listener();
	one.newconnection();
	return 0;
}


