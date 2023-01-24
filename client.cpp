#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string.h>
#include <fstream>
#define _WIN32_WINNT 0x501
#include <ws2tcpip.h>
int main(int argc, char const* argv[]){
	int port;
	bool getPut;
	std::string filename;
	std::string host;
	std::string ipAddress;
	std::string request;
	int result;
	if(argc != 5){//make sure we have all of our arguments
		std::cout << "Error: improper arguments." << std::endl;
		return 1;}
	host = argv[1];
	port = std::stoi(argv[2]);//parse the port number from the argument
	getPut = strcmp(argv[3], "PUT");//true for get, false for put
	filename = argv[4];
	WSADATA wsaData;//structure that holds data from the windows sockets
	WSAStartup(MAKEWORD(2,0), &wsaData);//start WSA
	
	struct addrinfo *address;
	if(getaddrinfo(host.c_str(), argv[2], 0, &address) !=0){//get address and other info about the host
		std::cout << "Error resolving hostname." << std::endl;
		return 1;
	}
	
	SOCKET socket1 = socket(address->ai_family, address->ai_socktype, address->ai_protocol);//create socket object, with appropriate type and protocol
	if(connect(socket1, address->ai_addr, address->ai_addrlen) != 0){//connect, and fall into the if statement if it fails
		std::cout << "Error connecting to host." << std::endl;
		closesocket(socket1);
		WSACleanup();
		return 1;
	}
	if(getPut){//form the GET request
		request = "GET /" + filename + " HTTP/1.0\r\n" + "Host: " + host + ":" + std::to_string(port) + "\r\nConnection: close\r\n\r\n";
	}
	else{//form the PUT request
		request = "PUT /" + filename + " HTTP/1.0\r\n\r\n";
	}
	
	const char *requestBuffer = request.c_str();
	result = send(socket1, requestBuffer, request.size(), 0);//send request using the socket
	if(result == SOCKET_ERROR){
		std::cout<< "Error sending request." << std::endl;
		closesocket(socket1);
		WSACleanup();
		return 1;
	}
	if(!getPut){//send the file if it was a PUT request
		std::string fileContents;
		std::string line;
		std::ifstream file;
		file.open(filename);//open file
		while(getline(file, line)){//read file line by line into fileContents
			fileContents += line += "\n";
		}
		file.close();
		const char *fileBuffer = fileContents.c_str();
		result = send(socket1, fileBuffer, fileContents.size(), 0);//send the file
	}
	std::string response;
	char responseBuffer[50];
	while(true){//loop until we have the entire response
		result = recv(socket1, responseBuffer, sizeof(responseBuffer), 0);//receive response
		if (result == SOCKET_ERROR){
			closesocket(socket1);
			WSACleanup();
			return 1;
		}
		if(result == 0)//we've received it all, exit
			break;
		response += std::string(responseBuffer, result);//add the recently received characters to the full result string
		
	}
	std::cout << response << std::endl;//output the received document
	closesocket(socket1);
	WSACleanup();
	return 0;
}