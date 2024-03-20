#ifndef SERVERCONFIGURATION_H
#define SERVERCONFIGURATION_H

#include <string>

class ServerConfiguration {
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

private:
    std::string listeningInterface;
    int listeningPort;
    int maxClientConnections;
    int bufferSize;
};

#endif // SERVERCONFIGURATION_H
