#include "Client.h"

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <thread>

#include "Helpers/Utilities.h"

Client::Client(const std::string server_ip, int server_port, int64_t buffer_size)
    : responseTriggered(false),
    sock(0),
    bufferSize(buffer_size),
    name("")
    {

    struct sockaddr_in serv_addr {};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        exit( EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported: " << strerror(errno) << std::endl;
        close(sock);
        exit( EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed: " << strerror(errno) << std::endl;
        close(sock);
        exit( EXIT_FAILURE);
    }
}

Client::~Client() {
    close(sock);
}

// Connect to the server
ServerResponse Client::ConnectToServer(const std::string name, const std::function<void(const std::vector<uint8_t>&, const std::string, ServerResponse)>& callback) {
    responseTriggered = true;
    Message message;
    message.command = (int)Commands::CONNECT;
    message.data.insert(message.data.end(), name.begin(), name.end());
    std::vector<uint8_t> data;
    Utilities::get()->encodePacket(message, data);
    send(sock, data.data(), data.size(), 0);
    ServerResponse error = waitForResponse();

    if(error != ServerResponse::SUCCESS)
        return error;

    this->name = name;

    // Start a separate thread to read data from the server
    std::thread readerThread(&Client::ReadData, this, callback);
    readerThread.detach();
    return ServerResponse::SUCCESS;
}

// Create a topic
ServerResponse Client::CreateTopic(const std::string& topic) {
    responseTriggered = true;
    Message message;
    message.command = (int)Commands::CREATE_TOPIC;
    message.data.insert(message.data.end(), topic.begin(), topic.begin() + topic.length());
    std::vector<uint8_t> data;
    Utilities::get()->encodePacket(message, data);
    send(sock, data.data(), data.size(), 0);
    return waitForResponse();
}

// Subscribe to a topic
ServerResponse Client::Subscribe(const std::string& topic) {
    responseTriggered = true;
    Message message;
    message.command = (int)Commands::SUBSCRIBE;
    message.data.insert(message.data.end(), topic.begin(), topic.begin() + topic.length());
    std::vector<uint8_t> data;
    Utilities::get()->encodePacket(message, data);
    send(sock, data.data(), data.size(), 0);
    return waitForResponse();
}

// Send data to the server
ServerResponse Client::SendData(std::vector<uint8_t>& data, const std::string& topicName) {
    responseTriggered = true;
    Message message;
    message.command = (int)Commands::SEND_DATA;
    message.data.insert(message.data.end(), data.begin(), data.end());
    message.topic_id = topics[topicName];
    data.clear();
    Utilities::get()->encodePacket(message, data);
    send(sock, data.data(), data.size(), 0);
    return waitForResponse();
}

// Read data from the server
void Client::ReadData(const std::function<void(const std::vector<uint8_t>&, const std::string, ServerResponse)> &callback) {
    while (true) {
        if (!responseTriggered) {
            uint8_t buff[bufferSize];
            int res = read(sock, buff, bufferSize);
            if (res > 0) {
                Message message;
                std::vector<uint8_t> data;
                data.insert(data.end(), buff, buff + res);
                Utilities::get()->decodePacket(data, message);
                callback(data, name, static_cast<ServerResponse>(message.command));
            } else if (res == 0) {
                std::vector<uint8_t> data;
                callback(data, "", ServerResponse::SERVER_DISCONNECTED);
                break;
            } else {
                std::vector<uint8_t> data;
                callback(data, "", ServerResponse::RESPONSE_READING_ERROR);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
}

ServerResponse Client::waitForResponse() {
    uint8_t buff[bufferSize];
    int res = read(sock, buff, bufferSize);
    if (res > 0) {
        Message message;
        std::vector<uint8_t> data;
        data.insert(data.end(), buff, buff + res);
        Utilities::get()->decodePacket(data, message);
        responseTriggered = false;
        if(message.command == (int)Commands::TOPICS_RESPONSE) {
            std::vector<int> topic_ids;
            topic_ids.insert(topic_ids.end(), message.data.begin(), message.data.end());
            for (int i = 0; i < topic_ids.size(); ++i) {
                Message request;
                request.command = (int)Commands::GET_TOPIC_NAME;
                request.topic_id = topic_ids.at(i);
                data.clear();
                Utilities::get()->encodePacket(request, data);
                send(sock, data.data(), data.size(), 0);
                ServerResponse error = waitForResponse();
                if(error != ServerResponse::SUCCESS) {
                    return error;
                }
            }
            return ServerResponse::SUCCESS;
        }
        else if(message.command == (int)Commands::GET_TOPIC_NAME_RESPONSE) {
            std::string topic(reinterpret_cast<const char *>(message.data.data()), message.data.size());
            topics.emplace(topic, message.topic_id);
            return ServerResponse::SUCCESS;
        }
        return (ServerResponse)message.command;
    } else if (res == 0) {
        return ServerResponse::SERVER_DISCONNECTED;
    } else {
        return ServerResponse::RESPONSE_READING_ERROR;
    }
}
