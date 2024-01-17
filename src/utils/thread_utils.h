// thread_utils.h

#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#pragma once

#include <pthread.h>

pthread_mutex_t client_list_mutex;
pthread_mutex_t display_mutex;

void initialize_mutexes();
void destroy_mutexes();
int create_and_detach_thread(void *(*start_routine) (void *), void *arg);
#endif