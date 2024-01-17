// server.c

#include "server.h"
#include "./utils/network_utils.h"
#include "./utils/thread_utils.h"
#include "client_handler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

int server_socket = 0;
volatile sig_atomic_t keep_running = 1;
int num_clients = 0;
int client_sockets[MAX_CLIENTS];

void sigint_handler(int _) {
    keep_running = 0;
}

void server_init() {
    server_socket = create_server_socket();
    struct sockaddr_in server_addr;
    configure_server_socket(server_socket, &server_addr);
    initialize_mutexes();
}

void server_start() {
    signal(SIGINT, sigint_handler);

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("\nServer started on port %d\n", SERVER_PORT);

    pthread_t input_thread_id;
    if (pthread_create(&input_thread_id, NULL, server_input_handler, NULL) != 0) {
        perror("Could not create thread for server input");
        exit(EXIT_FAILURE);
    }

    pthread_detach(input_thread_id);

    while (keep_running) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        client_t *client = malloc(sizeof(client_t));
        if (client == NULL) {
            perror("Failed to allocate memory for client");
            close(client_socket);
            continue;
        }

        client->socket = client_socket;
        inet_ntop(AF_INET, &client_addr.sin_addr, client->ip, INET_ADDRSTRLEN);

        if (getnameinfo((struct sockaddr *)&client_addr, sizeof(client_addr), 
                        client->hostname, HOSTNAME_LENGTH, NULL, 0, 0) != 0) {
            strncpy(client->hostname, "Unknown", HOSTNAME_LENGTH);
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_handler, client) != 0) {
            perror("Could not create thread for client");
            free(client);
            close(client_socket);
        } else {
            pthread_detach(thread_id);
        }
    }
}

void server_stop() {
    close(server_socket);
    destroy_mutexes();
}

void *server_input_handler(void *arg) {
    char input_buffer[1024];
    char server_message[1064];

    while (keep_running) {
        if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
            input_buffer[strcspn(input_buffer, "\n")] = 0;

            // Get the current time and format it
            time_t now = time(NULL);
            struct tm *now_tm = localtime(&now);
            char time_str[9];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", now_tm);

            // Format the server message
            snprintf(server_message, sizeof(server_message), "\n\n[%s] Server says: %s\n", time_str, input_buffer);

            // Broadcast the server message
            broadcast_message_from_server(server_message);
        }
    }

    return NULL;
}

int main() {
    server_init();
    server_start();
    server_stop();
    return 0;
}