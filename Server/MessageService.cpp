#include "MessageService.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

void MessageService::loadMessagesFromFile()
{
	std::ifstream file(messageFilename);
	if (!file.is_open())
	{
		std::cout << "The message file was not found, a new one will be created." << std::endl;
		return;
	}
	std::string line;
	while (std::getline(file, line))
	{
		if (!line.empty())
		{
			Message msg = Message::deserialize(line);
			messages.push_back(msg);
		}
	}
	file.close();
}

void MessageService::saveMessagesToFile()
{
	std::ofstream file(messageFilename);
	if (!file.is_open())
	{
		std::cerr << "Error: couldn't open the message recording file!" << std::endl;
		return;
	}
	for (const auto& msg : messages)
	{
		file << msg.serialize() << std::endl;
	}
	file.close();
	setFilePermissions(messageFilename);
}

void MessageService::setFilePermissions(const std::string& filename)
{
#ifdef _WIN32
	_chmod(filename.c_str(), _S_IREAD | _S_IWRITE);
#else
	chmod(filename.c_str(), S_IRUSR | S_IWUSR);
#endif
}

MessageService::MessageService(AuthManager& authManager) : authManager(authManager)
{
	loadMessagesFromFile();
}

MessageService::~MessageService()
{
	saveMessagesToFile();
}

bool MessageService::sendPrivateMessage(const std::string& sender, const std::string& recipient, const std::string& message)
{
	messages.push_back(Message(message, sender, recipient));
	saveMessagesToFile();
	std::cout << "Sent to " << recipient << std::endl;
	return true;
}

void MessageService::broadcastMessage(const std::string& sender, const std::string& message)
{
	for (auto& user : authManager.getUsers())
	{
		if (user.getLogin() != sender)
		{
			messages.push_back(Message(message, sender, user.getLogin()));
		}
	}
	saveMessagesToFile();
	std::cout << "Sent to all users" << std::endl;
}

std::vector<Message> MessageService::getUserMessage(const std::string& username) const
{
	std::vector<Message> userMessages;
	for (const auto& msg : messages)
	{
		if (msg.getReceiver() == username)
		{
			userMessages.push_back(msg);
		}
	}
	return userMessages;
}

void MessageService::clearUserMessages(const std::string& username)
{
	std::vector<Message> remainingMessages;
	for (const auto& msg : messages)
	{
		if (msg.getReceiver() != username)
		{
			remainingMessages.push_back(msg);
		}
	}
	messages = remainingMessages;
	saveMessagesToFile();
}