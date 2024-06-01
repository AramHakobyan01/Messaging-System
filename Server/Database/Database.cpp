#include "Database.h"
#include <iostream>

Database::Database() {
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
    if(db != nullptr) {
        setupDatabase();
        setAllClientsInactive();
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

Database* Database::get() {
    static Database database;
    return &database;
}

bool Database::insertTopic(const std::string& topic_name) {
    const char* sql = "INSERT INTO topics (name) VALUES (?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, topic_name.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::insertOrUpdateClient(const std::string& name, uint32_t client_address, int& client_id) {
    const char* insert_sql = "INSERT OR IGNORE INTO clients (name, client_address, is_active) VALUES (?, ?, ?);";
    const char* update_sql = "UPDATE clients SET is_active = 1 WHERE name = ? AND client_address = ?;";
    sqlite3_stmt* stmt;
    int rc;

    // Prepare and execute the INSERT statement
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare insert): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, client_address);
    sqlite3_bind_int(stmt, 3, 1);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error (step insert): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    // If no row was inserted (client already exists), execute the UPDATE statement
    if (sqlite3_changes(db) == 0) {
        rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error (prepare update): " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 2, client_address);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "SQL error (step update): " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);

        // Retrieve the client_id of the updated row
        const char* select_sql = "SELECT id FROM clients WHERE name = ? AND client_address = ?;";
        rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error (prepare select): " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 2, client_address);

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            client_id = sqlite3_column_int(stmt, 0);
        } else {
            std::cerr << "SQL error (select): " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
    } else {
        // If a new row was inserted, retrieve the client_id
        client_id = sqlite3_last_insert_rowid(db);
    }

    return true;
}


bool Database::subscribeClientToTopic(int client_id, int topic_id) {
    const char* sql = "INSERT INTO subscriptions (client_id, topic_id) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);
    sqlite3_bind_int(stmt, 2, topic_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::storeMessage(const Message& message, int& message_id) {
    const char* sql = "INSERT INTO messages (size, data, topic_id) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int64(stmt, 1, message.data.size());
    sqlite3_bind_blob(stmt, 2, message.data.data(), message.data.size(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, message.topic_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    message_id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return true;
}

std::vector<int> Database::getSubscribedClients(int topic_id) {
    const char* sql = "SELECT c.id FROM clients c JOIN subscriptions s ON c.id = s.client_id WHERE s.topic_id = ? AND c.is_active = 1;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return {};
    }

    sqlite3_bind_int(stmt, 1, topic_id);

    std::vector<int> client_ids;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        client_ids.push_back(sqlite3_column_int(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return client_ids;
}

std::vector<int> Database::getSubscribedTopics(int client_id) {
    const char* sql = "SELECT t.id FROM topics t JOIN subscriptions s ON t.id = s.topic_id WHERE s.client_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return {};
    }

    sqlite3_bind_int(stmt, 1, client_id);

    std::vector<int> topic_ids;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        topic_ids.push_back(sqlite3_column_int(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return topic_ids;
}

bool Database::trackDelivery(int message_id, int client_id) {
    const char* sql = "INSERT INTO delivery (message_id, client_id) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, message_id);
    sqlite3_bind_int(stmt, 2, client_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

int Database::getClientNameByAddress(uint32_t client_address) {
    const char* sql = "SELECT id FROM clients WHERE client_address = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, client_address);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }
    int client_id = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return client_id;
}

int Database::getTopicByName(const std::string &topic_name) {
    const char* sql = "SELECT id FROM topics WHERE name = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, topic_name.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }
    int topic_id = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return topic_id;
}

std::vector<Message> Database::getUndeliveredMessages() {
    const char* sql = "SELECT id, size, data, topic_id FROM messages WHERE id IN (SELECT message_id FROM delivery WHERE delivered = 0);";
    sqlite3_stmt* stmt;
    std::vector<Message> messages;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return messages;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Message message;
        message.id = sqlite3_column_int(stmt, 0);
        message.size = sqlite3_column_int(stmt, 1);
        message.data.assign((uint8_t*)sqlite3_column_blob(stmt, 2), (uint8_t*)sqlite3_column_blob(stmt, 2) + message.size);
        message.topic_id = sqlite3_column_int(stmt, 3);
        messages.push_back(message);
    }

    sqlite3_finalize(stmt);
    return messages;
}

bool Database::markMessageAsDelivered(int message_id, int client_id) {
    const char* sql = "UPDATE delivery SET delivered = 1 WHERE message_id = ? AND client_id = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, message_id);
    sqlite3_bind_int(stmt, 2, client_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::setClientInactive(int client_id) {
    const char* sql = "UPDATE clients SET is_active = 0 WHERE id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::unsubscribeClientFromTopic(int client_id, int topic_id) {
    const char* sql = "DELETE FROM subscriptions WHERE client_id = ? AND topic_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);
    sqlite3_bind_int(stmt, 2, topic_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::unsubscribeClientFromAllTopic(int client_id) {
    const char* sql = "DELETE FROM subscriptions WHERE client_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::deleteClientById(int client_id) {
    const char* sql = "DELETE FROM clients WHERE id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::setAllClientsInactive() {
    const char* sql = "UPDATE clients SET is_active = 0;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (prepare): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error (step): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

void Database::setupDatabase() {
    const char* setupSQL = R"(
        PRAGMA foreign_keys = ON;
        CREATE TABLE IF NOT EXISTS clients (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            is_active BOOLEAN DEFAULT 0,
            client_address INTEGER NOT NULL,
            name TEXT NOT NULL,
            UNIQUE(name, client_address)
        );
        CREATE TABLE IF NOT EXISTS topics (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE
        );
        CREATE TABLE IF NOT EXISTS subscriptions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            client_id INTEGER NOT NULL,
            topic_id INTEGER NOT NULL,
            FOREIGN KEY (client_id) REFERENCES clients(id),
            FOREIGN KEY (topic_id) REFERENCES topics(id),
            UNIQUE(client_id, topic_id)
        );
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            size INTEGER NOT NULL,
            data BLOB NOT NULL,
            topic_id INTEGER NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (topic_id) REFERENCES topics(id)
        );
        CREATE TABLE IF NOT EXISTS delivery (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            message_id INTEGER NOT NULL,
            client_id INTEGER NOT NULL,
            delivered BOOLEAN NOT NULL DEFAULT 0,
            FOREIGN KEY (message_id) REFERENCES messages(id),
            FOREIGN KEY (client_id) REFERENCES clients(id),
            UNIQUE(message_id, client_id)
        );
        CREATE INDEX IF NOT EXISTS idx_client_name ON clients(name);
        CREATE INDEX IF NOT EXISTS idx_topic_name ON topics(name);
        CREATE INDEX IF NOT EXISTS idx_message_topic ON messages(topic_id);
        CREATE INDEX IF NOT EXISTS idx_delivery_status ON delivery(delivered);
    )";

    char* errorMessage = nullptr;
    int rc = sqlite3_exec(db, setupSQL, nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Error setting up database: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    }
}