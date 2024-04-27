#include "Server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <thread>
#include <arpa/inet.h>

Server::Server() :
    listen_fd(0),
    socketManager(),
    serverConfiguration("config.yaml") {
}

Server::~Server() {
    if (listen_fd != -1) {
        socketManager.closeSocket(listen_fd);
    }
}

[[noreturn]] void Server::start() {
    initListener(); // Change parameters as needed

    while (true) {
        acceptConnection();
    }
}

void Server::initListener() {
    listen_fd = socketManager.createSocket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, serverConfiguration.getListeningInterface().c_str(), &serv_addr.sin_addr);
    serv_addr.sin_port = htons(serverConfiguration.getListeningPort());

    socketManager.bindSocket(listen_fd, reinterpret_cast<const struct sockaddr*>(&serv_addr), sizeof(serv_addr));

    socketManager.listenSocket(listen_fd, SOMAXCONN);
}

void Server::acceptConnection() {
    struct sockaddr_in client_addr{};
    socklen_t client_addrlen = sizeof(client_addr);
    int client_fd = socketManager.acceptSocket(listen_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addrlen);
    if (client_fd >= 0) {
        handleClient(client_fd);
    }
}

void Server::handleClient(int client_fd) {
    // Handle client operations asynchronously
    Client client(client_fd);
    std::thread receiveThread(&Server::receiveData, this, client);
    receiveThread.detach();
}

void Server::receiveData(Client client) {
    // Receive data asynchronously using io_uring
    while (true) {
        Message message(serverConfiguration.getBufferSize());
        message.size = socketManager.receiveData(client.getFD(), message.data, serverConfiguration.getBufferSize(), 0);
        if (message.size < 0) {
            std::cerr << "Receive failed for client_fd: " << client.getFD() << std::endl;
            return;
        } else if (message.size == 0) {
            std::cout << "Connection closed by client_fd: " << client.getFD() << std::endl;
            socketManager.closeSocket(client.getFD());
            return;
        } else {
            // Process received data
            processMessage(message, client);
        }
    }
}

void Server::sendData(int client_fd, const void* buf, size_t len) {
    // Send data asynchronously using io_uring
    int send_size = socketManager.sendData(client_fd, buf, len, 0);
    if (send_size < 0) {
        std::cerr << "Send failed for client_fd: " << client_fd << std::endl;
    }
}

void Server::processMessage(Message& message, Client client) {
    Commands command = client.getCommand(message);
    switch (command) {
        case Commands::SEND_DATA:
            sendData(client.getFD(), "barev client", 64);
            break;
        case Commands::SUBSCRIBE:
            sendData(client.getFD(), "barev client", 64);
            break;
        default:
            break;
    }
}