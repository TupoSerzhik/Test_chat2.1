#include "Server.h"
#include <iostream>
#include <string>

int main()
{
	Server server;
	if (!server.initialize(8080))
	{
		std::cerr << "Server initialization failed" << std::endl;
		return 1;
	}
	
	std::cout << "Chat Server started on port 8080" << std::endl;
	std::cout << "Press 'q' to quit" << std::endl;

	server.start();
	
	std::cout << "Server stopped" << std::endl;
	
	return 0;
}
