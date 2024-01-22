// server.h

#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

#define MAX_CLIENTS 10
#define SERVER_PORT 8080
#define HOSTNAME_LENGTH 256

/**
 * @brief Struct for storing client information
 * that will be displayed in the terminal.
 * 
 */
typedef struct {
    int socket;
    char ip[INET_ADDRSTRLEN];
    char hostname[HOSTNAME_LENGTH];
} client_t;


extern int num_clients;
extern int client_sockets[MAX_CLIENTS];

void sigint_handler(int _);
void server_init();
void server_start();
void server_stop();
void server_input_init();
void start_listening(int server_socket);
void accept_clients(int server_socket);
void *server_input_handler(void *arg);

#endif
