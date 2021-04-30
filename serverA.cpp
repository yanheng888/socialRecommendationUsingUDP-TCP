
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "servermain.h"

#include <unordered_map>
#include <string>
#include <iostream>
#include <algorithm>
#include <set>
#include <fstream>

using namespace std;


int main(int argc, char **argv)
{
	struct sockaddr_in addrServerA;	/* serverA address */
	struct sockaddr_in addrServerMain; /* server main address */

	int fd;				/* our socket */
	char buf[BUF_SIZE];	/* receive buffer */
	char *ip = "127.0.0.1";

	/* create a UDP socket */

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}
	addrServerA = setUpAddress(addrServerA,ip,SERVER_A_PORT);
	/* bind the socket to any valid IP address and a specific port */
	if (bind(fd, (struct sockaddr *)&addrServerA, sizeof(addrServerA)) < 0) {
		perror("bind failed");
		return 0;
	}
	
	// key: country name, value: a long string of all distinct users
	auto serverAmap = getTextData(FILE_A);
	auto map2 = getMap2(serverAmap);
	printf("Server A is up and running using UDP on port %d\n",SERVER_A_PORT);

	// first time to receive message from main server
	socklen_t fromlen = sizeof(addrServerMain);
	recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&addrServerMain, &fromlen);

	string str = getMapAllKeyString(serverAmap);
	sprintf(buf, str.c_str());

	//send a list of countries to main server
	printf("Server A has sent a country list to Main Server\n");
	if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&addrServerMain, sizeof(addrServerMain))==-1)
		perror("sendto");
	for(;;)
	{
		//buf is the country name to search
		memset((char *)&buf, 0, sizeof(buf));
		recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&addrServerMain, &fromlen);
		
		stringstream ss(buf);
		string countryToSearch;
		string userID;
		ss >> countryToSearch >> userID;
		printf("Server A has received a request for finding possible friends of User%s in %s\n",
		userID.c_str(),countryToSearch.c_str());

		//user not found
		if(map2[countryToSearch].find(userID) == map2[countryToSearch].end()){
			printf("User%s does not show up in %s\n", userID.c_str(), countryToSearch.c_str());
			bzero(buf,sizeof(buf));
			sprintf(buf,"UserNotFound");
			sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&addrServerMain, sizeof(addrServerMain));
			printf("Server A has sent “User%s not found” to Main Server\n", userID.c_str());
		}
		else{//user found
			string possibleFriends = getPossibleFriends(map2[countryToSearch][userID]);
			string xxx = "Server A found the following possible friends for User" + userID + " in " + countryToSearch + ": " + possibleFriends + "\n";
			printf(xxx.c_str());
			bzero(buf,sizeof(buf));
			sprintf(buf,possibleFriends.c_str());
			sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&addrServerMain, sizeof(addrServerMain));
			printf("Server A has sent the result to Main Server\n");
		}
	}

}
