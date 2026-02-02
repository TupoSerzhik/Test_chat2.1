#include "Client.h"
#include <iostream>
#include <string>
#include <sstream>

void cleanScreen()
{
	system("cls");
}

void waitForEnter()
{
	std::cout << " Press 'Enter' to continue...";
	std::cin.get();
	cleanScreen();
}

void showStartMenu()
{
	std::cout << std::endl << " =-=-=-=-=-=-=-=-=-=-=-=-=-=-= " << std::endl;
	std::cout << "         CLIENT CHAT         " << std::endl;
	std::cout << " =-=-=-=-=-=-=-=-=-=-=-=-=-=-= " << std::endl;
	std::cout << " > 1. Register" << std::endl;
	std::cout << " > 2. Log in" << std::endl;
	std::cout << " > 3. Exit" << std::endl;
	std::cout << " =-=-=-=-=-=-=-=-=-=-=-=-=-=-= " << std::endl;
	std::cout << " Select an option: ";
}

void ShowChatMenu(const std::string& username)
{
	std::cout << std::endl <<  " =-=-=-=-=-=-=-= Options =-=-=-=-=-=-=-= " << std::endl;
	std::cout << " Current user: " << username << std::endl;
	std::cout << " =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= " << std::endl;
	std::cout << " > 1. Send private message" << std::endl;
	std::cout << " > 2. Send broadcast message" << std::endl;
	std::cout << " > 3. Check my messages" << std::endl;
	std::cout << " > 4. Users list" << std::endl;
	std::cout << " > 5. Log out" << std::endl;
	std::cout << " =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= " << std::endl;
	std::cout << " Select an option: ";
}


int main()
{
	Client client;
	if (!client.connect())
	{
		std::cerr << " Couldn't connect to the server" << std::endl;
		waitForEnter();
		return 1;
	}

	bool running = true;
	while (running)
	{
		if (!client.isConnected())
		{
			std::cout << " Connection to the server is lost" << std::endl;
			break;
		}

		if (client.getCurrentUser().empty())
		{
			showStartMenu();

			char choice;
			std::cin >> choice;
			std::cin.get();
			
			switch (choice)
			{
			case '1':
			{
				std::string username, password;
				std::cout << " Enter username: ";
				std::getline(std::cin, username);
				std::cout <<" Enter password: ";
				std::getline(std::cin, password);
				
				if (client.registerUser(username, password))
				{
					std::cout << " Registration is successful!" << std::endl;
				}
				else
				{
					std::cout << " Registration error! The user already exists." << std::endl;
				}
				waitForEnter();
				break;
			}
			case '2':
			{
				std::string username, password;
				std::cout << " Enter username: " << std::endl << " > ";
				std::getline(std::cin, username);
				std::cout << " Enter password: " << std::endl << " > ";
				std::getline(std::cin, password);

				std::cout << " Trying to log in..." << std::endl;
				
				if (client.login(username, password))
				{
					std::cout << " Successful entry! Welcome, " << username << std::endl;
				}
				else
				{
					std::cout << " Login error! Invalid credentials." << std::endl;
				}
				waitForEnter();
				break;
			}
			case '3':
			{
				running = false;
				std::cout << " Exit the program..." << std::endl;
				break;
			}
			default:
			{
				std::cout << " Wrong choice!" << std::endl;
				break;
			}
			}
		}
		else
		{
			ShowChatMenu(client.getCurrentUser());

			char choice;
			std::cout << " > ";
			std::cin >> choice;
			//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			switch (choice)
			{
			case '1':
			{
					std::string recipient, message;
					std::cout << " Enter recipient: " << std::endl << " > ";
					std::getline(std::cin, recipient);
					std::cout << " Enter message: " << std::endl << " > ";
					std::getline(std::cin, message);

					if (client.sendMessage(recipient, message))
					{
						std::cout << " Message sent! " << std::endl;
					}
					else
					{
						std::cout << " Error sending the message!" << std::endl;
					}
					waitForEnter();
					break;
			}
			case '2':
			{
				std::string message;
				std::cout << " Enter broadcast message: " << std::endl << " > ";
				std::getline(std::cin, message);

				if (client.broadcastMessage(message))
				{
					std::cout << " Message sent to all users!" << std::endl;
				}
				else
				{
					std::cout << " Error sending the message!" << std::endl;
				}
				waitForEnter();
				break;
			}
			case '3':
			{
				std::cout << " Check the messages..." << std::endl;
				auto messages = client.checkMessages();
				if (messages.empty())
				{
					std::cout << " You don't have any new messages" << std::endl;
				}
				else
				{
					std::cout << std::endl << " =-=-=-=-=-= YOUR MESSAGE =-=-=-=-=-= " << std::endl;
					for (size_t i = 0; i < messages.size(); ++i)
					{
						std::cout << (i + 1) << ". From " << messages[i].getSender()
							<< ": " << messages[i].getText() << std::endl;
					}
					std::cout << " =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= " << std::endl;
				}
				waitForEnter();
				break;
			}
			case '4':
			{
				auto users = client.listUsers();
				std::cout << " =-=-=-=-= REGISTERED USERS =-=-=-=-=-=" << std::endl;
				for (const auto& user : users)
				{
					std::cout << " > " << user << std::endl;
				}
				std::cout << " =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= " << std::endl;
				waitForEnter();
				break;
			}
			case '5':
			{
				client.logout();
				std::cout << " Account logout completed! " << std::endl;
				waitForEnter();
				break;
			}
			default:
			{
				std::cout << " Wrong choice!" << std::endl;
				waitForEnter();
				break;
			}
			}
		}
	}

	client.disconnect();
	std::cout << " The program is completed." << std::endl;
	return 0;
}