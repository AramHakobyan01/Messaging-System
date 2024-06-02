#include "SocketManager.h"
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

#include "../Helpers/Structures.h"

SocketManager::SocketManager() {
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        std::cerr << "io_uring initialization failed: " << strerror(errno) << std::endl;
        exit(errno);
    }
}

SocketManager::~SocketManager() {
    io_uring_queue_exit(&ring);
}

int SocketManager::createSocket(int domain, int type, int protocol) {
    int sockfd = socket(domain, type | SOCK_NONBLOCK, protocol);
    if (sockfd < 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        exit(errno);
    }
    return sockfd;
}

void SocketManager::bindSocket(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (bind(sockfd, addr, addrlen) < 0) {
        std::cerr << "Binding failed: " << strerror(errno) << std::endl;
        exit(errno);
    }
}

void SocketManager::listenSocket(int sockfd, int backlog) {
    if (listen(sockfd, backlog) < 0) {
        std::cerr << "Listening failed: " << strerror(errno) << std::endl;
        exit(errno);
    }
}

void SocketManager::initiateAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    auto* opData = new OperationData{sockfd, {}, 0};
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_accept(sqe, sockfd, addr, addrlen, 0);
    io_uring_sqe_set_data(sqe, opData);
    io_uring_submit(&ring);
}

void SocketManager::initiateReceive(int sockfd, size_t len, int flags) {
    auto* opData = new OperationData{sockfd, std::vector<uint8_t>(len), len};
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_recv(sqe, sockfd, opData->buffer.data(), len, flags);
    io_uring_sqe_set_data(sqe, opData);
    io_uring_submit(&ring);
}

void SocketManager::initiateSend(int sockfd, const std::vector<uint8_t> &data, int flags) {
    auto* opData = new OperationData{sockfd, data, data.size()};
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_send(sqe, sockfd, opData->buffer.data(), data.size(), flags);
    io_uring_sqe_set_data(sqe, opData);
    io_uring_submit(&ring);
}

void SocketManager::closeSocket(int sockfd) {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

void SocketManager::processEvents() {
    struct io_uring_cqe* cqe;
    while (true) {
        int ret = io_uring_wait_cqe(&ring, &cqe);
        if (ret < 0) {
            std::cerr << "io_uring_wait_cqe failed: " << strerror(-ret) << std::endl;
            break;
        }

        auto* opData = static_cast<OperationData*>(io_uring_cqe_get_data(cqe));
        if (cqe->res >= 0) {
            if (cqe->flags & IORING_CQE_F_MORE) {
                // handle batched completions if needed
            }

            if (opData->buffer.size() == 0) {
                handleAccept(cqe);
            } else if ((int)opData->buffer[1] < 8) {
                handleReceive(cqe);
            } else if ((int)opData->buffer[1] >= 8) {
                handleSend(cqe);
            }
        } else {
            std::cerr << "Operation failed: " << strerror(-cqe->res) << std::endl;
        }

        io_uring_cqe_seen(&ring, cqe);
    }
}

void SocketManager::handleAccept(struct io_uring_cqe* cqe) {
    auto* opData = static_cast<OperationData*>(io_uring_cqe_get_data(cqe));
    int client_fd = cqe->res;
    if (client_fd >= 0) {
        if (onAccept) {
            onAccept(client_fd);
        }
    }
    delete opData;
}

void SocketManager::handleReceive(struct io_uring_cqe* cqe) {
    auto* opData = static_cast<OperationData*>(io_uring_cqe_get_data(cqe));
    ssize_t recv_size = cqe->res;
    if (recv_size >= 0) {
        opData->buffer.resize(recv_size);
        if (onReceive) {
            onReceive(opData->sockfd, opData->buffer);
        }
    }
    delete opData;
}

void SocketManager::handleSend(struct io_uring_cqe* cqe) {
    auto* opData = static_cast<OperationData*>(io_uring_cqe_get_data(cqe));
    ssize_t sent_size = cqe->res;
    if (sent_size >= 0) {
        if (onSend) {
            onSend(opData->sockfd);
        }
    }
    delete opData;
}