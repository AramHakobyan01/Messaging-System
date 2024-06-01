#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

#include <iostream>
#include <functional>
#include <cstdint>
#include <atomic>

enum class ServerResponse {
    DATA_RESPONSE = 10,
    SUCCESS = 11,
    SERVER_DB_ERROR = 12,
    DECODE_ERROR = 13,
    WRONG_TOPIC = 14,
    INVALID_CLIENT = 15,
    HAVE_NOT_CLIENTS_SUBSCRIBED = 16,
    NOT_CONNECTED = 17,
    SERVER_DISCONNECTED = 18,
    RESPONSE_READING_ERROR = 19
};

class Client {
public:
    explicit Client(const std::string server_ip = "0.0.0.0", int server_port = 8080, int64_t buffer_size = 4048);
    ~Client();
    ServerResponse ConnectToServer(const std::string name, const std::function<void(const std::vector<uint8_t>&, const std::string, ServerResponse)>& callback);
    ServerResponse CreateTopic(const std::string& topic);
    ServerResponse Subscribe(const std::string& topic);
    ServerResponse SendData(std::vector<uint8_t>& data, const std::string& topicName);

private:
    void ReadData(const std::function<void(const std::vector<uint8_t>&, const std::string, ServerResponse)> &callback);
    ServerResponse waitForResponse();

private:
    std::atomic<bool> responseTriggered;
    int sock;
    int64_t bufferSize;
    std::string name;
    std::unordered_map<std::string, int> topics;
};

#endif // CLIENT_CLIENT_H_
