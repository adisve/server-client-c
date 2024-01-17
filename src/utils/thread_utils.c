// thread_utils.c

#include "thread_utils.h"

void initialize_mutexes() {
    pthread_mutex_init(&display_mutex, NULL);
    pthread_mutex_init(&client_list_mutex, NULL);
}

void destroy_mutexes() {
    pthread_mutex_destroy(&display_mutex);
    pthread_mutex_destroy(&client_list_mutex);
}