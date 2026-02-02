#include "AuthManager.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

std::string AuthManager::hashPassword(const std::string& password) const
{
	return sha1.hash(password);
}

void AuthManager::loadUserFromFile()
{
	std::ifstream file(userFilename);
	if (!file.is_open())
	{
		std::cout << "The user file was not found, a new one will be created." << std::endl;
		return;
	}
	std::string line;
	while (std::getline(file, line))
	{
		if (!line.empty())
		{
			User user = User::deserialize(line);
			users.push_back(user);
		}
	}
	file.close();
}

void AuthManager::saveUserToFile()
{
	std::ofstream file(userFilename);
	if (!file.is_open())
	{
		std::cerr << "Error: couldn't open the file for writing users!" << std::endl;
		return;
	}
	for (const auto& user : users)
	{
		file << user.serialize() << std::endl;
	}
	file.close();

	setFilePermissions(userFilename);
}

void AuthManager::setFilePermissions(const std::string& filename)
{
#ifdef _WIN32
	_chmod(filename.c_str(), _S_IREAD | _S_IWRITE); // было _chmod
#else
	chmod(filename.c_str(), S_IRUSR | S_IWUSR);
#endif
}

AuthManager::AuthManager()
{
	loadUserFromFile();
	if (users.empty())
	{
		users.push_back(User("admin", sha1.hash("admin")));
		users.push_back(User("user1", sha1.hash("pass1")));
		users.push_back(User("user2", sha1.hash("pass2")));
		saveUserToFile();
	}
}

AuthManager::~AuthManager()
{
	saveUserToFile();
}

bool AuthManager::registerUser(const std::string& username, const std::string& password)
{
	for (const auto& user : users)
	{
		if (user.getLogin() == username)
		{
			return false;
		}
	}

	users.push_back(User(username, hashPassword(password)));
	saveUserToFile();
	
	return true;
}

User* AuthManager::login(const std::string& username, const std::string& password)
{
	std::string hashedPassword = hashPassword(password);
	for (auto& user : users)
	{
		if (user.getLogin() == username && user.getPass() == hashedPassword)
		{
			return &user;
		}
	}
	return nullptr;
}

User* AuthManager::findUser(const std::string& username)
{
	for (auto& user : users)
	{
		if (user.getLogin() == username)
		{
			return &user;
		}
	}
	return nullptr;
}

bool AuthManager::checkUser(const std::string& loginname)
{
	for (auto& user : users)
	{
		if (user.getLogin() == loginname)
		{
			return true;
		}
	}
	return false;
}

const std::vector<User> AuthManager::getUsers() const
{
	return users;
}

void AuthManager::listUsers() const
{
	std::cout << std::endl << " =-=-= Registered Users =-=-= " << std::endl;
	for (const auto& user : users)
	{
		std::cout << " > " << user.getLogin() << std::endl;
	}
}










