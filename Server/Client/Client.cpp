#include "Client.h"

#include <iostream>
#include <cstring>

Client::Client(int client_fd) :
    fd(client_fd) {}

int Client::getFD() {
    return fd;
}

Commands Client::getCommand(Message& message) {
    std::string message1(message.data + 4, message.size - 4);
    std::cout << "message\t" << message1 << std::endl;

    int command = (static_cast<unsigned char>(message.data[0]) << 24) |
                       (static_cast<unsigned char>(message.data[1]) << 16) |
                       (static_cast<unsigned char>(message.data[2]) << 8)  |
                       (static_cast<unsigned char>(message.data[3]));

    strcpy(message.data, message.data + 4);
    std::cout << "command " << command << std::endl;
    return (Commands)command;
}