// network_utils.c

#include "network_utils.h"
#include "../server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_server_socket() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return server_socket;
}

void configure_server_socket(int server_socket, struct sockaddr_in *server_addr) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0)
    {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }
}
