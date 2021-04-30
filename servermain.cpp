#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "servermain.h"
#include <arpa/inet.h>
#include <unistd.h> // for close
#include <string>
#define NO_STDIO_REDIRECT
using namespace std;

int main(void)
{
	struct sockaddr_in addrServerMain;
	struct sockaddr_in addrServerA;
	struct sockaddr_in addrServerB;
	struct sockaddr_in addrClient;
	int fd;
	int TCPFD;
	char *ip = "127.0.0.1";	/* ip address */
	char buf[BUF_SIZE];

	/* create a file descriptor  */
	fd = createFD(fd);
	TCPFD = socket(AF_INET, SOCK_STREAM, 0);
	//set up main server address
	addrServerMain = setUpAddress(addrServerMain,ip,SERVER_MAIN_PORT);

	//bind socket
	if (bind(fd, (struct sockaddr *)&addrServerMain, sizeof(addrServerMain)) < 0) {
		perror("bind failed");
		return 0;
	}       

  
	printf("Main server is up and running.\n");

	////set up backend server A address
	addrServerA = setUpAddress(addrServerA,ip,SERVER_A_PORT);

	////set up backend server B address
	addrServerB = setUpAddress(addrServerB,ip,SERVER_B_PORT);

	////set up client server A address
	addrClient = setUpAddress(addrClient,ip,SERVER_MAIN_CLIENT_PORT);
   
	//first message from main server to ask serverA to send a list of countries it has
	sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&addrServerA, sizeof(addrServerA));

	//first message from main server to ask serverB to send a list of countries it has
	sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&addrServerB, sizeof(addrServerB));
	
	unordered_map<std::string, std::string> mainMap;
	//receive a list of countries from server A
	socklen_t fromlenA = sizeof(addrServerA);
	recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&addrServerA, &fromlenA);
	printf("Main server has received the country list from server A using UDP over port %d\n",SERVER_MAIN_PORT);
	//save backend server A country list to map and flag 0 each country
	mainMap = saveCountryList(mainMap,"0",string(buf));

	//receive a list of countries from server B
	memset((char *)&buf, 0, sizeof(buf));
	socklen_t fromlenB = sizeof(addrServerB);
	recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&addrServerB, &fromlenB);
	//save backend server B country list to map and flag 1 each country
	mainMap = saveCountryList(mainMap,"1",string(buf));
	printf("Main server has received the country list from server B using UDP over port %d\n",SERVER_MAIN_PORT);
	printResponsibleServer(mainMap);
	//bind tcp client
	if (bind(TCPFD, (struct sockaddr *)&addrClient, sizeof(addrClient)) < 0) {
		perror("bind failed");
		return 0;
	}   

	// Listen for clients:
    if(listen(TCPFD, 1) < 0){
        printf("Error while listening\n");
        return -1;
    }
    int addrlen = sizeof(addrClient);
	int childPid, new_client_socket;
	struct sockaddr_in new_client_addr;
	for(int i = 0; i < 2; i++){
		socklen_t new_client_addr_len = sizeof(new_client_addr);
		 new_client_socket = accept(TCPFD, (struct sockaddr *)&new_client_addr, &new_client_addr_len);
		 if((childPid = fork()) == 0){
			 //close(TCPFD);
			 while(1){
				bzero(buf, sizeof(buf));
				recv(new_client_socket,buf,1024,0);

				//get country to search and userID from client
				stringstream ss(buf);
				string countryToSearch;
				string userID;
				ss >> countryToSearch >> userID;
				printf("Main server has received the request on User %s in %s from client %d using TCP over port %d\n",
				userID.c_str(), countryToSearch.c_str(), i+1,SERVER_MAIN_CLIENT_PORT);
				
				//not find country to search
				if(mainMap.find(countryToSearch) == mainMap.end())
				{
					printf("%s does not show up in server A&B\n", countryToSearch.c_str());
					bzero(buf, sizeof(buf));
					sprintf(buf, "CountryNotFound"); // 0 means country not found
					socklen_t client_addr_len = sizeof(addrClient);

					sendto(new_client_socket, buf, strlen(buf), 0, (struct sockaddr *)&new_client_addr, sizeof(new_client_addr));

					printf("Main Server has sent “%s: Not found” to client%d using TCP over port %d\n",
					countryToSearch.c_str(), i+1,SERVER_MAIN_CLIENT_PORT);
				}
				else
				{
					string server = mainMap[countryToSearch] == "0" ? "A":"B";
					int port = server == "A" ? SERVER_A_PORT:SERVER_B_PORT;
					printf("%s shows up in server %s\n", countryToSearch.c_str(), server.c_str());

					if(server == "A"){
						sprintf(buf,(countryToSearch + " " + userID).c_str());
						sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&addrServerA, sizeof(addrServerA));

						printf("Main Server has sent request of User %s to server %s using UDP over port %d\n",
						userID.c_str(), server.c_str(), port);
						memset((char *)&buf, 0, sizeof(buf));
						recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&addrServerA, &fromlenA);
					}
					else{
						sprintf(buf,(countryToSearch + " " + userID).c_str());
						sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&addrServerB, sizeof(addrServerB));

						printf("The Main Server has sent request for %s to server %s using UDP over port %d\n",
						countryToSearch.c_str(), server.c_str(), port);
						memset((char *)&buf, 0, sizeof(buf));
						recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&addrServerB, &fromlenB);
					}
					if(strcmp(buf, "UserNotFound") == 0){
						printf("Main server has received “User %s: Not found” from server %s\n", userID.c_str(), server.c_str());
						sendto(new_client_socket, buf, strlen(buf), 0, (struct sockaddr *)&new_client_addr, sizeof(new_client_addr));
						printf("Main Server has sent message to client %d using TCP over %d\n", i+1, SERVER_MAIN_PORT);
					}else{
						printf("The Main server has received searching result(s) of %s from server%s\n",
						countryToSearch.c_str(), server.c_str());
						sendto(new_client_socket, buf, strlen(buf), 0, (struct sockaddr *)&new_client_addr, sizeof(new_client_addr));
						printf("Main Server has sent searching result(s) to client%d using TCP over port %d\n",i+1,SERVER_MAIN_CLIENT_PORT);
					}
					

				}

			 }
		 }
	}
	close(new_client_socket);

	close(fd);
	close(TCPFD);
	return 0;
}
