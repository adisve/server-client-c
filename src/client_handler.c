// client_handler.c

#include "client_handler.h"
#include "./utils/thread_utils.h"
#include "./server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void broadcast_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&client_list_mutex);

    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] != sender_socket) {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }

    pthread_mutex_unlock(&client_list_mutex);
}

void broadcast_message_from_server(const char *message) {
    pthread_mutex_lock(&client_list_mutex);

    for (int i = 0; i < num_clients; i++) {
        send(client_sockets[i], message, strlen(message), 0);
    }

    pthread_mutex_unlock(&client_list_mutex);
}

void *client_handler(void *arg) {
    client_t *client = (client_t *)arg;
    int client_socket = client->socket;

    pthread_mutex_lock(&client_list_mutex);
    if (num_clients < MAX_CLIENTS) {
        client_sockets[num_clients++] = client_socket; // Assuming client_sockets is still int array
    }
    pthread_mutex_unlock(&client_list_mutex);

    char buffer[1024];
    char broadcast_buffer[1080];

    while (1) {
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) { break; }

        buffer[bytes_received] = '\0';

        // Get the current time and format it
        time_t now = time(NULL);
        struct tm *now_tm = localtime(&now);
        char time_str[9];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", now_tm);

        // Format the message for broadcast
        snprintf(broadcast_buffer, sizeof(broadcast_buffer), "\n\n[%s] %s (%s) says: %s\n", time_str, client->hostname, client->ip, buffer);

        // Lock the display mutex and print the message on the server
        pthread_mutex_lock(&display_mutex);
        printf("\n%s\n", broadcast_buffer);
        pthread_mutex_unlock(&display_mutex);

        // Broadcast the message
        broadcast_message(broadcast_buffer, client_socket);
    }

    // Clean up after the loop ends
    pthread_mutex_lock(&client_list_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] == client_socket) {
            client_sockets[i] = client_sockets[num_clients - 1];
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&client_list_mutex);

    // Close the client socket and free the client memory
    close(client_socket);
    printf("\nClient %s (%s) disconnected.\n", client->hostname, client->ip);
    free(client);

    return NULL;
}
