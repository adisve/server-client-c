// client_handler.h

#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#pragma once

void *client_handler(void *arg);
void broadcast_message(const char *message, int sender_socket);
void broadcast_message_from_server(const char *message);

#endif