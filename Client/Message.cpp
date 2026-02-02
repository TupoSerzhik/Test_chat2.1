#include "Message.h"

Message::Message(const std::string& text, const std::string& sender, const std::string& receiver) : _text(text), _sender(sender), _receiver(receiver)
{
}

std::string Message::getText() const
{
	return _text;
}

std::string Message::getSender() const
{
	return _sender;
}

std::string Message::getReceiver() const
{
	return _receiver;
}

void Message::setText(const std::string& text)
{
	_text = text;
}

void Message::setSender(const std::string& sender)
{
	_sender = sender;
}

void Message::setReceiver(const std::string& receiver)
{
	_receiver = receiver;
}

std::string Message::serialize() const
{
	return  _sender + "|" + _receiver + "|" + _text;
}

Message Message::deserialize(const std::string& data)
{
	size_t pos1 = data.find('|');
	size_t pos2 = data.find('|',pos1 + 1);

	if (pos1 != std::string::npos && pos2 != std::string::npos)
	{
		std::string sender = data.substr(0, pos1);
		std::string receiver = data.substr(pos1 + 1, pos2 -1);
		std::string text = data.substr(pos2 + 1);
		return Message(text, sender, receiver);
	}
	return Message("", "", "");
}


