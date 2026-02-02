#include "Client.h"

bool Client::initialiizeWinsock()
{
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

bool Client::connectToServer()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    if (::connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Connerction to server failed" << std::endl;
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        return false;
    }
    return true;
}

bool Client::sendRequest(const std::string& request)
{
    if (!connected)
    {
        return false;
    }
    std::string fullRequest = request +"\n";
    return send(clientSocket, fullRequest.c_str(), fullRequest.length(), 0) != SOCKET_ERROR;
}

std::string Client::receiveResponse()
{
    if (!connected)
    {
        return "";
    }
    char buffer[4096];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';

        std::string response(buffer);
        size_t pos = response.find('\n');
        if (pos != std::string::npos)
        {
            response = response.substr(0, pos);
        }
        return response;
    }
    return "";
}


Client::Client(const std::string& ip, int port) : serverIP(ip), serverPort(port)
{
}

Client::~Client()
{
    disconnect();
}

bool Client::connect()
{
    if (!initialiizeWinsock())
    {
        std::cerr << "Winsock initialization failed" << std::endl;
        return false;
    }

    if (!connectToServer())
    {
        WSACleanup();
        return false;
    }
    connected = true;
    std::cout << "Connected to server " << serverIP << ": " << serverPort << std::endl;
    return true;
}

void Client::disconnect()
{
    if (connected)
    {
        logout();
    }
    if (clientSocket != INVALID_SOCKET)
    {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    WSACleanup();
    connected = false;
}

bool Client::login(const std::string& username, const std::string& password)
{
    std::string request = "LOGIN " + username + " " + password;
    if (!sendRequest(request))
    {
        return false;
    }

    std::string response = receiveResponse();
    if (response.find("LOGIN_SUCCESS") == 0)
    {
       currentUser = username;
       std::cout << "Successful entry! Checking the messages..." << std::endl;

       fd_set readSet;
       FD_ZERO(&readSet);
       FD_SET(clientSocket, &readSet);
    
       timeval timeout;
       timeout.tv_sec = 1;
       timeout.tv_usec = 0;
        
       int result = select(0, &readSet, NULL, NULL, &timeout);
       if (result > 0 && FD_ISSET(clientSocket, &readSet))
       {
           std::string queuedResponse = receiveResponse();
           if (queuedResponse.find("QUEUED_MESSAGES") == 0)
           {
               std::cout << std::endl << " =-=-= YOU HAVE NEW MESSAGES. =-=-=" << std::endl;
               std::istringstream iss(queuedResponse);
               std::string command;
               std::getline(iss, command, '|');

               std::string messageData;
               int messageCount = 0;
               while(std::getline(iss, messageData,'|'))
               {
                   if (!messageData.empty())
                   {
                       Message msg = Message::deserialize(messageData);
                       std::cout << "From " << msg.getSender() << ": " << msg.getText() << std::endl;
                       messageCount++;
                   }
               }
               std::cout << " =-=-= RECEIVED " << messageCount << " MESSAGES =-=-= " << std::endl;
           }
       }
       return true;
    }
    return false;
}

bool Client::registerUser(const std::string& username, const std::string& password)
{
    std::string request = "REGISTER " + username + " " + password;
    if (!sendRequest(request))
    {
        return false;
    }
    std::string response = receiveResponse();
    return response == "REGISTER_SECCESS";
}

bool Client::sendMessage(const std::string& recipient, const std::string& message)
{
    std::string request = "SEND " + currentUser + " " + recipient + " " + message;
    if (!sendRequest(request))
    {
        return false;
    }
    std::string response = receiveResponse();
    return response == "SEND_SUCCESS";
}

bool Client::broadcastMessage(const std::string& message)
{
    std::string request = "BROADCAST " + currentUser + " " + message;
    if (!sendRequest(request))
    {
        return false;
    }
    std::string response = receiveResponse();
    return response == "BROADCAST_SUCCESS";
}

std::vector<Message> Client::checkMessages()
{
    std::vector<Message> messages;

    std::string request = "CHECK_MESSAGES " + currentUser;
    if (!sendRequest(request))
    {
        return messages;
    }
    std::string response = receiveResponse();

    if (response == "NO_MESSAGES")
    {
        return messages;
    }
    else if (response.find("MESSAGES") == 0)
    {
        std::istringstream iss(response);
        std::string command;
        std::getline(iss, command, '|');

        std::string messageData;
        while (std::getline(iss, messageData, '|'))
        {
            if (!messageData.empty())
            {
                Message msg = Message::deserialize(messageData);
                messages.push_back(msg);
            }
        }
    }
    return messages;
}

std::vector<std::string> Client::listUsers()
{
    std::vector<std::string> users;
    
    std::string request = "LIST_USERS";
    if (!sendRequest(request))
    {
        return users;
    }

    std::string response = receiveResponse();
    if (response.find("USERS_LIST") == 0)
    {
        std::istringstream iss(response);
        std::string command;
        std::getline(iss, command, '|');

        std::string username;
        while (std::getline(iss, username, '|'))
        {
            if (!username.empty())
            {
                users.push_back(username);
            }
        }
    }
    return users;
}

bool Client::logout()
{
    if (!connected || currentUser.empty())
    {
        return true;
    }
    std::string request = "LOGOUT";
    sendRequest(request);
    std::string response = receiveResponse();

    currentUser.clear();
    return response == "LOGOUT_SECCESS";
}

bool Client::isConnected() const
{
    return connected;
}

std::string Client::getCurrentUser() const
{
    return currentUser;
}
