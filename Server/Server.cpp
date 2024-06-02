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
    serverConfiguration("config.yaml"),
    responseTriggered(false){

    socketManager.onAccept = std::bind(&Server::onAccept, this, std::placeholders::_1);
    socketManager.onReceive = std::bind(&Server::onReceive, this, std::placeholders::_1, std::placeholders::_2);
}

Server::~Server() {
    if (listen_fd != -1) {
        socketManager.closeSocket(listen_fd);
    }
}

[[noreturn]] void Server::start() {
    initListener();
    std::cout << "server started" << std::endl;
    std::thread responseThread(&Server::sendMessageToClients, this);
    responseThread.detach();

    std::thread receiveThread(&SocketManager::processEvents, &socketManager);
    receiveThread.detach();

    while(true) {
        struct sockaddr_in client_addr{};
        socklen_t client_addrlen = sizeof(client_addr);
        socketManager.initiateAccept(listen_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addrlen);
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

void Server::onAccept(int client_fd) {
    // Handle client operations asynchronously
    struct sockaddr_in client_addr{};
    socklen_t client_addrlen = sizeof(client_addr);
    getpeername(client_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addrlen);
    Client client;
    client.fd = client_fd;
    client.address = client_addr.sin_addr.s_addr;
    clients.push_back(client);
    socketManager.initiateReceive(client.fd, serverConfiguration.getBufferSize(), 0);
}

void Server::onReceive(int client_fd, std::vector<uint8_t>& data) {

    if(data.size() > 0)
        socketManager.initiateReceive(client_fd, serverConfiguration.getBufferSize(), 0);
    else {
        socketManager.closeSocket(client_fd);
        for(int i = 0; i < clients.size(); i++) {
            if(clients.at(i) .fd == client_fd) {
                if(clients.at(i) .id > 0) {
                    clientsMap.erase(clients.at(i) .id);
                    Database::get()->setClientInactive(clients.at(i) .id);
                }
                clients.erase(clients.begin() + 1);
                return;
            }
        }
    }

    Client* client = nullptr;
    for(int i = 0; i < clients.size(); i++) {
        if(clients.at(i) .fd == client_fd) {
            client = &clients.at(i);
            break;
        }
    }

    if (!processMessage(data, *client)) {
        socketManager.closeSocket(client_fd);
        for(int i = 0; i < clients.size(); i++) {
            if(clients.at(i) .fd == client_fd) {
                clients.erase(clients.begin() + i);
                break;
            }
        }
        if(client->id > 0) {
            clientsMap.erase(client->id);
            Database::get()->setClientInactive(client->id);
            return;
        }
    }
}

void Server::sendMessageToClients() {
    while (true) {
        if (!responseTriggered) {
            std::vector<Message> messages = Database::get()->getUndeliveredMessages();
            for (auto& message : messages) {
                std::cout << message.topic_id << std::endl;
                std::vector<int> client_ids = Database::get()->getSubscribedClients(message.topic_id);
                for (int client_id : client_ids) {
                    Client client;
                    auto it = clientsMap.find(client_id);
                    if (it != clientsMap.end()) {
                        client = it->second;
                    }
                    if (client.fd > 0) {
                        std::vector<uint8_t> data;
                        message.command = (int)Commands::SEND_DATA_RESPONSE;
                        Utilities::get()->encodePacket(message, data);
                        socketManager.initiateSend(client.fd, data, 0) ;
                        Database::get()->markMessageAsDelivered(message.id, client_id);
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Server::sendSimpleResponse(const Client& client, Commands command) {
    responseTriggered = true;
    Message message;
    message.command = (int)command;
    message.topic_id = 0;
    std::vector<uint8_t> data;
    Utilities::get()->encodePacket(message, data);
    socketManager.initiateSend(client.fd, data, 0);
    responseTriggered = false;
}

bool Server::processMessage(std::vector<uint8_t>& data, Client& client) {
    Message message;
    if(!Utilities::get()->decodePacket(data, message)) {
        sendSimpleResponse(client, Commands::DECODE_ERROR);
        return true;
    }
    switch ((Commands)message.command) {
        case Commands::SEND_DATA: {
            if(message.topic_id <= 0) {
                sendSimpleResponse(client, Commands::WRONG_TOPIC);
                return true;
            }
            std::vector<int> subscribedClients = Database::get()->getSubscribedClients(message.topic_id);

            if(subscribedClients.size() == 0) {
                sendSimpleResponse(client, Commands::HAVE_NOT_CLIENTS_SUBSCRIBED);
                return true;
            }

            if(client.id <= 0) {
                sendSimpleResponse(client, Commands::NOT_CONNECTED);
                return true;
            }
            int message_id = 0;

            if(!Database::get()->storeMessage(message, message_id)) {
                sendSimpleResponse(client, Commands::DECODE_ERROR);
                return true;
            }

            for (int i = 0; i < subscribedClients.size(); i++) {
                if(subscribedClients.at(i) != client.id) {
                    if(!Database::get()->trackDelivery(message_id, subscribedClients.at(i))) {
                        sendSimpleResponse(client, Commands::DECODE_ERROR);
                        return true;
                    }
                }
            }

            sendSimpleResponse(client, Commands::SUCCESS);
            return true;
        }
        case Commands::CREATE_TOPIC: {
            std::string topic(reinterpret_cast<const char *>(message.data.data()), message.data.size());

            int topic_id = Database::get()->insertTopic(topic);

            if(topic_id == -1) {
                sendSimpleResponse(client, Commands::DB_ERROR);
                return true;
            }

            sendSimpleResponse(client, Commands::SUCCESS);
            return true;
        }
        case Commands::SUBSCRIBE: {
            std::string topic(reinterpret_cast<const char *>(message.data.data()), message.data.size());

            int topic_id = Database::get()->getTopicByName(topic);

            if(client.id > 0 && topic_id > 0) {
                if(!Database::get()->subscribeClientToTopic(client.id, topic_id)) {
                    sendSimpleResponse(client, Commands::DB_ERROR);
                    return true;
                }
            } else {
                sendSimpleResponse(client, Commands::INVALID_CLIENT);
                return true;
            }

            sendSimpleResponse(client, Commands::SUCCESS);
            return true;
        }
        case Commands::UNSUBSCRIBE: {
            std::string topic(reinterpret_cast<const char *>(message.data.data()), message.data.size());

            if(client.id > 0 && message.topic_id > 0) {
                if(!Database::get()->unsubscribeClientFromTopic(client.id, message.topic_id)) {
                    sendSimpleResponse(client, Commands::DB_ERROR);
                    return true;
                }
            } else {
                sendSimpleResponse(client, Commands::INVALID_CLIENT);
                return true;
            }

            sendSimpleResponse(client, Commands::SUCCESS);
            return true;
        }
        case Commands::CONNECT: {
            std::string data(reinterpret_cast<const char *>(message.data.data()), message.data.size());
            if(!Database::get()->insertOrUpdateClient(data, client.address, client.id)) {
                sendSimpleResponse(client, Commands::DB_ERROR);
                return true;
            }

            clientsMap.emplace(client.id, client);

            std::vector<int> topics = Database::get()->getSubscribedTopics(client.id);
            if(topics.size() == 0) {
                sendSimpleResponse(client, Commands::SUCCESS);
                return true;
            }

            responseTriggered = true;
            Message response_message;
            response_message.command = (int)Commands::TOPICS_RESPONSE;
            response_message.topic_id = 0;
            response_message.data.insert(response_message.data.end(), topics.begin(), topics.end());
            std::vector<uint8_t> response;
            Utilities::get()->encodePacket(response_message, response);
            socketManager.initiateSend(client.fd, response, 0);
            responseTriggered = false;

            return true;
        }
        case Commands::GET_TOPIC_NAME: {
            if(message.topic_id <= 0) {
                sendSimpleResponse(client, Commands::WRONG_TOPIC);
                return true;
            }

            std::string topicName = Database::get()->getTopicByID(message.topic_id);
            if(topicName == "") {
                sendSimpleResponse(client, Commands::DB_ERROR);
                return true;
            }

            responseTriggered = true;
            Message response_message;
            response_message.command = (int)Commands::GET_TOPIC_NAME_RESPONSE;
            response_message.topic_id = message.topic_id;
            response_message.data.insert(response_message.data.end(), topicName.begin(), topicName.end());
            std::vector<uint8_t> response;
            Utilities::get()->encodePacket(response_message, response);
            socketManager.initiateSend(client.fd, response, 0);
            responseTriggered = false;

            return true;
        }
        case Commands::DISCONNECT: {
            Database::get()->unsubscribeClientFromAllTopic(client.id);
            Database::get()->deleteClientById(client.id);
            socketManager.closeSocket(client.fd);
            clientsMap.erase(client.id);
            return false;
        }
        default:
            break;
    }
    return true;
}