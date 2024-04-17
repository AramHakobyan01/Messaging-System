#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

#include <iostream>
#include <string>

enum class Commands {
    CREATE_TOPIC = 0,
    SUBSCRIBE = 1,
    UNSUBSCRIBE = 2,
    SEND_DATA = 3
};

class Client {
public:
    Client() = default;
    ~Client() = default;
    void ConnectToServer();
    void Subscribe(const std::string& topic);
    void SendData(const std::string& data);

private:
    void ReadData();

private:
    int sock;
};

#endif // CLIENT_CLIENT_H_
