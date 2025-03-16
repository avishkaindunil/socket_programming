#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>

#define PORT 8080

void handleServer(int sock) {
    char buffer[1024] = {0};
    std::string message;

    std::thread read_thread([&]() {
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int valread = read(sock, buffer, sizeof(buffer));
            if (valread <= 0) {
                std::cerr << "Server disconnected or read error" << std::endl;
                break;
            }
            std::cout << "Server: " << buffer << std::endl;
        }
        close(sock);
    });

    std::thread write_thread([&]() {
        while (true) {
            std::getline(std::cin, message);
            if (message == "exit") {
                break;
            }
            send(sock, message.c_str(), message.length(), 0);
        }
        close(sock);
    });

    read_thread.join();
    write_thread.join();
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close(sock);
        return -1;
    }

    handleServer(sock);

    return 0;
}
