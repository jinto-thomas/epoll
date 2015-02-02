#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
using namespace std;

class myHostInfo {
	private:
	char *ip_addr;
	char *name;

	public:
	myHostInfo() {
		ip_addr = NULL;
		name = NULL;
	}

	myHostInfo(char *address) {
		struct hostent *h;
		h = gethostbyname(address);
		char *d = inet_ntoa(*((struct in_addr *)h->h_addr));
		ip_addr = new char[strlen(d)+1];
		strcpy(ip_addr,d);
		name = new char[strlen(h->h_name)+1];
		strcpy(name,h->h_name);
//		name = h->h_name;
	}

	void printHost() {
		cout << "Host name is : " << name << endl;
	}

	void printIp() {
		cout << "IP address is : " << ip_addr << endl;
	}

	~myHostInfo() {
		delete [] name;
		delete [] ip_addr;
		name = 0;
		ip_addr = 0;
	}

};


int main(int argc,char **argv) {
	myHostInfo one(argv[1]);
	one.printIp();
	one.printHost();
	return 0;
}
