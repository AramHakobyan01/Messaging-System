#ifndef SERVER_H
#define SERVER_H

#include "Socket/SocketManager.h"
#include "ServerConfiguration/ServerConfiguration.h"
#include "Client/Client.h"

class Server {
public:
    Server();
    ~Server();

    void start();
private:
    void initListener();
    void acceptConnection();
    void handleClient(int client_fd);
    void receiveData(Client client);
    void sendData(int client_fd, const void* buf, size_t len);
    void processMessage(const Message& message, Client client);

private:
    int listen_fd;
    std::vector<Client> clients;
    ServerConfiguration serverConfiguration;
    SocketManager socketManager;
};


#endif // SERVER_H
