#ifndef KOSH_H
#define KOSH_H 1
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#include <fsysimport.h>
#include <buffer.h>
#include <hash.h>
#include <server.h>

int buffer_size;
buffposition** buffer;
sem_t* client_can_produce;
sem_t* server_can_consume;
#endif
