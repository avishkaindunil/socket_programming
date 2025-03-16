#include <iostream>
#include <thread>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void handle_receive(int socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(socket, buffer, 1024);
        if (valread <= 0) {
            std::cerr << "Client disconnected or read failed" << std::endl;
            break;
        }
        std::cout << "Message from client: " << buffer << std::endl;
    }
}

void handle_send(int socket) {
    std::string message;
    while (true) {
        std::getline(std::cin, message);
        if (message == "exit") {
            break;
        }
        send(socket, message.c_str(), message.length(), 0);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);  // Creating socket file descriptor
    if (server_fd == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {   // Forcefully attaching socket to the port 8080
        std::cerr << "setsockopt failed" << std::endl;
        close(server_fd);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {  // Bind the socket to the network address and port
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {               // Start listening for incoming connections
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "Waiting for a connection..." << std::endl;

    // Accept an incoming connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        std::cerr << "Accept failed" << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "Connection established" << std::endl;

    // Create threads for sending and receiving messages
    std::thread receive_thread(handle_receive, new_socket);
    std::thread send_thread(handle_send, new_socket);

    // Wait for threads to finish
    receive_thread.join();
    send_thread.join();

    // Close the socket
    close(new_socket);
    close(server_fd);
    return 0;
}
