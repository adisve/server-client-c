/*
* Implements the core server functionality like socket creation, 
* binding, listening, and accepting clients.
*/

#pragma once

#ifndef SERVER_H
#define SERVER_H

#define SERVER_PORT 9999
#define MAX_CLIENTS 10

void server_init();

void server_start();

void server_stop();

void sigint_handler(int _);

#endif //SERVER_H