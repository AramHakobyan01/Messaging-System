#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <queue>
#include "ServerConfiguration/ServerConfiguration.h"
#include "ClientConfig/ClientConfig.h"
#include "ClientGroup/ClientGroup.h"
#include "Message/Message.h"

class Server {
public:
    Server(const ServerConfiguration& config);
    void start();
    void stop();
    void acceptNewClient(const ClientConfig& client);
    void processClientRequest(const ClientConfig& client);
    void processMessageFromClient(const ClientConfig& client, const Message& message);
    void dispatchMessageToSubscribers(const Message& message);

private:
    ServerConfiguration configuration;
    std::map<std::string, ClientGroup> clients;
    std::queue<Message> messageQueue;
};

#endif // SERVER_H
