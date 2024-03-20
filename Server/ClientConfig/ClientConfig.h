#ifndef CLIENTCONFIG_H
#define CLIENTCONFIG_H

#include <string>
#include <set>
#include "../Message/Message.h"
#include "../Socket/Socket.h"

class ClientConfig {
private:
    Socket socket;
    std::string name;
    std::set<std::string> subscribedTopics;

public:
    ClientConfig(const std::string& clientName, const Socket& clientSocket);
    void sendMessage(const Message& message) const;
    void subscribe(const std::string& topic);
    void unsubscribe(const std::string& topic);
    void processReceivedMessage(const Message& message);
};

#endif // CLIENTCONFIG_H
