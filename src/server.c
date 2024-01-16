#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bits/pthreadtypes.h>
#include <signal.h>
#include "client_handler.h"

int server_socket = 0;
volatile sig_atomic_t keep_running = 1;

void sigint_handler(int _) {
    (void) _;
    keep_running = 0;
}

void server_init(int port)
{
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }
}

void server_start()
{
    signal(SIGINT, sigint_handler);

    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", SERVER_PORT);

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

        // Allocate memory for client socket and pass to the thread
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
    printf("Server stopped\n");
}