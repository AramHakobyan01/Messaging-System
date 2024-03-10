#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

#include <iostream>
#include <string>

/**
 * @brief Represents a client to connect to the messaging server.
 */
class Client {
private:
    /**
     * @brief Constructs a Client object.
     */
    Client() = default;

    /**
     * @brief Destructs a Client object.
     */
    ~Client() = default;

public:
    /**
     * @brief Returns the singleton instance of the Client.
     * @return Reference to the singleton Client instance.
     */
    static Client& GetInstance();

    /**
     * @brief Connects the client to the server.
     */
    void ConnectToServer();

    /**
     * @brief Subscribes the client to a given topic.
     * @param topic The topic to subscribe to.
     */
    void Subscribe(const std::string& topic);

    /**
     * @brief Sends data to the server.
     * @param data The data to send.
     */
    void SendData(const std::string& data);

private:
    /**
     * @brief Reads data from the server.
     */
    void ReadData();

private:
    int sock; /**< Socket file descriptor for communication with the server. */
};

#endif // CLIENT_CLIENT_H_
