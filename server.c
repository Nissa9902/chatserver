// Usage: ./server <port_number>

// This server should listen on the specified port, waiting for incoming connections.
// After a client connects, add it to a list of currently connected clients.
// If any message comes in from *any* connected client, then it is repeated to *all*
//    other connected clients.
// If reading or writing to a client's socket fails, then that client should be removed from the linked list. 

// Remember that blocking read calls will cause your server to stall. Instead, set your
// your sockets to be non-blocking. Then, your reads will never block, but instead return
// an error code indicating there was nothing to read- this error code can be either
// EAGAIN or EWOULDBLOCK, so make sure to check for both. If your read call fails
// with that error, then ignore it. If it fails with any other error, then treat that
// client as though they have disconnected.

// You can create non-blocking sockets by passing the SOCK_NONBLOCK argument to both
// the socket() function, as well as the accept4() function.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 43679
#define MAX_CLIENTS 10

typedef struct {
    int socket;
    struct sockaddr_in address;
    char name[20];
} Client;

Client clients[MAX_CLIENTS];
int num_clients = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(char *message, int sender_socket) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].socket != sender_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}

void handle_client(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[1024];
    int index = -1;
    
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            close(client_socket);
            if (index != -1) {
                printf("%s has quit\n", clients[index].name);
                pthread_mutex_lock(&mutex);
                for (int i = index; i < num_clients - 1; i++) {
                    clients[i] = clients[i + 1];
                }
                num_clients--;
                pthread_mutex_unlock(&mutex);
                char quit_message[50];
                sprintf(quit_message, "%s has quit", clients[index].name);
                broadcast_message(quit_message, client_socket);
            }
            break;
        }
        buffer[bytes_received] = '\0';
        if (strncmp(buffer, "name ", 5) == 0) {
            char new_name[20];
            sscanf(buffer, "name %s", new_name);
            if (strlen(new_name) > 0) {
                pthread_mutex_lock(&mutex);
                if (index != -1) {
                    char name_change_message[50];
                    sprintf(name_change_message, "%s has changed their name to %s", clients[index].name, new_name);
                    strcpy(clients[index].name, new_name);
                    broadcast_message(name_change_message, client_socket);
                }
                pthread_mutex_unlock(&mutex);
            }
        } else if (strncmp(buffer, "quit", 4) == 0) {
            close(client_socket);
            if (index != -1) {
                printf("%s has quit\n", clients[index].name);
                pthread_mutex_lock(&mutex);
                for (int i = index; i < num_clients - 1; i++) {
                    clients[i] = clients[i + 1];
                }
                num_clients--;
                pthread_mutex_unlock(&mutex);
                char quit_message[50];
                sprintf(quit_message, "%s has quit", clients[index].name);
                broadcast_message(quit_message, client_socket);
            }
            break;
        } else {
            pthread_mutex_lock(&mutex);
            if (index != -1) {
                char message[1074];
                sprintf(message, "%s: %s", clients[index].name, buffer);
                broadcast_message(message, client_socket);
            }
            pthread_mutex_unlock(&mutex);
        }
    }
    free(arg);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    pthread_t tid;
    
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_socket, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        socklen_t client_addr_len = sizeof(client_addr);
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
            perror("accept");
            continue;
        }
        
        if (num_clients < MAX_CLIENTS) {
            clients[num_clients].socket = client_socket;
            clients[num_clients].address = client_addr;
            sprintf(clients[num_clients].name, "User%d", num_clients);
            printf("%s has connected\n", clients[num_clients].name);
            pthread_create(&tid, NULL, (void *)handle_client, (void *)&client_socket);
            num_clients++;
        } else {
            printf("Too many clients. Connection rejected.\n");
            close(client_socket);
        }
    }
    
    return 0;
}
