#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <string>
#include <iostream>
#include <algorithm>
#include <set>
#include "servermain.h"
using namespace std;
#define BUF_SIZE 2048
#define SERVER_MAIN_CLIENT_PORT 33368
int main(void)
{
    int socket_desc;
    struct sockaddr_in server_addr;
    // char server_message[BUF_SIZE], client_message[BUF_SIZE];
    char country[BUF_SIZE];
    char buf[BUF_SIZE];
    // Clean buffers:
    // memset(server_message,'\0',sizeof(server_message));
    // memset(client_message,'\0',sizeof(client_message));
    
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    printf("Client is up and running.\n");
    string countryToSearch;
    string userID;
    printf("Enter country name:");
    cin >> countryToSearch;
    printf("Enter user ID: ");
    cin >> userID;
    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_MAIN_CLIENT_PORT);
    //server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    
    for(;;){
        // Get input from the user:
        sprintf(buf, (countryToSearch + " " + userID).c_str());
        // Send the message to server:
        sendto(socket_desc, buf, strlen(buf), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        printf("Client has sent %s and User %s to Main Server using TCP over port %d\n",countryToSearch.c_str(), userID.c_str(), SERVER_MAIN_CLIENT_PORT);

        // // Receive the server's response:
        bzero(buf, sizeof(buf));
        read(socket_desc, buf, sizeof(buf));

        if(strcmp(buf, "CountryNotFound") == 0){
            //<Country Name>: Not found
            printf("%s: Not found\n",countryToSearch.c_str());
        }else if(strcmp(buf, "UserNotFound") == 0){
            //User <user ID>: Not found
            printf("User %s: Not found\n", userID.c_str());
        }else{
            //User<user ID1>, User<user ID2>, ... is/are possible friend(s) of User<user ID> in <Country Name>
            string msg = string(buf);
            msg = getClientPossibleFriends(msg);
            msg += "is/are possible friend(s) of User" + userID + " in " + countryToSearch + "\n";
            printf(msg.c_str());
        }
        printf("-----Start a new request-----\n");
        printf("Enter country name:");
        cin >> countryToSearch;
        printf("Enter user ID: ");
        cin >> userID;

    }//for loop end
    
    close(socket_desc);
    
    return 0;
}
