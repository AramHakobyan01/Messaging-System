#include <iostream>
#include "Client.h"

int main(int argc, char const *argv[]) {
    Client& client = Client::GetInstance();
    client.ConnectToServer();
    std::string input;
    std::cout << "\nEnter topic: ";
    std::getline(std::cin, input);
    client.Subscribe(input);
    while (true) {
        std::cout << "\nEnter message: ";
        std::getline(std::cin, input);
        client.SendData(input);
    }
    return 0;
}
