#ifndef SERVER_STRUCTURES_H
#define SERVER_STRUCTURES_H

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

#define DB_PATH "./db/server.db"
#define QUEUE_DEPTH 128

// Packet delimiters
#define Flg_START       0xF0
#define Flg_STOP        0xF1
#define Flg_CTRL_ESC    0xF2
#define CTRL_ESC_MASK   0xAA

#define BYTE_MASK_L           0x00FF

#define getLowByte(x) ((uint8_t)(x & BYTE_MASK_L))
#define getHighByte(x) ((uint8_t)((x >> 8) & BYTE_MASK_L))

#define MIN_PACKET_LENGTH   4

// Message structure
struct Message {
    int id;
    int topic_id;
    int command;
    std::vector<uint8_t> data;
    Message() {
        id = 0;
        topic_id = 0;
        command = 0;
    }
};

enum class Commands {
    CREATE_TOPIC = 1,
    SUBSCRIBE,
    UNSUBSCRIBE,
    SEND_DATA,
    CONNECT,
    GET_TOPIC_NAME,
    DISCONNECT,

    TOPICS_RESPONSE,
    SEND_DATA_RESPONSE,
    GET_TOPIC_NAME_RESPONSE,

    SUCCESS,
    DB_ERROR,
    DECODE_ERROR,
    WRONG_TOPIC,
    INVALID_CLIENT,
    HAVE_NOT_CLIENTS_SUBSCRIBED,
    NOT_CONNECTED
};
#endif //SERVER_STRUCTURES_H
