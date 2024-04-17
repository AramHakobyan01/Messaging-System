#ifndef SERVERCONFIGURATION_H
#define SERVERCONFIGURATION_H

#include <string>

class ServerConfiguration {
public:
    ServerConfiguration(const std::string &configFile);

    std::string getListeningInterface() const;
    int getListeningPort() const;
    int getMaxClientConnections() const;
    int getBufferSize() const;
    void setBufferSize(int size);

private:
    std::string listeningInterface;
    int listeningPort;
    int maxClientConnections;
    int bufferSize;
};

#endif // SERVERCONFIGURATION_H
