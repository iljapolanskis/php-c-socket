#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

void handle_sigint(int sig);

const char *socket_path = "/tmp/php-c-socket";
int server_socket;

int serve() {
    struct sockaddr_un server_addr;
    char buffer[1024];

    // Register signal handler for SIGINT
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);

    // Create a socket
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    // Remove any existing socket file
    unlink(socket_path);

    // Specify server details
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind");
        close(server_socket);
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Failed to listen");
        close(server_socket);
        exit(1);
    }


    printf("Server listening...\n");

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("Failed to accept connection");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {  // Child process
            close(server_socket);  // Close the listening socket in the child

            // Handle the client connection
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received > 0) {
                printf("Received: %s\n", buffer);
            }
            close(client_socket);
            exit(0);  // End the child process
        } else if (pid > 0) {  // Parent process
            printf("Accepted connection from client\n");
            close(client_socket);  // Close the client socket in the parent
        } else {
            perror("Fork failed");
            close(client_socket);
        }
    }
}

void handle_sigint(int sig) {
    printf("\nGracefully shutting down...\n");
    close(server_socket);
    unlink(socket_path);  // Remove the socket file
    exit(0);
}
