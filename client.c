// Usage: ./client <IP_address> <port_number>

// The client attempts to connect to a server at the specified IP address and port number.
// The client should simultaneously do two things:
//     1. Try to read from the socket, and if anything appears, print it to the local standard output.
//     2. Try to read from standard input, and if anything appears, print it to the socket. 

// Remember that you can use a pthread to accomplish both of these things simultaneously.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 43679
#define SERVER_IP "127.0.0.1"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_MSG_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <port number>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Server address structure
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection to server failed");
        return 1;
    }

    char message[MAX_MSG_SIZE];
    while (1) {
        printf("Enter message: ");
        fgets(message, MAX_MSG_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0'; // Remove newline character
        
        // Send message to server
        if (send(client_socket, message, strlen(message), 0) == -1) {
            perror("Failed to send message to server");
            break;
        }
    }

    // Close socket
    close(client_socket);

    return 0;
}

