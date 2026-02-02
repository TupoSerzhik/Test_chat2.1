#pragma once

#include <string>
#include <vector>
#include "Message.h"
#include "AuthManager.h"

class MessageService
{
private:
	AuthManager& authManager;
	std::vector<Message> messages;
	std::string messageFilename = "messages.dat";

	void loadMessagesFromFile();
	void saveMessagesToFile();
	void setFilePermissions(const std::string& filename);

public:
	MessageService(AuthManager& authManager);
	~MessageService();

	bool sendPrivateMessage(const std::string& sender, const std::string& recipient, const std::string& message);
	void broadcastMessage(const std::string& sender, const std::string& message);
	std::vector<Message> getUserMessage(const std::string& username) const;
	void clearUserMessages(const std::string& username);
};




