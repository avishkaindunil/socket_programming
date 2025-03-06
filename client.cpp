#include <iostream>
#include <thread>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void handle_receive(int socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(socket, buffer, 1024);
        if (valread <= 0) {
            std::cerr << "Server disconnected or read failed" << std::endl;
            break;
        }
        std::cout << "\nMessage from server: " << buffer << std::endl;
        std::cout << "Enter your message: ";
        std::cout.flush();
    }
}

void handle_send(int socket) {
    std::string message;
    while (true) {
        std::cout << "Enter your message: ";
        std::getline(std::cin, message);
        if (message == "exit") {
            break;
        }
        send(socket, message.c_str(), message.length(), 0);
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket file descriptor
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        close(sock);
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close(sock);
        return -1;
    }

    // Create threads for sending and receiving messages
    std::thread receive_thread(handle_receive, sock);
    std::thread send_thread(handle_send, sock);

    // Wait for threads to finish
    receive_thread.join();
    send_thread.join();

    // Close the socket
    close(sock);
    return 0;
}
