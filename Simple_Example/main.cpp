#include "Client/Client.h"

void func(const std::vector<uint8_t>& data, const std::string& topic_name, ServerResponse responseCode) {
    std::string response(reinterpret_cast<const char *>(data.data()), data.size());

    if(responseCode == ServerResponse::SUCCESS) {
        return;
    }
    if(responseCode == ServerResponse::DATA_RESPONSE)
        std::cout << "topic_name: " << topic_name << " responseCode: " << (int)responseCode << " response: " << response << std::endl;
    else
        std::cout << "topic_name: " << topic_name << " responseCode: " << (int)responseCode << std::endl;
}

int main() {

    Client client("0.0.0.0", 8085, 4048);

    std::cout << " ConnectToServer: " << (int)client.ConnectToServer("name", func) << std::endl;
    std::cout << " CreateTopic: " << (int)client.CreateTopic("topic") << std::endl;
    std::cout << " Subscribe: " << (int)client.Subscribe("topic") << std::endl;

    std::string input = "data";
    std::vector<uint8_t> data;
    data.insert(data.end(), input.begin(), input.end());
    std::cout << " SendData: " << (int)client.SendData(data, "topic") << std::endl;

    return 0;
}
