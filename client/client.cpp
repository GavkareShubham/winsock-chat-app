#include<iostream>
#include<WS2tcpip.h>
#include<WinSock2.h>
#include<thread>
#include<string>

using namespace std;

#pragma comment(lib, "ws2_32.lib")
/*
	Steps for client :
	1. Initialize the winsock library
	2. Create a socket
	3. connect to the server
	4. Send and receive data
	5. Close the socket 
*/

bool initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void sendMsg(SOCKET s) {
	cout << "Enter your chat name : " << endl;
	string name;
	getline(cin, name);
	string message;

	while (1) {
		getline(cin, message);
		string msg = name + ": " + message;
		int bytesent = send(s, msg.c_str(), msg.size(), 0);
		if (bytesent == SOCKET_ERROR) {
			cerr << "Send failed!" << endl;
			break;
		}

		if(message == "exit") {
			cout << "Exiting chat..." << endl;
			break;
		}
	}
	closesocket(s);
	WSACleanup();
}

void receiveMsg(SOCKET s) {
	char buffer[4096];
	int recvlength;
	while (1)
	{
		recvlength = recv(s, buffer, sizeof(buffer) - 1, 0);
		if (recvlength == SOCKET_ERROR) {
			cerr << "Receive failed!" << endl;
			break;
		}
		if (recvlength == 0) {
			cout << "Server disconnected." << endl;
			break;
		}
		buffer[recvlength] = '\0'; // Null-terminate the string
		cout << "Received: " << buffer << endl;

	}
	closesocket(s);
	WSACleanup();

}

int main() {
	// Step 1: Initialize Winsock
	if(!initialize()) {
		cerr << "Winsock initialization failed!" << endl;
		return 1;
	}

	// Step 2: Create a socket
	SOCKET s;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s == INVALID_SOCKET) {
		cerr << "Socket creation failed!" << endl;
		WSACleanup();
		return 1;
	}
	int port = 12345;
	string serverAdress = "127.0.0.1"; // Localhost for testing
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port); // Port number
	// lets bind this
	inet_pton(AF_INET, serverAdress.c_str(), &(serveraddr.sin_addr));

	if(connect(s,reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cerr << "Connection to server failed!" << endl;
		closesocket(s);
		WSACleanup();
		return 1;
	}

	cout << "Connected to server!" << endl;
		
	// Step 4: Send and receive data done in threads

	//now we can create two theads for sending and receiving data
	thread senderThread(sendMsg, s);
	thread receiverThread(receiveMsg, s);

	senderThread.join(); // Detach the sender thread to run independently
	receiverThread.join(); // Detach the receiver thread to run independently


	return 0;
}