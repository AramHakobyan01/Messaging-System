#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <liburing.h>
#include <vector>
#include <functional>

class SocketManager {
    struct OperationData {
        int sockfd;
        std::vector<uint8_t> buffer;
        size_t length;
    };
public:
    SocketManager();
    ~SocketManager();

    int createSocket(int domain, int type, int protocol);
    void bindSocket(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    void listenSocket(int sockfd, int backlog);
    void initiateAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    void initiateReceive(int sockfd, size_t len, int flags);
    void initiateSend(int sockfd, const std::vector<uint8_t> &data, int flags);
    void closeSocket(int sockfd);

    void processEvents();

    // Callbacks
    std::function<void(int)> onAccept;
    std::function<void(int, std::vector<uint8_t>&)> onReceive;
    std::function<void(int)> onSend;

private:
    void handleAccept(struct io_uring_cqe* cqe);
    void handleReceive(struct io_uring_cqe* cqe);
    void handleSend(struct io_uring_cqe* cqe);

private:
    struct io_uring ring;
};

#endif // SOCKETMANAGER_H