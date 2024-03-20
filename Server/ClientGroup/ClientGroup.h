#ifndef CLIENTGROUP_H
#define CLIENTGROUP_H

#include <vector>
#include "../ClientConfig/ClientConfig.h"

class ClientGroup {
public:
    ClientGroup();
    void addClient(const ClientConfig& client);
    void removeClient(const ClientConfig& client);
    void sendMessageToGroup(const Message& message) const;
private:
    std::vector<ClientConfig> clients;
};

#endif // CLIENTGROUP_H
