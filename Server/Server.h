#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <iostream>
#include <vector>
#include <string>
#include <map>

class Server {
public:
    Server() = default;
    ~Server() = default;
public:
    void Init();
    void Start();
private:
    void HandleClient(int client_socket);
    void ForwardMessage(const std::string& message, int sender_socket);
private:
    int m_socket;
    std::map<std::string, std::vector<int>> m_topics;
    std::map<int, std::string> m_clients;
};

#endif //SERVER_SERVER_H
