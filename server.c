
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 43679
#define MAX_CLIENTS 10

// Structure to store client information
typedef struct {
    int socket;
    struct sockaddr_in address;
    char name[20];
} Client;

Client clients[MAX_CLIENTS];
int num_clients = 0;
int active_clients = 0; // Initialize active_clients

// Function to broadcast message to all clients except sender
void broadcast_message(char *message, int sender_socket) {
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].socket != sender_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

// Function to handle client connections
void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[1024];
    int index = -1; // Index of the client in the clients array

    // Find the index of the client in the clients array
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].socket == client_socket) {
            index = i;
            break;
        }
    }

    // Increment active_clients when a new client connects
    __sync_fetch_and_add(&active_clients, 1);

    // Receive and broadcast messages
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            // Handle client disconnect
            close(client_socket);
            if (index != -1) {
                // Remove client from list
                printf("%s has quit\n", clients[index].name);
                for (int i = index; i < num_clients - 1; i++) {
                    clients[i] = clients[i + 1];
                }
                num_clients--;
            }
            // Decrement active_clients when a client disconnects
            __sync_fetch_and_sub(&active_clients, 1);
            break;
        }

        buffer[bytes_received] = '\0';

        // Check for special commands
        if (strncmp(buffer, "name ", 5) == 0) {
            // Change client name
            char new_name[20];
            sscanf(buffer, "name %s", new_name);
            if (strlen(new_name) > 0) {
                if (index != -1) {
                    printf("%s has changed their name to %s\n", clients[index].name, new_name);
                    strcpy(clients[index].name, new_name);
                }
            }
        } else if (strncmp(buffer, "quit", 4) == 0) {
            // Client quit command
            close(client_socket);
            if (index != -1) {
                // Remove client from list
                printf("%s has quit\n", clients[index].name);
                for (int i = index; i < num_clients - 1; i++) {
                    clients[i] = clients[i + 1];
                }
                num_clients--;
            }
            // Decrement active_clients when a client quits
            __sync_fetch_and_sub(&active_clients, 1);
            break;
        } else {
            // Broadcast message to other clients
            if (index != -1) {
                char message[strlen(clients[index].name) + strlen(buffer) + 3]; // +3 for ':', space, and null terminator
                sprintf(message, "%s: %s", clients[index].name, buffer);

                broadcast_message(message, client_socket);
                // Print the message sent by the client
                printf("%s: %s\n", clients[index].name, buffer);
            }
        }
    }

    // If there are no active clients, exit the server
    if (active_clients == 0) {
        printf("All clients disconnected. Server shutting down.\n");
        exit(EXIT_SUCCESS);
    }

    return NULL;
}


int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    pthread_t tid;
    
    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    // Initialize server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Bind server socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    // Accept incoming connections
    while (1) {
        socklen_t client_addr_len = sizeof(client_addr);
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
            perror("accept");
            continue;
        }
        // Add client to list
        if (num_clients < MAX_CLIENTS) {
            clients[num_clients].socket = client_socket;
            clients[num_clients].address = client_addr;
            sprintf(clients[num_clients].name, "User%d", num_clients);
            printf("%s has connected\n", clients[num_clients].name);
            // Handle client in a separate thread
            pthread_create(&tid, NULL, handle_client, &client_socket);
            num_clients++;
        } else {
            printf("Too many clients. Connection rejected.\n");
            close(client_socket);
        }
    }
    
    return 0;
}
