#include "ServerConfiguration.h"

#include <yaml-cpp/yaml.h>
#include <iostream>

#include "../Helpers/Structures.h"

ServerConfiguration::ServerConfiguration(const std::string &configFile) :
        listeningInterface("0.0.0.0"),
        listeningPort(8080),
        maxClientConnections(0),
        bufferSize(4096 + MIN_PACKET_LENGTH) {
    try {
        YAML::Node config = YAML::LoadFile(configFile);

        if (config["listening_interface"]) {
            listeningInterface = config["listening_interface"].as<std::string>();
        }

        if (config["listening_port"]) {
            listeningPort = config["listening_port"].as<int>();
        }

        if (config["max_client_connections"]) {
            maxClientConnections = config["max_client_connections"].as<int>();
        }

        if (config["buffer_size"]) {
            bufferSize = config["buffer_size"].as<int>() + MIN_PACKET_LENGTH;
        }
    } catch (const YAML::Exception& e) {
        std::cerr << "Error loading configuration from YAML file: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

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
