#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 8888
#define MAX_PENDING_CONNECTIONS 10
#define MAX_BUFFER_SIZE 1024

void handle_client(int client_socket) 
{
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;

    while (1) {
        // Receive data from client
        bytes_received = recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Error receiving data from client");
            break;
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
            break;
        }

        // Echo received data back to client
        send(client_socket, buffer, bytes_received, 0);
        memset(buffer, 0, MAX_BUFFER_SIZE);
    }

    // Close the client socket
    close(client_socket);
}

int main()
{
    pid_t pid, sid;

    pid = fork();

    if (pid < 0) 
    {
        perror("Error forking");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // Parent process exits
        exit(EXIT_SUCCESS);
    }

    umask(0); // Set the file mode creation mask

    sid = setsid(); // Create a new session
    if (sid < 0) {
        perror("Error creating new session");
        exit(EXIT_FAILURE);
    }

    // Change the working directory if needed
    if ((chdir("/")) < 0) {
        perror("Error changing directory");
        exit(EXIT_FAILURE);
    }

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set server address details
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_PENDING_CONNECTIONS) < 0) {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }

    printf("Server daemon running...\n");

    while (1) {
        // Accept incoming connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }

        // Handle client communication
        handle_client(client_socket);
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
