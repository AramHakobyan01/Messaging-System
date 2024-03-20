#ifndef SERVERCONFIGURATION_H
#define SERVERCONFIGURATION_H

#include <string>

class ServerConfiguration {
private:
    std::string listeningInterface;
    int listeningPort;
    int maxClientConnections;
    int bufferSize;

public:
    ServerConfiguration();
    std::string getListeningInterface() const;
    void setListeningInterface(const std::string& interface);
    int getListeningPort() const;
    void setListeningPort(int port);
    int getMaxClientConnections() const;
    void setMaxClientConnections(int maxConnections);
    int getBufferSize() const;
    void setBufferSize(int size);
};

#endif // SERVERCONFIGURATION_H
