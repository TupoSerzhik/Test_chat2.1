#include "User.h"
#include <sstream>

User::User(const std::string& login, const std::string& pass) : _login(login), _pass(pass)
{
}

std::string User::getLogin() const
{
	return _login;
}

std::string User::getPass() const
{
	return _pass;
}

void User::setLogin(const std::string& login)
{
	_login = login;
}

void User::setPass(const std::string& pass)
{
	_pass = pass;
}

std::string User::serialize() const
{
	return _login + "|" + _pass;
}

User User::deserialize(const std::string& data)
{
	size_t pos = data.find('|');
	if (pos != std::string::npos)
	{
		std::string login = data.substr(0, pos);
		std::string pass = data.substr(pos + 1);
		return User(login, pass);
	}
	return User("Error: the object could not be restored from the serialized format", "");
}


