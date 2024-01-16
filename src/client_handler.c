#include <unistd.h>

void *client_handler(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg);

    char buffer[1024];

    while (1)
    {
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            break;
        }
        send(client_socket, buffer, bytes_received, 0);
    }

    close(client_socket);
    printf("Client disconnected.\n");
    return NULL;
}