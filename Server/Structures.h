#ifndef SERVER_STRUCTURES_H
#define SERVER_STRUCTURES_H

#include <string>
#include <unordered_map>
#include <vector>

#define QUEUE_DEPTH 128

// Message structure
struct Message {
    char* data;
    int size;
    std::string topic;

    Message(int size) :
        size(size),
        data(new char[size]) {}

    ~Message() {
        if(data != nullptr)
            delete[] data;
    }
};

enum class Commands {
    CREATE_TOPIC = 0,
    SUBSCRIBE = 1,
    UNSUBSCRIBE = 2,
    SEND_DATA = 3
};

#endif //SERVER_STRUCTURES_H
