#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char *argv[])
{
	int status;
	struct addrinfo host_info;
	struct addrinfo *host_info_list;
	ssize_t bytes_recieved;

	if (argc != 3)
	{
		cout << "worng input" << endl;
		exit(EXIT_FAILURE);
	}
	//getting name, ip and port
	string name = argv[1];
	string arguman = argv[2];
	size_t pos = arguman.find(":");
	if (pos == -1)
	{
		cout << "worng input!" << endl;
		exit(EXIT_FAILURE);
	}
	string ip = arguman.substr(0, pos);
	const char* cip = ip.c_str();
	string port = arguman.substr(pos+1);
	const char* cport = port.c_str();
	//end

	//creating socket
	memset(&host_info, 0, sizeof host_info);

	host_info.ai_family = AF_INET;
	host_info.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(cip, cport, &host_info, &host_info_list);
	if (status)
	{
		cout << "getaddrinfo error" << gai_strerror(status);
		exit(EXIT_FAILURE);
	}

	int socketfd;
	socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
	if (socketfd == -1) 
	{
		cout << "socket error";
		exit(EXIT_FAILURE);
	}

	status = connect(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
	if (status == -1)
	{
		cout << "connect error";
		exit(EXIT_FAILURE);
	}
	//end of creating socket

	//sendig the name
	send(socketfd, name.c_str(), name.size(), 0);
	char get_back[1000];
	bytes_recieved = recv(socketfd, get_back, 1000, 0);
	if (bytes_recieved == -1)
	{
		cout << "recieved error on getting back the name" << endl;
		exit(EXIT_FAILURE);
	}
	get_back[bytes_recieved] = '\0';
	string result = get_back;
       	if (result == "name already exist")
	{
		cout << "name already exist!!!" << endl;
		exit(EXIT_FAILURE);
	}	
	//end

	//gettig input
	string input;
	while(getline(cin, input))
	{
		char command_number[1000] ,income[1000];
		const char *cinput = input.c_str();
		send(socketfd, cinput, strlen(cinput), 0);
		char trash[1000];
		//recv(socketfd, trash, 1000, 0);
		//cerr << trash << endl;
		bzero (command_number ,1000);
		ssize_t bytes_recieved = recv (socketfd, command_number ,1000 ,0);
		command_number[bytes_recieved] = '\0';
		cerr << command_number << endl;
		string temp(command_number);
		if (temp == "1") {
			// string size;
			// char packet[1000];
			// read (socketfd ,size.c_str() ,1000);
			// for (int i = 0; i < (atoi(size)%1000) +1; ++i) {
			// 	read (socketfd ,packet ,1000);
			// 	string temp ;
			// 	size_t s;
			// 	while(packet.find("#")){

			// 		cout << packet.substr(packet.find ("#"))
			// 	}
			// }
		}
		// else if (temp == "2") {}
		// else if (temp == "3") {
		// 	read (socketfd ,income ,1000);
		// 	cout << income;

		// }
		// else if (temp == "4") {}
		else if (temp == "5") {
			close (socketfd);
			return 0;
		}
		else {
			bzero (income ,1000);
			recv (socketfd ,income ,1000,0);
			// cerr << "command is :" << income << endl;
			cout << income << endl;
		}
	}
	//end of getting input

	//closing socket
	freeaddrinfo(host_info_list);
	close(socketfd);
}
