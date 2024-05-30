#ifndef SERVER_DATABASE_H
#define SERVER_DATABASE_H

#include <sqlite3.h>
#include <string>

#include "../Helpers/Structures.h"

class Database {
public:
    static Database* get();

    bool insertClient(const std::string& name, uint32_t client_address, int& client_id);
    bool insertTopic(const std::string &topic_name);
    bool subscribeClientToTopic(int client_id, int topic_id);
    bool storeMessage(const Message& message, int& message_id);

    std::vector<int> getSubscribedClients(int topic_id);
    bool trackDelivery(int message_id, int client_id);
    int getClientNameByAddress(uint32_t client_address);
    int getTopicByName(const std::string &topic_name);
    std::vector<Message> getUndeliveredMessages();

    bool markMessageAsDelivered(int message_id, int client_id);

    bool unsubscribeClientFromTopic(int client_id, int topic_id);
    bool unsubscribeClientFromAllTopic(int client_id);
    bool deleteClientById(int client_id);

private:
    explicit Database();
    ~Database();

    void setupDatabase();
    bool setAllClientsInactive();

private:
    sqlite3* db;
};


#endif //SERVER_DATABASE_H
