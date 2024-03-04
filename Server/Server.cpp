#include "Server.h"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <cstring>

void Server::Init() {
    if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address {};
    int opt = 1;

    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(m_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(m_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

void Server::Start() {
    while (true) {
        int new_socket;
        struct sockaddr_in address {};
        int addrlen = sizeof(address);
        if ((new_socket = accept(m_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::cout << "New client connected" << std::endl;
        std::thread client_thread(&Server::HandleClient, this, new_socket);
        client_thread.detach();
    }
}

void Server::HandleClient(int client_socket) {
    char buffer[1024] = {0};
    while (true) {
        if (read(client_socket, buffer, 1024) == 0) {
            std::cout << "Client disconnected" << std::endl;
            m_topics.erase(m_clients[client_socket]);
            m_clients.erase(client_socket);
            close(client_socket);
            break;
        }

        std::string message(buffer);
        if (message.substr(0, 9) == "SUBSCRIBE") {
            std::string topicName = message.substr(10);
            m_topics[topicName].push_back(client_socket);
            m_clients[client_socket] = topicName;
            std::cout << "Client subscribed to topic: " << topicName << std::endl;
            message = "SUCCESS";
            send(client_socket, message.c_str(), message.length(), 0);
            memset(buffer, 0, 1024);

        } else {
            ForwardMessage(buffer, client_socket);
            memset(buffer, 0, 1024);
        }
    }
}

void Server::ForwardMessage(const std::string& message, int sender_socket) {
    std::cout << "massage: " << message << std::endl;
    std::string topicName = m_clients[sender_socket];

    auto topicIt = m_topics.find(topicName);
    if (topicIt != m_topics.end()) {
        for (int subscriber_socket : topicIt->second) {
            if (subscriber_socket != sender_socket) {
                send(subscriber_socket, message.c_str(), message.length(), 0);
            }
        }
    }
}