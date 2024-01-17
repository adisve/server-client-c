#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

int server_socket = 0;
volatile sig_atomic_t keep_running = 1;
int client_sockets[MAX_CLIENTS];
int num_clients = 0;

pthread_mutex_t client_list_mutex;
pthread_mutex_t display_mutex;

void sigint_handler(int _) {
    (void) _;
    keep_running = 0;
}

void server_init()
{
    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse the address and port
    struct sockaddr_in server_addr;

    // Zero out the struct, to avoid garbage values
    memset(&server_addr, 0, sizeof(server_addr));

    // Set up the server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Attempt to bind the socket to the address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }

    // Initialize mutex for display and client list
    pthread_mutex_init(&display_mutex, NULL);
    pthread_mutex_init(&client_list_mutex, NULL);
}

void server_start()
{
    signal(SIGINT, sigint_handler);

    // Start listening for connections,
    // with a maximum of MAX_CLIENTS connections in the queue
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", SERVER_PORT);

    pthread_t input_thread_id;
    if (pthread_create(&input_thread_id, NULL, server_input_handler, NULL) != 0)
    {
        perror("Could not create thread for server input");
        exit(EXIT_FAILURE);
    }
    pthread_detach(input_thread_id);

    while (keep_running)
    {
        // Accept a connection
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // Accept the connection and attempt to create a new socket for it
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        // Attempt to allocate memory for client socket and pass to the thread
        int *client_socket_ptr = malloc(sizeof(int));
        if (client_socket_ptr == NULL)
        {
            perror("Unable to allocate memory for client socket");
            continue;
        }
        *client_socket_ptr = client_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_handler, client_socket_ptr) != 0)
        {
            perror("Could not create thread for client");
            free(client_socket_ptr); // If thread creation fails, free allocated memory
        }
        else
        {
            pthread_detach(thread_id);
        }
    }

    server_stop();
}

void server_stop()
{
    close(server_socket);
    pthread_mutex_destroy(&display_mutex);
    pthread_mutex_destroy(&client_list_mutex);
    printf("Server stopped\n");
}

void *client_handler(void *arg)
{
    // Get the client socket from the argument
    int client_socket = *(int *)arg;
    free(arg);

    pthread_mutex_lock(&client_list_mutex);
    if (num_clients < MAX_CLIENTS) {
        client_sockets[num_clients++] = client_socket;
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
        snprintf(broadcast_buffer, sizeof(broadcast_buffer), "[%s] Client %d says: %s\n", time_str, client_socket, buffer);

        // Lock the display mutex and print the message on the server
        pthread_mutex_lock(&display_mutex);
        printf("%s", broadcast_buffer);
        pthread_mutex_unlock(&display_mutex);

        // Broadcast the message
        broadcast_message(broadcast_buffer);
    }

    pthread_mutex_lock(&client_list_mutex);

    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] == client_socket) {
            client_sockets[i] = client_sockets[num_clients - 1];
            num_clients--;
            break;
        }
    }

    pthread_mutex_unlock(&client_list_mutex);

    // Close the client socket
    close(client_socket);
    printf("Client %d disconnected.\n", client_socket);
    return NULL;
}

void broadcast_message(const char *message)
{
    pthread_mutex_lock(&client_list_mutex);

    for (int i = 0; i < num_clients; i++) {
        send(client_sockets[i], message, strlen(message), 0);
    }

    pthread_mutex_unlock(&client_list_mutex);
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
            snprintf(server_message, sizeof(server_message), "[%s] Server says: %s\n\n", time_str, input_buffer);

            // Broadcast the server message
            broadcast_message(server_message);
        }
    }

    return NULL;
}