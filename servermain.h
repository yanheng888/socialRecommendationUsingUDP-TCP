
#pragma once

#include <unordered_map>
#include <string>
#include <iostream>
#include <algorithm>
#include <set>
#include <fstream>
#include <sstream>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // for close
#include "vector"
#define SERVER_MAIN_PORT	32368	/* hard-coded port number */
#define SERVER_MAIN_CLIENT_PORT 33368 /* hard-coded port number */
#define SERVER_A_PORT	30368	/* hard-coded port number */
#define SERVER_B_PORT	31368	/* hard-coded port number */
#define FILE_A "input/dataA.txt"
#define FILE_B "input/dataB.txt"
#define BUF_SIZE 2048

//split the string by given delimiter
std::vector<std::string> split(std::string& s, char delimiter){
     std::vector <std::string> tokens;
      
    // stringstream class check1
    std::stringstream check1(s);
      
    std::string intermediate;
      
    // Tokenizing w.r.t. space ' '
    while(getline(check1, intermediate, delimiter))
    {
        tokens.push_back(intermediate);
    }
    return tokens;
  
}

//map2 is for main server
std::unordered_map<std::string,std::unordered_map<std::string,std::set<std::string> > > getMap2(std::unordered_map<std::string, std::string> map){
    //map is map<country, line of userIDs>
    //map2 <country, map<userID, same interest group of userId>>
    std::unordered_map<std::string,std::unordered_map<std::string,std::set<std::string> > > map2;
    for(auto entry: map){  
        std::string country = entry.first; 
        std::vector<std::string> lines = split(entry.second, '\n');
        std::unordered_map<std::string,std::set<std::string> >  tempMap;
        for(auto line: lines){
            //nums = userIds which are in same group
            std::vector<std::string> nums = split(line,' ');
            std::set<std::string> tempSet;
            for(auto num: nums){
                tempSet.insert(num);
            }
            for(auto num: nums){
                if(tempMap.find(num) == tempMap.end()){
					std::set<std::string> s2(tempSet.begin(), tempSet.end());
                    tempMap[num] = s2;
                }else{
                    for(auto ele: tempSet){
                        tempMap[num].insert(ele);
                    }
                }
				tempMap[num].erase(num);
            }
        }
        map2[country] = tempMap;
    }
    return map2;
}

//trim the space in the beginning and ending of a string
std::string trimLeadingTrailingSpace(std::string stripString)
{
	while(!stripString.empty() && std::isspace(*stripString.begin()))
    	stripString.erase(stripString.begin());

	while(!stripString.empty() && std::isspace(*stripString.rbegin()))
		stripString.erase(stripString.length()-1);
	return stripString;
}

// get all distinct integer from a string which may include duplicate integer
std::set<std::string> removeDupString(std::string str)
{
	// Used to split string around spaces.
	std::istringstream ss(str);
    std::set<std::string> st;
	std::string word; // for storing each word

	// Traverse through all words
	// while loop till we get
	// strings to store in string word
	while (ss >> word)
	{
		st.insert(word);
	}
    return st;
}

/**
 * extract data from file, save country as key list of users as value
 * the value is a long string of all distinct users and first integer of
 * the string is the number of distinct users in this country
 */
std::unordered_map<std::string, std::string> getTextData(std::string fName){
    std::unordered_map<std::string, std::string> map;

	std:: ifstream file(fName);
	std::string country = "";
	std::string users;
	std::string line;
	std::vector<std::string> usersGroup;
	std::vector<std::string> v;
	while(getline(file, line)){
		v.push_back(line);
	}
	std::string str = "";
	for(int i = 0; i < v.size(); i++){
		if(std::isdigit(v[i][0])){
			str += v[i] + "\n";
		}else{
			if(country != ""){
				map[country] = str;
				str = "";
			}
			country = v[i];
		}
	}
	file.close();
	map[country] = str;
	
    return map;
}

//get all keys from map and concatenate all keys
std::string getMapAllKeyString(std::unordered_map<std::string, std::string> map)
{
    std::string str = "";
    for(auto it = map.begin(); it != map.end(); ++it)
	{
		str += " " + it->first;		
	}
    return str;
}
//create a FileDescriptor
int createFD(int fd)
{
	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");
	return fd;
}

//create a FileDescriptor for TCP
int createTCPFD(){
	return socket(AF_INET, SOCK_STREAM, 0);
}

//set up address
struct sockaddr_in setUpAddress(struct sockaddr_in addr, char* ip, int port)
{
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (inet_aton(ip, &addr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	return addr;
}

// main server use this method to save country name in map as key. If this country belongs to server A
// the value is "0", otherwise the value is "1"
std::unordered_map<std::string, std::string> saveCountryList(std::unordered_map<std::string, std::string> map,std::string flag, std::string countryList)
{
    countryList = trimLeadingTrailingSpace(countryList);
    auto countrySt = removeDupString(countryList);
    for(auto it = countrySt.begin(); it != countrySt.end(); it++)
    {
        map[*(it)] = flag;
    }
    return map;
}

/**
 * usersNumber: the number of distinct users in a country
 * usersList: a string includes all distinct users in a country
 * 
 * */


//main server will use this method to print backend server and their corresponding countrys
void printResponsibleServer(std::unordered_map<std::string, std::string> map)
{
	printf("Server A\n");
	for(auto it = map.begin(); it != map.end(); it++)
	{
		if(it->second == "0"){
			printf((it->first + "\n").c_str());
		}
	}
	printf("\nServer B\n");
	for(auto it = map.begin(); it != map.end(); it++)
	{
		if(it->second == "1"){
			printf((it->first + "\n").c_str());
		}
	}
}


std::string getPossibleFriends(std::set<std::string> st){
	std::string res = "";
	for(auto ele: st){
		res += ele + ", ";
	}
	if(res == "") return res;
	res.pop_back();
	res.pop_back();
	return res;
}

std::string getClientPossibleFriends(std::string s){
	auto v = split(s, ',');
	std::string res = "";
	for(auto ele: v){
		res += "User" + ele + ", ";
	}
	if(res == "") return res;
	res.pop_back();
	res.pop_back();
	return res;
}