#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <sys/epoll.h>

#define PORT 8080

void handleClient(int client_socket) {
    char buffer[1024] = {0};
    std::string message;
    std::thread read_thread([&]() {
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int valread = read(client_socket, buffer, sizeof(buffer));
            if (valread <= 0) {
                std::cerr << "Client disconnected or read error" << std::endl;
                break;
            }
            std::cout << "Client: " << buffer << std::endl;
        }
        close(client_socket);
    });

    std::thread write_thread([&]() {
        while (true) {
            std::getline(std::cin, message);
            if (message == "exit") {
                break;
            }
            send(client_socket, message.c_str(), message.length(), 0);
        }
        close(client_socket);
    });

    read_thread.join();
    write_thread.join();
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed" << std::endl;
        close(server_fd);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "Server is waiting for connections on port " << PORT << "..." << std::endl;

    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        std::cout << "New connection established" << std::endl;
        std::thread client_thread(handleClient, new_socket);
        client_thread.detach();
    }

    close(server_fd);
    return 0;
}
