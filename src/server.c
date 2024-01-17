// server.c

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

void sigint_handler(int _)
{
    keep_running = 0;
}

void server_init()
{
    server_socket = create_server_socket();
    struct sockaddr_in server_addr;
    configure_server_socket(server_socket, &server_addr);
    initialize_mutexes();
}

void start_listening(int server_socket)
{
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("\nServer started on port %d\n", SERVER_PORT);
}

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

void server_start()
{
    signal(SIGINT, sigint_handler);
    start_listening(server_socket);
    server_input_init();
    accept_clients(server_socket);
}

void server_input_init()
{
    if (create_and_detach_thread(server_input_handler, NULL) != 0)
    {
        perror("Could not create thread for server input");
        exit(EXIT_FAILURE);
    }
}

void server_stop()
{
    close(server_socket);
    destroy_mutexes();
}

int main()
{
    server_init();
    server_start();
    server_stop();
    return 0;
}