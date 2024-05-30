#include "Server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <thread>
#include <arpa/inet.h>

#include "Database/Database.h"

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
    initListener();
    std::thread responseThread(&Server::sendMessageToClients, this);
    responseThread.detach();

    while (true) {
        acceptConnection();
        sleep(1);
    }
}

void Server::initListener() {
    listen_fd = socketManager.createSocket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, serverConfiguration.getListeningInterface().c_str(), &serv_addr.sin_addr);
    serv_addr.sin_port = htons(serverConfiguration.getListeningPort());

    socketManager.bindSocket(listen_fd, reinterpret_cast<const struct sockaddr*>(&serv_addr), sizeof(serv_addr));

    socketManager.listenSocket(listen_fd, serverConfiguration.getMaxClientConnections() == 0 ? SOMAXCONN : serverConfiguration.getMaxClientConnections());
}

void Server::acceptConnection() {
    struct sockaddr_in client_addr{};
    socklen_t client_addrlen = sizeof(client_addr);
    Client client;
    client.fd = socketManager.acceptSocket(listen_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addrlen);
    if (client.fd > 0) {
        client.address = client_addr.sin_addr.s_addr;
        handleClient(client);
    }
}

void Server::handleClient(const Client& client) {
    // Handle client operations asynchronously
    std::thread receiveThread(&Server::receiveData, this, client);
    receiveThread.detach();
}

void Server::receiveData(Client client) {
    // Receive data asynchronously using io_uring
    while (true) {
        std::vector<uint8_t> data;
        int size = socketManager.receiveData(client.fd, data, serverConfiguration.getBufferSize(), 0);
        if (size < 0) {
            std::cerr << "Receive failed for client_fd: " << client.fd << std::endl;
            socketManager.closeSocket(client.fd);
            // TODO error handling
            return;
        } else if (size == 0) {
            std::cout << "Connection closed by client_fd: " << client.fd << std::endl;
            socketManager.closeSocket(client.fd);
            // TODO error handling
            return;
        } else {
            // Process received data
            if(!processMessage(data, client))
                return;
        }
    }
}

void Server::sendMessageToClients() {
    while (true) {
        std::vector<Message> messages = Database::get()->getUndeliveredMessages();
        for (const auto& message : messages) {
            std::vector<int> clients = Database::get()->getSubscribedClients(message.topic_id);
//            for (const auto& client : clients) {
//                if (sendDataToClient(client.fd, message.data)) {
//                    Database::get()->markMessageAsDelivered(message.id, client.id);
//                }
//            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void Server::sendSimpleResponse() {

}

bool Server::processMessage(std::vector<uint8_t>& data, Client& client) {
    Message message;
    if(!Utilities::get()->decodePacket(data, message)) {
        // TODO error handling
    }
    std::cout << "command " << message.command << std::endl;
    switch ((Commands)message.command) {
        case Commands::SEND_DATA: {
            if(message.topic_id <= 0) {
                // TODO error handling
            }
            std::vector<int> subscribedClients = Database::get()->getSubscribedClients(message.topic_id);

            if(subscribedClients.size() == 0) {
                // TODO error handling
            }

            if(client.id <= 0) {
                // TODO error handling
            }
            int message_id = 0;

            if(!Database::get()->storeMessage(message, message_id)) {
                // TODO error handling
            }

            for (int i = 0; i < subscribedClients.size(); i++) {
                if(subscribedClients.at(i) != client.id) {
                    if(Database::get()->trackDelivery(message_id, subscribedClients.at(i))) {
                        // TODO error handling
                    }
                }
            }

            // TODO send success
            break;
        }
        case Commands::CREATE_TOPIC: {
            std::string topic(reinterpret_cast<const char *>(message.data.data()), message.data.size());

            int topic_id = Database::get()->insertTopic(topic);

            if(topic_id == -1) {
                // TODO error handling
            }

            // TODO send to client topic id
        }
        case Commands::SUBSCRIBE: {
            std::string topic(reinterpret_cast<const char *>(message.data.data()), message.data.size());

            int topic_id = Database::get()->getTopicByName(topic);

            if(client.id > 0 && topic_id > 0) {
                if(!Database::get()->subscribeClientToTopic(client.id, topic_id)) {
                    // TODO error handling
                }
            } else {
                // TODO error handling
            }

            break;
        }
        case Commands::UNSUBSCRIBE: {
            std::string topic(reinterpret_cast<const char *>(message.data.data()), message.data.size());


            if(client.id > 0 && message.topic_id > 0) {
                if(!Database::get()->unsubscribeClientFromTopic(client.id, message.topic_id)) {
                    // TODO error handling
                }
            } else {
                // TODO error handling
            }
        }
        case Commands::CONNECT: {
            std::string data(reinterpret_cast<const char *>(message.data.data()), message.data.size());
            Database::get()->insertClient(data, client.address, client.id);
            // TODO error handling
        }
        case Commands::DISCONNECT: {
            Database::get()->unsubscribeClientFromAllTopic(client.id);
            Database::get()->deleteClientById(client.id);
            // TODO error handling
            socketManager.closeSocket(client.fd);
            return false;
        }
        default:
            break;
    }
    return true;
}