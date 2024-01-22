// thread_utils.c

/**
 * @file thread_utils.c
 * @author Adis Veletanlic
 * @brief The main purpose of this file is to provide
 * the implementation of the thread utilities. It contains
 * the functions for initializing and destroying mutexes,
 * as well as the function for creating and detaching
 * threads.
 * 
 * @version 0.1
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "thread_utils.h"
#include <stdio.h>

/**
 * @brief Initializes the mutexes, one for the display
 * and one for the client list that is shared between
 * the client handler and the server input handler.
 * 
 */
void initialize_mutexes() {
    pthread_mutex_init(&display_mutex, NULL);
    pthread_mutex_init(&client_list_mutex, NULL);
}

/**
 * @brief Destroys the mutexes.
 * 
 */
void destroy_mutexes() {
    pthread_mutex_destroy(&display_mutex);
    pthread_mutex_destroy(&client_list_mutex);
}

/**
 * @brief Creates a new thread and detaches it, so that
 * it runs independently from the main thread. If the
 * thread creation fails, the function returns -1. We
 * detach threads that are not joined, because
 * otherwise they would remain in the system as zombie
 * threads.
 * 
 * @param start_routine The function that the thread
 * will execute.
 * @param arg The argument that will be passed to the
 * start_routine function.
 * @return int 0 if the thread was created successfully,
 * -1 otherwise.
 */
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