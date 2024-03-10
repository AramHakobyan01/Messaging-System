#include "Client.h"

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <thread>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

Client& Client::GetInstance() {
    static Client instance;
    return instance;
}

// Connect to the server
void Client::ConnectToServer() {
    sockaddr_in serv_addr {};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error creating socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); // Use a constant for port number

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Start a separate thread to read data from the server
    std::thread readerThread(&Client::ReadData, this);
    readerThread.detach();
}

// Subscribe to a topic
void Client::Subscribe(const std::string& topic) {
    std::string subscribeMsg = "SUBSCRIBE " + topic;
    send(sock, subscribeMsg.c_str(), subscribeMsg.length(), 0);
    std::cout << "\nSubscribed to topic: " << topic << std::endl;
}

// Send data to the server
void Client::SendData(const std::string& data) {
    send(sock, data.c_str(), data.length(), 0);
}

// Read data from the server
void Client::ReadData() {
    char buffer[1024] = {0};
    while (true) {
        int res = read(sock, buffer, 1024);
        if (res > 0) {
            std::cout << "\nReceived from server: " << buffer << std::endl;
            memset(buffer, 0, 1024);
        } else if (res == 0) {
            std::cerr << "Server disconnected" << std::endl;
            break;
        } else {
            perror("Error reading from server");
            break;
        }
    }
}
