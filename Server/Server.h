#pragma once

#include <string>
#include <vector>
#include <map>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "AuthManager.h"
#include "MessageService.h"

#pragma comment(lib,"ws2_32.lib")

class Server
{
private:
	SOCKET serverSocket;
	AuthManager authManager;
	MessageService messageService;
	std::map<std::string, std::vector<Message>> messageQueue;
	bool running;

	void handleClient(SOCKET clientSocket);
	void processRequest(SOCKET clientSocket, const std::string& request);
	void sendResponse(SOCKET clientSocket, const std::string& response);
	void addMessageToQueue(const std::string& recipient, const Message& message);
	std::vector<Message> getMessagesForUser(const std::string& username);
public:
	Server();
	~Server();
	bool initialize(int port = 8080);
	void start();
	void stop();
};

