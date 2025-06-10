#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<tchar.h>
#include<thread>
#include<vector>

using namespace std;

// this is to tell that we are using winSock lib of version 2 
#pragma comment(lib,"ws2_32.lib")

/*
  -> we are going to use winsocket lib we have to do following steps
  1. initialize the winsock lib
  2. create the socket
  3. get ip and port
  4. then bind the ip/port with the socket
  5. listen on the socket
  6. after liesting we have accpet and at accept the call will be blocking call so here the call will be stopped
     until we get call from connection from client
  7. then we recv and send
  8. the after we close the socket
  9. at last we cleanup the winsock lib

*/

// first start the winsock
bool initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

// this is the function that will be run in a thread to interact with the client
void interactWithClient(SOCKET clientSocket,vector<SOCKET>&clients) {
	// send/recv client 
	cout << "Client connected" << endl;

	// now we have to create a buffer to recv and send data
	char buffer[4096] = { 0 };
	while (1) {
		int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesrecvd <= 0) {
			cout << "client disconnected" << endl;
			break; // if we get 0 or negative value then client has disconnected
		}
		// now we have to make a string from the buffer
		string message(buffer, bytesrecvd);
		cout << "message from client : " << message << endl;

		// now we have to send the message to each client except the one who sent the message
		for (auto client : clients)
		{
			if (client != clientSocket) {
				send(client, message.c_str(), message.size(), 0);
			}
		}
	}

	// now we can remove the client from the vector 
	auto it = find(clients.begin(), clients.end(), clientSocket);
	if(it != clients.end()) {
		clients.erase(it); // remove the client from the vector
	}
	
	closesocket(clientSocket);
}

int main() {
	// Step 1 :  initialization
	if (!initialize()) {
		cout << "winsock initialization falied" << endl;
		return 1;
	}
	
	cout << "Server Program" << endl;

	// Step 2 : create socket
	// socket(Addresstype,kind of protocol(TCP/UDP), actual protocol)
	// socket(IPV4,TCP,default)
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) {
		cout << "socket creation failed" << endl;
		return 1;
	}
	
	// Step 3 : get ip and port
	// create address and structure
	int port = 12345;
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port); // we cant put int directly to sinport so use htons host to network


	// now convert the ip address to binary format
	// convert ip add (0.0.0.0) and put it inside the sin_family in binray form
	if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
		cout << "setting address structure failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	
	// Step 4 : bind the socket
	// but we have reinterpret_cast to convert sockaddr_in to sockaddr
	if (bind(listenSocket, reinterpret_cast<sockaddr*> (&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << "bind failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Step 5 : listen
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "listen failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Server is listening on port " << port << endl;

	// Step 6 : accept 
	vector<SOCKET> clients; // to store the client sockets
	// by below we can make many accept calls with multithreading
	while (1) {
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET) {
			cout << "accept failed" << endl;
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		clients.push_back(clientSocket); // store the client socket

		// if we reach here then we have a client connected to us
		// now if we want to connect with other client we can use multithreading 
		// so that we can handle multiple clients
		thread t1(interactWithClient, clientSocket, std::ref(clients));
		t1.detach(); // detach the thread so that it runs independently
	}
	
	
	closesocket(listenSocket);

	// Last Step : cleanup
	WSACleanup();
	return 0;
}