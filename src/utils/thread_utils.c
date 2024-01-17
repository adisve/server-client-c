// thread_utils.c

#include "thread_utils.h"
#include <stdio.h>

void initialize_mutexes() {
    pthread_mutex_init(&display_mutex, NULL);
    pthread_mutex_init(&client_list_mutex, NULL);
}

void destroy_mutexes() {
    pthread_mutex_destroy(&display_mutex);
    pthread_mutex_destroy(&client_list_mutex);
}

int create_and_detach_thread(void *(*start_routine) (void *), void *arg) {
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, start_routine, arg) != 0)
    {
        perror("Could not create thread");
        return -1;
    }
    pthread_detach(thread_id);
    return 0;
}