#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <string>
#include <cstdint>

class Message {
public:
    Message();
    int64_t getSize() const;
    void setData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> getData() const;
    void setTopic(const std::string& topic);
    std::string getTopic() const;
    void setMessageId(const std::string& messageId);
    std::string getMessageId() const;

private:
    int64_t size;
    std::vector<uint8_t> data;
    std::string topic;
    std::string messageId;
};

#endif // MESSAGE_H
