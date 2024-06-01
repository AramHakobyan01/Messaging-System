#ifndef SERVER_H
#define SERVER_H

#include "SocketManager//SocketManager.h"
#include "ServerConfiguration/ServerConfiguration.h"
#include "Helpers/Utilities.h"

class Server {
public:
    Server();
    ~Server();

    [[noreturn]] void start();
private:
    void initListener();
    void acceptConnection();
    void handleClient(const Client& client);
    void receiveData(Client client);
    void sendMessageToClients();
    void sendSimpleResponse(const Client& client, Commands command);
    bool processMessage(std::vector<uint8_t>& data, Client& client);

private:
    int listen_fd;
    ServerConfiguration serverConfiguration;
    SocketManager socketManager;
    std::unordered_map<int, Client> clients;
    std::atomic<bool> responseTriggered;
};


#endif // SERVER_H
