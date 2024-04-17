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
    int command = static_cast<int>(Commands::SUBSCRIBE);
    std::string buffer(sizeof(command), '\0');
    buffer[0] = static_cast<char>((command >> 24) & 0xFF);
    buffer[1] = static_cast<char>((command >> 16) & 0xFF);
    buffer[2] = static_cast<char>((command >> 8) & 0xFF);
    buffer[3] = static_cast<char>(command & 0xFF);
    buffer.append(topic);

    send(sock, buffer.c_str(), buffer.length(), 0);
}

// Send data to the server
void Client::SendData(const std::string& data) {
    int command = static_cast<int>(Commands::SEND_DATA);
    std::string buffer(sizeof(command), '\0');
    buffer[0] = static_cast<char>((command >> 24) & 0xFF);
    buffer[1] = static_cast<char>((command >> 16) & 0xFF);
    buffer[2] = static_cast<char>((command >> 8) & 0xFF);
    buffer[3] = static_cast<char>(command & 0xFF);
    buffer.append(data);

    send(sock, buffer.c_str(), buffer.length(), 0);
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
