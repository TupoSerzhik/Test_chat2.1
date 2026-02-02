#pragma once
#include <string>

class Message
{
private:
	std::string _text;
	std::string _sender;
	std::string _receiver;

public:
	Message() = default;
	Message(const std::string& text, const std::string& sender, const std::string& receiver);

	std::string getText() const;
	std::string getSender() const;
	std::string getReceiver() const;

	void setText(const std::string& text);
	void setSender(const std::string& sender);
	void setReceiver(const std::string& receiver);

	std::string serialize() const;
	static Message deserialize(const std::string& data);
};

