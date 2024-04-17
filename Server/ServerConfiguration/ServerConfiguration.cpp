#include "ServerConfiguration.h"

ServerConfiguration::ServerConfiguration(const std::string &configFile) :
        listeningInterface("0.0.0.0"),
        listeningPort(8080),
        maxClientConnections(10),
        bufferSize(4096) {}

std::string ServerConfiguration::getListeningInterface() const {
    return listeningInterface;
}

int ServerConfiguration::getListeningPort() const {
    return listeningPort;
}

int ServerConfiguration::getMaxClientConnections() const {
    return maxClientConnections;
}

int ServerConfiguration::getBufferSize() const {
    return bufferSize;
}
