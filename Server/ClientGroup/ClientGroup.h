#ifndef CLIENTGROUP_H
#define CLIENTGROUP_H

#include <vector>
#include "../ClientConfig/ClientConfig.h"

class ClientGroup {
private:
    std::vector<ClientConfig> clients;

public:
    ClientGroup();
    void addClient(const ClientConfig& client);
    void removeClient(const ClientConfig& client);
    void sendMessageToGroup(const Message& message) const;
};

#endif // CLIENTGROUP_H
