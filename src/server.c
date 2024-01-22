// server.c

/**
 * @file server.c
 * @author Adis Veletanlic
 * @brief The main purpose of this file is to provide
 * the implementation of the server. It contains the
 * main function, as well as the functions for
 * initializing, starting and stopping the server.
 * 
 * The server is implemented as a TCP server that
 * listens for incoming connections on port 8080.
 * 
 * The server is able to handle multiple clients
 * simultaneously. The maximum number of clients
 * is defined by the MAX_CLIENTS constant.
 * 
 * @version 0.1
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include "server.h"
#include "./utils/network_utils.h"
#include "./utils/thread_utils.h"
#include "client_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

int server_socket = 0;
volatile sig_atomic_t keep_running = 1;
int num_clients = 0;
int client_sockets[MAX_CLIENTS];

/**
 * @brief Handles the SIGINT signal by setting the
 * keep_running flag to 0.
 * 
 * @param _ The signal number.
 */
void sigint_handler(int _)
{
    keep_running = 0;
}

/**
 * @brief Initializes the server by creating the
 * server socket and configuring it.
 * 
 */
void server_init()
{
    server_socket = create_server_socket();
    struct sockaddr_in server_addr;
    configure_server_socket(server_socket, &server_addr);
    initialize_mutexes();
}

/**
 * @brief Starts listening for incoming connections
 * on the server socket.
 * 
 * @param server_socket The server socket.
 */
void start_listening(int server_socket)
{
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("\nServer started on port %d\n", SERVER_PORT);
}

/**
 * @brief Accepts new clients and handles them by
 * creating a new thread for each client.
 * 
 * @param server_socket The server socket.
 */
void accept_clients(int server_socket)
{
    while (keep_running)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Accept failed");
            continue;
        }
        handle_new_client(client_socket, &client_addr);
    }
}

/**
 * @brief Broadcasts the given message to all clients.
 * 
 * @param message The message to be broadcasted.
 */
void *server_input_handler(void *arg)
{
    char input_buffer[1024];
    char server_message[1064];

    while (keep_running) {
        if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL)
        {
            input_buffer[strcspn(input_buffer, "\n")] = 0;

            time_t now = time(NULL);
            struct tm *now_tm = localtime(&now);
            char time_str[9];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", now_tm);

            snprintf(server_message, sizeof(server_message), "\n[%s] Server says: %s\n\n", time_str, input_buffer);
            broadcast_message_from_server(server_message);
        }
    }

    return NULL;
}

/**
 * @brief Starts the server by calling the
 * start_listening and accept_clients functions.
 * 
 */
void server_start()
{
    signal(SIGINT, sigint_handler);
    start_listening(server_socket);
    server_input_init();
    accept_clients(server_socket);
}

/**
 * @brief Stops the server by closing the server
 * socket and destroying the mutexes.
 * 
 */
void server_input_init()
{
    if (create_and_detach_thread(server_input_handler, NULL) != 0)
    {
        perror("Could not create thread for server input");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Stops the server by closing the server
 * socket and destroying the mutexes.
 * 
 */
void server_stop()
{
    close(server_socket);
    destroy_mutexes();
}

/**
 * @brief The main function. It initializes the
 * server, starts it and then stops it.
 * 
 * @return int 0 if the program was executed
 * successfully, -1 otherwise.
 */
int main()
{
    server_init();
    server_start();
    server_stop();
    return 0;
}