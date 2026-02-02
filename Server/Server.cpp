#include "Server.h"
#include <iostream>
#include <sstream>
#include <algorithm>

void Server::handleClient(SOCKET clientSocket)
{
	char buffer[4096];
	int bytesReceived;

	User* currentUser = nullptr;

	while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0)
	{
		buffer[bytesReceived] = '\0';
		std::string request(buffer);
		std::cout << "Received request: " << request << std::endl;

		processRequest(clientSocket, request);

		if (request.find("LOGOUT") == 0)
		{
			currentUser = nullptr;
			break;
		}
	}
	if (bytesReceived == 0)
	{
		std::cout << "Client disconnected" << std::endl;
	}
	else if (bytesReceived == SOCKET_ERROR)
	{
		std::cerr << "Received failed" << WSAGetLastError() << std::endl;
	}
	closesocket(clientSocket);
}

void Server::processRequest(SOCKET clientSocket, const std::string& request)
{
	std::istringstream iss(request);
	std::string command;
	iss >> command;

	if (command == "LOGIN")
	{
		std::string username, password;
		iss >> username >> password;

		User* user = authManager.login(username, password);
		if (user)
		{
			sendResponse(clientSocket, "LOGIN_SECCESS " + username + "\n");

			auto queuedMessages = getMessagesForUser(username);
			auto storedMessages = messageService.getUserMessage(username);

			if (!queuedMessages.empty() || !storedMessages.empty())
			{
				std::string messageResponse = "QUEUED_MESSAGES";
				for (const auto& msg : queuedMessages)
				{
					std::string formattedMsg = "From: " + msg.getSender() + ": " + msg.getText();
					messageResponse += "|" + formattedMsg;
				}
				for (const auto& msg : storedMessages)
				{
					std::string formattedMsg = "From: " + msg.getSender() + ": " + msg.getText();
					messageResponse += "|" + formattedMsg;
				}
				sendResponse(clientSocket, messageResponse + "\n");

				if (!storedMessages.empty())
				{
					messageService.clearUserMessages(username);
				}
			}
		}
		else
		{
			sendResponse(clientSocket, "LOGIN_FAILED\n");
		}
	}
	else if (command == "REGISTER")
	{
		std::string username, password;
		iss >> username >> password;

		if (authManager.registerUser(username, password))
		{
			sendResponse(clientSocket, "REGSTER_SECCESS\n");
		}
		else
		{
			sendResponse(clientSocket, "REGISTER_FAILED\n");
		}
	}
	else if (command == "SEND")
	{
		std::string sender, recipient, message;
		iss >> sender >> recipient;
		std::getline(iss, message);

		if (!message.empty() && message[0] == ' ')
		{
			message = message.substr(1);
		}

		Message msg(message, sender, recipient);
		addMessageToQueue(recipient, msg);

		sendResponse(clientSocket, "SEND_SUCCESS\n");
		std::cout << "Message queued for user: " << recipient << std::endl;
	}
	else if (command == "BROADCAST")
	{
		std::string sender, message;
		iss >> sender;
		std::getline(iss, message);
	
		if (!message.empty() && message[0] == ' ')
		{
			message = message.substr(1);
		}
		messageService.broadcastMessage(sender, message);

		for (const auto& user : authManager.getUsers())
		{
			if (user.getLogin() != sender)
			{
				Message msg(message, sender, user.getLogin());
				addMessageToQueue(user.getLogin(), msg);
			}
		}
		sendResponse(clientSocket, "BROADCAST_SUCCESS\n");
	}
	else if (command == "CHECK_MESSAGES")
	{
		std::string username;
		iss >> username;

		auto queuedMessages = getMessagesForUser(username);
		auto storedMessages = messageService.getUserMessage(username);

		std::vector<Message> allMessages;
		allMessages.insert(allMessages.end(), queuedMessages.begin(), queuedMessages.end());
		allMessages.insert(allMessages.end(), queuedMessages.begin(), storedMessages.end());
	
		if (!storedMessages.empty())
		{
			messageService.clearUserMessages(username);
		}
		if (allMessages.empty())
		{
			sendResponse(clientSocket, "NO_MESSAGES\n");
		}
		else
		{
			std::string response = "MESSAGES";
			for (const auto& msg : allMessages)
			{
				std::string formattedMsg = msg.getSender() + ": " + msg.getText();
				response += "|" + formattedMsg;
			}
			sendResponse(clientSocket, response + "\n");
		}
	}
	else if (command == "LISTUSERS")
	{
		std::string response = "USER_LIST";
		for (const auto& user : authManager.getUsers())
		{
			response += "|" + user.getLogin();
		}
		sendResponse(clientSocket, response + "\n");
	}
}

void Server::sendResponse(SOCKET clientSocket, const std::string& response)
{
	send(clientSocket, response.c_str(), response.length(), 0);
}

void Server::addMessageToQueue(const std::string& recipient, const Message& message)
{
	messageQueue[recipient].push_back(message);
	messageService.sendPrivateMessage(message.getSender(), recipient, message.getText());
}

std::vector<Message> Server::getMessagesForUser(const std::string& username)
{
	auto it = messageQueue.find(username);
	if (it != messageQueue.end())
	{
		std::vector<Message> messages = it->second;
		messageQueue.erase(it);
		return messages;
 	}
	return {};
}

Server::Server() : serverSocket(INVALID_SOCKET) , running(false), messageService(authManager)
{
}

Server::~Server()
{
	stop();
}

bool Server::initialize(int port)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "WSAStartup failed" << std::endl;
		return false;
	}
	
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		std::cerr << "Socket creation failed" << std::endl;
		WSACleanup();
		return false;
	}
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Bind failed" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}

	if (listen(serverSocket, 5) == SOCKET_ERROR)
	{
		std::cerr << "Listen failed" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}
	std::cout << "Server initialized on port " << port << std::endl;
	return true;
}

void Server::start()
{
	running = true;
	std::cout << "Server startead. Waiting for connections..." << std::endl;

	while (running)
	{
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(serverSocket, &readSet);

		timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int result = select(0, &readSet, NULL, NULL, &timeout);

		if (result == SOCKET_ERROR)
		{
			std::cerr << "Select failed" << std::endl;
			break;
		}

		if (result > 0 && FD_ISSET(serverSocket, &readSet))
		{
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(clientAddr);

			SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
			if (clientSocket == INVALID_SOCKET)
			{
				std::cerr << "Accept failed" << std::endl;
				continue;
			}

			char clientIP[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
			std::cout << "Client connected from " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

			handleClient(clientSocket);
		}
	}
}

void Server::stop()
{
	running = false;
	if (serverSocket != INVALID_SOCKET)
	{
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;
	}
	WSACleanup();
 }
