#include "ServerConfiguration.h"

#include <yaml-cpp/yaml.h>
#include <iostream>

ServerConfiguration::ServerConfiguration(const std::string &configFile) :
        listeningInterface("0.0.0.0"),
        listeningPort(8080),
        maxClientConnections(10),
        bufferSize(4096) {
    try {
        // Load configuration from YAML file
        YAML::Node config = YAML::LoadFile(configFile);

        // Check if listening interface is specified in the config
        if (config["listening_interface"]) {
            listeningInterface = config["listening_interface"].as<std::string>();
        }

        // Check if listening port is specified in the config
        if (config["listening_port"]) {
            listeningPort = config["listening_port"].as<int>();
        }

        // Check if max client connections is specified in the config
        if (config["max_client_connections"]) {
            maxClientConnections = config["max_client_connections"].as<int>();
        }

        // Check if buffer size is specified in the config
        if (config["buffer_size"]) {
            bufferSize = config["buffer_size"].as<int>();
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
