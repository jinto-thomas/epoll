#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#define MYPORT 8080
#define BACKLOG 5
#define MAXDATASIZE 500
void send_error(char spr[],int fd);
char *select_header(char *name);
int file_size(char *name);
int directory_check(char *name,char *spr,int fd);
int main(void)
{
	int sockfd, new_fd, sin_size,numbytes, n, fd, filelen, f_s;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	char *s_h;
	char get[5],path[50],root[100],buf[MAXDATASIZE],spr[MAXDATASIZE],ar[204800];
	int yes = 1;
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		perror("socket");
		exit(1);
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero),'\0',8);
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	printf("listening\n");
	if (listen(sockfd,BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	while (1) {
		sin_size = sizeof(struct sockaddr_in);
		strcpy(root,"/home/jinto");
		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr,&sin_size)) == -1) {
			perror("accept");
			exit(1);
		}
		printf("*****************server : got connection from %s********************\n",inet_ntoa(their_addr.sin_addr));
		if (!fork()) {
			close(sockfd);
			if ((numbytes = recv(new_fd,buf,MAXDATASIZE-1,0)) == -1)
				perror("recv");
			buf[numbytes] = '\0';
			printf("numbytes received is %d\n",numbytes);
			printf("%s\n",buf); //request from client
			sscanf(buf,"%s%s",get,path);
			printf("sscanf %s\n",path);
			chdir(root);//choose home directory
			if (strcmp(path,"/") == 0) {
				if ((fd = open("index.html",O_RDONLY)) == -1) {
					perror("openindex");
					exit(1);
				}
				filelen = file_size("index.html");
				strcat(path,"index.html");
				printf("strcpy into y %s\n",path);
			}
			else {
				if (!directory_check((path+1),spr,new_fd)) {
					if ((fd = open(path+1,O_RDONLY)) == -1) {
						perror("openfile!");
						exit(1);
					}
					filelen = file_size(path+1);
					s_h = select_header(path+1);
				}
				else {
					strcat(root,path);
					chdir(root);
					printf("=====>>>>> root now is %s\n",root);
					if ((fd = open("index.html",O_RDONLY)) == -1) {
						perror("openfile!");
						exit(1);
					}
					filelen = file_size("index.html");
					s_h = select_header("index.html");
				}
			}
			n = read(fd,ar,filelen);//file read into ar
			// bzero(&root,sizeof(root));
			sprintf(spr,"HTTP/1.1 200 OK\nContent-length: %d\nContent-Type: %s\n\n",filelen,s_h);
			printf("===== from spr ====\n %s\n",spr);
			char *reply = (char *)malloc(strlen(spr)+filelen);
			strcpy(reply,spr);
			printf("===================\n");
			bcopy(ar,reply+strlen(spr),filelen);
			printf("++++++after bcpy+++++\n%s\n",reply);
			printf("+++++++++++++++++++++\n");
			if (send(new_fd,reply,strlen(spr)+filelen,0) == -1)
				perror("send");
			free(reply);
			close(new_fd);
			exit(0);
		}
		close(new_fd);
	}
	return 0;
}
char *select_header(char *name)
{
	char *ext = strrchr(name,'.');
	if ((strcmp(ext,".html") == 0) || strcmp(ext,".txt") == 0)
		return "text/html";
	else if (strcmp(ext,".jpg") == 0)
		return "image/jpg";
	else if (strcmp(ext,".png") == 0)
		return "image/png";
	else if (strcmp(ext,".gif") == 0)
		return "image/gif";
	return "text/html";//if nothing specified(for index file)
}
int file_size(char *name)
{
	struct stat t;
	if (stat(name,&t) == -1) {
		perror("stat");
		exit(1);
	}
	return t.st_size;
}
int directory_check(char *name,char *spr,int fd)
{
	struct stat t;
	if (stat(name,&t) == -1) {
		send_error(spr,fd);
		perror("stat-dir");
		exit(1);
	}
	return t.st_mode & S_IFDIR;
}
void send_error(char *spr,int fd)
{
	sprintf(spr,"HTTP/1.1 404 Not Found\nContent-length: 57\nContent-Type: text/html\n\n<html><title>ERROR!</title><body>Permission Denied</body>");
	send(fd,spr,strlen(spr),0);
}
