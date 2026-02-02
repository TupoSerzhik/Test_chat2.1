#pragma once

#include <vector>
#include <string>

class User
{
private:
	std::string _login;
	std::string _pass;

public:
	User() = default;
	User(const std::string& login, const std::string& pass);

	std::string getLogin() const;
	std::string getPass() const;

	void setLogin(const std::string& login);
	void setPass(const std::string& pass);

	std::string serialize() const;
	static User deserialize(const std::string& data);

};

