#ifndef CLIENTCONFIG_H
#define CLIENTCONFIG_H

#include <string>
#include <vector>
#include "../Message/Message.h"
#include "../Socket/Socket.h"

class ClientConfig {
public:
    ClientConfig(const std::string& clientName, const Socket& clientSocket);
    void sendMessage(const Message& message) const;
    void subscribe(const std::string& topic);
    void unsubscribe(const std::string& topic);
    void processReceivedMessage(const Message& message);

private:
    Socket socket;
    std::string name;
    std::vector<std::string> subscribedTopics;
};

#endif // CLIENTCONFIG_H
