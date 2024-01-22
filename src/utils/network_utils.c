// network_utils.c

/**
 * @file network_utils.c
 * @author Adis Veletanlic
 * @brief The main purpose of this file is to provide
 * the implementation of the network utilities. It contains
 * the functions for creating and configuring the server
 * socket.
 * 
 * @version 0.1
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "network_utils.h"
#include "../server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Creates the server socket by calling the
 * socket function. If the socket creation fails,
 * the program exits with EXIT_FAILURE.
 * 
 * @return int The server socket.
 */
int create_server_socket() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return server_socket;
}

/**
 * @brief Configures the server socket with the
 * given server address. The server address is
 * configured to use the INADDR_ANY address and
 * the port 8080.
 * 
 * @param server_socket The server socket.
 * @param server_addr The server address.
 */
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
