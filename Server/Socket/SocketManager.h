#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <liburing.h>

class SocketManager {
public:
    SocketManager();
    ~SocketManager();
    int createSocket(int domain, int type, int protocol);
    void bindSocket(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    void listenSocket(int sockfd, int backlog);
    int acceptSocket(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int receiveData(int sockfd, void *buf, size_t len, int flags);
    int sendData(int sockfd, const void *buf, size_t len, int flags);
    void closeSocket(int sockfd);

private:
    struct io_uring ring;
};

#endif // SOCKETMANAGER_H