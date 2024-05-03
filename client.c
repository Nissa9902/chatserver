// Usage: ./client <SERVER_IP> <PORT>

// Noor, Sasha, and Taslima worked on this 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>


#define PORT 43679
#define SERVER_IP "127.0.0.1"
#define MAX_MSG_SIZE 1024


int client_socket;


void *read_from_socket(void *arg) {
    char buffer[MAX_MSG_SIZE];
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            // Handle server disconnect
            printf("Server disconnected\n");
            break;
        }
        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);
    }
    return NULL;
}

void *read_from_stdin(void *arg) {
    char message[MAX_MSG_SIZE];
    while (1) {
        printf("Enter message (or 'name <new_name>' or 'quit' to exit): ");
        fgets(message, MAX_MSG_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0'; // Remove newline character

        if (strncmp(message, "name ", 5) == 0) {
            // Change client name
            char new_name[20];
            sscanf(message, "name %s", new_name);
            if (strlen(new_name) > 0) {
                if (write(client_socket, message, strlen(message) + 1) == -1) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            } else {
                printf("Invalid name. Please try again.\n");
            }
        } else if (strcmp(message, "quit") == 0) {
            // Client quit command
            break;
        } else {
            if (write(client_socket, message, strlen(message) + 1) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <port number>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
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
        printf("Enter message (or 'quit' to exit, 'name <new_name>' to change name): ");
        fgets(message, MAX_MSG_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0'; // Remove newline character
        
        // Send message to server
        if (send(client_socket, message, strlen(message), 0) == -1) {
            perror("Failed to send message to server");
            break;
        }

        // If the message is "quit", break out of the loop
        if (strcmp(message, "quit") == 0) {
            break;
        }
    }

    // Close socket
    close(client_socket);

    return 0;
}