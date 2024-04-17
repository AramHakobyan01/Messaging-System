#include "Client.h"

#include <iostream>

Client::Client(int client_fd) :
    fd(client_fd) {}

int Client::getFD() {
    return fd;
}

Commands Client::processMessage(const Message& message) {
    std::string message1(message.data, message.size);
    std::cout << "message\t" << message1 << std::endl;

    int command = (static_cast<unsigned char>(message.data[0]) << 24) |
                       (static_cast<unsigned char>(message.data[1]) << 16) |
                       (static_cast<unsigned char>(message.data[2]) << 8)  |
                       (static_cast<unsigned char>(message.data[3]));

    std::cout << "command " << command << std::endl;
    return (Commands)command;
}