#include "SocketManager.h"
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

#define QUEUE_DEPTH 128

SocketManager::SocketManager() {
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        std::cerr << "io_uring initialization failed: " << strerror(errno) << std::endl;
        exit(errno); // Exit with errno
    }
}

SocketManager::~SocketManager() {
    io_uring_queue_exit(&ring);
}

int SocketManager::createSocket(int domain, int type, int protocol) {
    int sockfd = socket(domain, type | SOCK_NONBLOCK, protocol);
    if (sockfd < 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        exit(errno); // Exit with errno
    }
    return sockfd;
}

void SocketManager::bindSocket(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (bind(sockfd, addr, addrlen) < 0) {
        std::cerr << "Binding failed: " << strerror(errno) << std::endl;
        exit(errno); // Exit with errno
    }
}

void SocketManager::listenSocket(int sockfd, int backlog) {
    if (listen(sockfd, backlog) < 0) {
        std::cerr << "Listening failed: " << strerror(errno) << std::endl;
        exit(errno); // Exit with errno
    }
}

int SocketManager::acceptSocket(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int client_fd = accept4(sockfd, addr, addrlen, SOCK_NONBLOCK);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Accepting connection failed: " << strerror(errno) << std::endl;
            exit(errno); // Exit with errno
        }
    }
    return client_fd;
}

int SocketManager::receiveData(int sockfd, void *buf, size_t len, int flags) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_recv(sqe, sockfd, buf, len, flags);
    io_uring_sqe_set_data(sqe, nullptr);
    if (io_uring_submit(&ring) < 0) {
        std::cerr << "io_uring_submit failed: " << strerror(errno) << std::endl;
        return -1; // Return error code
    }

    // Wait for completion
    struct io_uring_cqe *cqe;
    if (io_uring_wait_cqe(&ring, &cqe) < 0) {
        std::cerr << "io_uring_wait_cqe failed: " << strerror(errno) << std::endl;
        return -1; // Return error code
    }

    // Check if the operation was successful
    if (cqe->res < 0) {
        std::cerr << "Receive failed for sockfd: " << sockfd << ": " << strerror(-cqe->res) << std::endl;
        return -1; // Return error code
    }

    int recv_size = cqe->res;
    io_uring_cqe_seen(&ring, cqe); // Release the completion queue entry
    return recv_size;
}

int SocketManager::sendData(int sockfd, const void *buf, size_t len, int flags) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_send(sqe, sockfd, const_cast<void *>(buf), len, flags);
    io_uring_sqe_set_data(sqe, const_cast<void *>(buf)); // Set buffer pointer as user data
    if (io_uring_submit(&ring) < 0) {
        std::cerr << "io_uring_submit failed: " << strerror(errno) << std::endl;
        return -1; // Return error code
    }

    // Wait for completion
    struct io_uring_cqe *cqe;
    if (io_uring_wait_cqe(&ring, &cqe) < 0) {
        std::cerr << "io_uring_wait_cqe failed: " << strerror(errno) << std::endl;
        return -1; // Return error code
    }

    // Release the completion queue entry
    io_uring_cqe_seen(&ring, cqe);

    return 0;
}


void SocketManager::closeSocket(int sockfd) {
    close(sockfd);
}
