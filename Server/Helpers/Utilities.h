#ifndef SERVER_UTILITIES_H
#define SERVER_UTILITIES_H

#include "Structures.h"

class Utilities {
    struct Packet {
        bool isStartExist;
        bool isCtrlEscExist;
        uint8_t packetPtr;
        std::vector<uint8_t> data;
    };
public:
    static Utilities* get();

    uint16_t MODBUS_CRC16(const uint8_t *nData, uint16_t wLength);
    bool encodePacket(Message& message, std::vector<uint8_t>& data);
    bool decodePacket(std::vector<uint8_t>& data, Message& message);

private:
    Utilities() = default;
    ~Utilities() = default;

    bool checkIsPacketCorrect(Packet& packet, uint8_t byte);
    uint64_t createPacket(std::vector<uint8_t>& tmpdata, std::vector<uint8_t>& data);
};


#endif //SERVER_UTILITIES_H
