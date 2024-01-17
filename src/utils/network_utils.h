// network_utils.h

#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <netinet/in.h>

int create_server_socket();
void configure_server_socket(int server_socket, struct sockaddr_in *server_addr);

#endif