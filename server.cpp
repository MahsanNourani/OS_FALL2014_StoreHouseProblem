#ifndef SERVER__
#define SERVER__ 

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <vector>
#include <cstdio>
#include <sstream>
#include <fstream>
#include "stuck.h"
#include "item.h"

#define READ 0
#define WRITE 1

#define SHMSZ 100
using namespace std;

int main(int argc, char *argv[])
{

	Stuck* stuck = new Stuck();
	FILE* myfile;

	// myfile.fopen ("myfiles.txt" ,"r");

	int status;
	struct addrinfo host_info;
	struct addrinfo *host_info_list;
	fd_set readfds, masterfds;
	struct timeval timeout;
	vector<int> fds;
	ssize_t bytes_recieved;
	vector<string> client_names;

	if (argc != 2)
	{
		cout << "wrong input" << endl;
		exit(0);
	}

	memset(&host_info, 0, sizeof host_info);

	host_info.ai_family = AF_INET;
	host_info.ai_socktype = SOCK_STREAM;
	host_info.ai_flags = AI_PASSIVE;

	status = getaddrinfo(NULL, argv[1], &host_info, &host_info_list);

	if (status != 0)
		cout << "getaddrinfo error" << gai_strerror(status);
	int sockfd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
	if (sockfd == -1)
		cout << "socket error" << endl;
	int yes = 1;
	status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	status = bind(sockfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
	if (status == -1)
		perror ("bind error!");
		// cout << "bind error" << endl;

	status = listen(sockfd, 5);
	if (status == -1)
		cout << "listen error" << endl;

	FD_ZERO(&masterfds);
	FD_SET(sockfd, &masterfds);
	int max_fd = sockfd;

	int pipe_fd_send[2];
	int pipe_fd_recieve[2];

	if (pipe (pipe_fd_send) < 0) { perror("could not create pipe! "); return 0; }
	if (pipe (pipe_fd_recieve) < 0) { perror ("could not create pipe! "); return 0; }
	
	//process 2 to process commands!

	int pid_prog2 = fork();
	//child of process 1 -process 2
	if (pid_prog2 == 0) {
		int shmid;
		key_t key = 2020;
		char* shm ,s;
		char c;
		//create segment!
		if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
		    perror("shmget");
		    exit(1);
		}
		//attach segment tou memory space
		if ((shm = (char*)shmat(shmid, NULL, 0)) == (char *) -1) {
		    perror("shmat");
		    exit(1);
		}


		int pid_prog3 = fork();
		//child of process 2 -process 3
		if (pid_prog3 == 0) {
			while (1) {
				while (*shm == '*') {
					// cerr << "oumad ounja vali meseke biroon nmire! :| " << *shm << endl;
					sleep (1);
				}
				// cerr << "shm contains: " << *shm << endl;
				myfile = fopen ("myfiles.txt" ,"w");
				if (myfile != NULL){
					// cerr << "file baze!\n"; 
					// sleep (10);
					// cerr << "stcuk.size() = " << stuck -> all_items.size() << endl;
					for (int i = 0; i < stuck -> all_items.size(); ++i)
					{
						string temp = "";
						stringstream ss;
						temp += stuck -> all_items[i] -> get_item_name() + " ";
						ss << stuck -> all_items[i] -> get_item_amount();
						temp += ss.str() + "\n";
						// cerr << "temp is : " << temp << endl;
						fputs (temp.c_str() ,myfile);
					}
					fclose(myfile);
					// cerr << "file ro bast!\n";
				}
				*shm = '*';
			}
		}
		//parent - process 2 
		else {
			int shmid2;
			char* shm2 ;
			char* s2;
			//locate the segment
			if ((shmid2 = shmget(key, SHMSZ, 0666)) < 0) {
			    perror("shmget");
			    exit(1);
			}
			//attach segment to data space
			if ((shm2 = (char*)shmat(shmid2, NULL, 0)) == (char *) -1) {
			    perror("shmat");
			    exit(1);
			}

			while (1) {
				/*here comes the third child!*/
				char _new_command [1000];
				bzero (_new_command ,1000);
				read (pipe_fd_send[READ] ,_new_command ,1000);
				string new_command (_new_command);
				// string recieve_ (recieve);
				string command = "";
				size_t pos = new_command.find(" ");
				command = new_command.substr(0 ,pos);
				string command_op = new_command.substr (new_command.find (" ") + 1);

				// char* command_operand = strtok (recieve ,NULL);
				if (command == "get_item_list") {
					write (pipe_fd_recieve[WRITE] ,"1" ,1000);
					string list = "";
					stringstream tmp;
					for (int i = 0; i < stuck -> all_items.size(); ++i) {
						stringstream ss;
						list += stuck -> all_items[i] -> get_item_name();
						list += " ";
						ss << stuck -> all_items[i] -> get_item_amount();
						list += ss.str();
						list += "#";
					}
					// cerr << "listi ke mire tou pipe = " << list << endl;
					write (pipe_fd_recieve[WRITE] ,list.c_str() ,1000);
				}
				else if (command == "get_item") {
					write (pipe_fd_recieve[WRITE] ,"2" ,1000);
					// cerr << "tou command ouadm!\n";
					string i_name = "";
					// cerr << command_op << " = command_op"<<endl;
					Item* temp = stuck -> search_item (command_op);
					if (temp -> get_item_name() == "NO Such Item!") i_name = "item does not exist!\n";
					else { 
						stringstream ss;ss << (temp -> get_item_amount());
						string temp = ss.str();
						i_name = "\namount of " + command_op + " is " + temp + "\n";
					}
					write (pipe_fd_recieve[WRITE] ,i_name.c_str() ,1000);
				}
				else if (command == "income" || command == "outcome") {
					write (pipe_fd_recieve[WRITE] ,"3" ,1000);
					string message;
					string ret;
					string income_name = command_op.substr (0 ,command_op.find(" "));

					int income_amount = atoi(command_op.substr(command_op.find(" ")+1).c_str());
					Item* temp = stuck -> search_item (income_name);
					if (temp -> get_item_name() == "NO Such Item!") {
						if (command == "outcome") message = "Item does not exist!\n";

						else {
							stuck -> all_items.push_back (new Item (income_name ,income_amount));
							cout << "here is the list:\n";
							stuck -> print_list();
							message = "item added successfuly! \n"; 
						}
					}

					else {
						if (command == "outcome") {
							if (temp -> get_item_amount() - income_amount < 0 ) message = "there is not enough of this item to sell!\n";
							else {
								temp -> remove_item_from_stuck (income_amount);
								message = "items removed successfuly!\n";
							}
						}
						else {
							temp -> add_item_to_stuck (income_amount);
							message = "item added successfuly! \n";
						}
							cerr << "here is the list:\n";
							stuck -> print_list();
					}
					write (pipe_fd_recieve[WRITE] ,message.c_str() ,1000);
				}
				else {
					write (pipe_fd_recieve[WRITE] ,"6" ,1000);
					cout << "my unknown command!" << endl;
					write (pipe_fd_recieve[WRITE] ,"unknown command!\n" ,1000);
				}
				s2 = shm2;
				*s2 = 'r';
				// *s2 = NULL;
			}
		}	
	}

	else {

		while(true) {
			int new_sockfd;
			struct sockaddr_storage their_addr;
			socklen_t addr_size = sizeof(their_addr);
			timeout.tv_sec = 8;
			timeout.tv_usec = 400;
			FD_ZERO(&readfds);
			readfds = masterfds;
			select (max_fd + 1, &readfds, NULL, NULL, &timeout);
			if (FD_ISSET(sockfd, &readfds))
			{
				new_sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
				if (new_sockfd == -1)
					cout << "accept error" << endl;

				cout << "new connection - " << new_sockfd << endl;

				if (new_sockfd > sockfd)
				{
					if (new_sockfd > max_fd)
						max_fd = new_sockfd;
				}
				char name[1000];
				bytes_recieved = recv(new_sockfd, name, 1000, 0);
				if (bytes_recieved == -1)
				{
					cout << "recieve failed on getting name" << endl;
					exit(EXIT_FAILURE);
				}
				name[bytes_recieved] = '\0';
				string client_name = name;
				int flag = true;
				for (int i = 0; i < client_names.size(); i++)
					if (client_names[i] == client_name)
						flag = false;
				if (flag)
				{
					send(new_sockfd, "no problem", 10, 0);
					FD_SET(new_sockfd, &masterfds);
					fds.push_back(new_sockfd);
					client_names.push_back(client_name);
				}
				else
				{
					send(new_sockfd, "name already exist", 23, 0);
				}
			}
			for (int i = 0; i < fds.size(); i++) {
				if (FD_ISSET(fds[i], &readfds))
				{
					char recieve[1000];
					bzero(recieve, 1000);
					bytes_recieved = read(fds[i], recieve, 1000);
					cerr << "command recieved : " << recieve << endl;
					if (bytes_recieved == -1)
					{
						cout << "recieve error" << endl;
						exit(EXIT_FAILURE);
					}
					recieve[bytes_recieved] = '\0';
					if (recieve[0] == 'e') {
						write (fds[i] ,"5" ,1000);
						close (fds[i]);
						FD_CLR(fds[i], &masterfds);
						cerr << "client " << client_names[i] << " exited!\n";
						client_names.erase(client_names.begin()+i);
						break;
					}
					write (pipe_fd_send[WRITE] ,recieve ,1000);	
					//read command number from pipe and send it to client
					bzero (recieve ,1000);
					read (pipe_fd_recieve[READ] ,recieve ,1000);
					write (fds[i] ,recieve ,1000);
					//read command log from pipe and write it to client
					bzero (recieve ,1000);
					sleep(2);
					read (pipe_fd_recieve[READ] ,recieve ,1000);
					// cerr << "chizi ke oun sare pipe recieve mishe : " << recieve << endl;
					write (fds[i] ,recieve ,1000);
				}
			}

		}
	}

	delete stuck;
	return 0;
}
#endif
