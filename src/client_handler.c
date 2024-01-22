// client_handler.c

/**
 * @file client_handler.c
 * @author Adis Veletanlic
 * @brief The main purpose of this file is to provide
 * the implementation of the client handler. It contains
 * the function for handling new clients, as well as
 * the function for handling client messages.
 * 
 * Both clients and the server/host are able to send
 * messages to all other clients. Locking is used to
 * ensure that messages are not printed out of order.
 * 
 * @version 0.1
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "client_handler.h"
#include "./utils/thread_utils.h"
#include "./server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

/**
 * @brief Broadcasts the given message to all clients
 * except the client that sent the message.
 * 
 * @param message The message to broadcast.
 * @param sender_socket The socket of the client that
 * sent the message.
 */
void broadcast_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&client_list_mutex);

    for (int i = 0; i < num_clients; i++)
    {
        if (client_sockets[i] != sender_socket)
        {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }

    pthread_mutex_unlock(&client_list_mutex);
}

/**
 * @brief Broadcasts the given message to all clients
 * from the server.
 * 
 * @param message The message to broadcast.
 */
void broadcast_message_from_server(const char *message) {
    pthread_mutex_lock(&client_list_mutex);

    for (int i = 0; i < num_clients; i++)
    {
        send(client_sockets[i], message, strlen(message), 0);
    }

    pthread_mutex_unlock(&client_list_mutex);
}

/**
 * @brief Handles the client messages. The client
 * messages are broadcasted to all clients.
 * 
 * The client_handler function is designed to manage a client's connection 
 * in a multi-threaded server environment. It takes a pointer to a client_t
 * structure as an argument, which contains information about the client.
 *
 * First, it locks the client_list_mutex to safely add the client's socket 
 * to the client_sockets array if the current number of clients is less 
 * than the maximum allowed.
 *
 * Then, it enters a loop where it receives messages from the client. If no
 * bytes are received, it breaks the loop. Each received message is timestamped
 * and formatted for broadcast.
 *
 * The function then locks the display_mutex to safely print the message on the server. 
 * After unlocking the mutex, it broadcasts the message to all clients.
 *
 * When the client disconnects, the function cleans up by removing the client's socket 
 * from the client_sockets array, decrementing the number of clients, and unlocking the client_list_mutex.
 *
 * Finally, it closes the client's socket, prints a disconnection message, and frees 
 * the memory allocated for the client before returning NULL.
 * 
 * @param arg The client that sent the message.
 * @return void* 
 */
void *client_handler(void *arg) {
    client_t *client = (client_t *)arg;
    int client_socket = client->socket;

    pthread_mutex_lock(&client_list_mutex);
    if (num_clients < MAX_CLIENTS)
    {
        client_sockets[num_clients++] = client_socket;
    }
    pthread_mutex_unlock(&client_list_mutex);

    char buffer[1024];
    char broadcast_buffer[1080];

    while (1)
    {
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) { break; }

        buffer[bytes_received] = '\0';

        time_t now = time(NULL);
        struct tm *now_tm = localtime(&now);
        char time_str[9];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", now_tm);

        snprintf(broadcast_buffer, sizeof(broadcast_buffer), "\n[%s] %s (%s) says: %s\n\n", time_str, client->hostname, client->ip, buffer);

        pthread_mutex_lock(&display_mutex);
        printf("\n%s\n", broadcast_buffer);
        pthread_mutex_unlock(&display_mutex);

        broadcast_message(broadcast_buffer, client_socket);
    }

    pthread_mutex_lock(&client_list_mutex);
    for (int i = 0; i < num_clients; i++)
    {
        if (client_sockets[i] == client_socket)
        {
            client_sockets[i] = client_sockets[num_clients - 1];
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&client_list_mutex);

    close(client_socket);
    printf("\nClient %s (%s) disconnected.\n", client->hostname, client->ip);
    free(client);

    return NULL;
}

/**
 * @brief Handles the new client. The client is
 * added to the client list and a new thread is
 * created for the client.
 * 
 * @param client_socket The socket of the new client.
 * @param client_addr The address of the new client.
 */
void handle_new_client(int client_socket, struct sockaddr_in *client_addr)
{
    client_t *client = malloc(sizeof(client_t));
    if (client == NULL)
    {
        perror("Failed to allocate memory for client");
        close(client_socket);
        return;
    }

    client->socket = client_socket;
    inet_ntop(AF_INET, &client_addr->sin_addr, client->ip, INET_ADDRSTRLEN);

    if (getnameinfo((struct sockaddr *)&client_addr, sizeof(client_addr), client->hostname, HOSTNAME_LENGTH, NULL, 0, 0) != 0)
    {
        strncpy(client->hostname, "Unknown", HOSTNAME_LENGTH);
    }

    if (create_and_detach_thread(client_handler, client) != 0)
    {
        perror("Could not create/detach thread for client");
        free(client);
        close(client_socket);
    }
}