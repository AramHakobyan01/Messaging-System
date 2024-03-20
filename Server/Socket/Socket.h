#ifndef SOCKET_H
#define SOCKET_H

#include <vector>
#include <functional>
#include <cstdint>

class Socket {
public:
    void asyncRead(std::function<void(const std::vector<uint8_t>& data)> callback) const;
    void asyncWrite(const std::vector<uint8_t>& data, std::function<void()> callback) const;
    void close();
};

#endif // SOCKET_H
