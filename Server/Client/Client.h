#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

#include <string>
#include <vector>

#include "../Structures.h"

class Client {
public:
    Client(int client_fd);
    ~Client() = default;

    // Getters
    int getFD();

    Commands getCommand(Message& message);
private:
    int fd;
    std::vector<std::string> subscriptions;
};


#endif //SERVER_CLIENT_H
