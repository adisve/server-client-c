// client_handler.h

#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#pragma once

#include <netinet/in.h>

void *client_handler(void *arg);
void broadcast_message(const char *message, int sender_socket);
void broadcast_message_from_server(const char *message);
void handle_new_client(int client_socket, struct sockaddr_in *client_addr);

#endif