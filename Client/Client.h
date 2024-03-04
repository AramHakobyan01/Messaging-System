#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <iostream>

class Client {
private:
    Client() = default;
    ~Client() = default;
public:
    static Client& GetInstance();
    void ConnectToServer();
    void Subscribe(const std::string& topic);
    void SendData(const std::string& data);
private:
    void ReadData();
private:
    int sock;
};

#endif //CLIENT_CLIENT_H
