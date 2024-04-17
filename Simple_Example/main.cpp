// client.c

#include "../Client/Client.h"

int main() {

    Client client;
    client.ConnectToServer();
    client.Subscribe("test");
    while (true) {
        char message[4096];
        printf("Enter message: ");
        fgets(message, 4096, stdin);
        client.SendData(message);
    }
    return 0;
}
