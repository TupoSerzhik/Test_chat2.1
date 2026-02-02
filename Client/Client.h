#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Message.h"
#include <sstream>

#pragma comment (lib, "ws2_32.lib")

class Client
{
private:
	SOCKET clientSocket;
	std::string serverIP;
	int serverPort;
	std::string currentUser;
	bool connected;

	bool initialiizeWinsock();
	bool connectToServer();
	bool sendRequest(const std::string& request);
	std::string receiveResponse();

public:
	Client(const std::string& ip = "127.0.0.1", int port = 8080);
	~Client();
	bool connect();
	void disconnect();
	bool login(const std::string& username, const std::string& password);
	bool registerUser(const std::string& username, const std::string& password);
	bool sendMessage(const std::string& recipient, const std::string& message);
	bool broadcastMessage(const std::string& message);
	std::vector<Message> checkMessages();
	std::vector<std::string> listUsers();
	bool logout();
	bool isConnected() const;
	std::string getCurrentUser() const;
};

