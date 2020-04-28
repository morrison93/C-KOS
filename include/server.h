#ifndef SERVER_H
#define SERVER_H 1
#include <buffer.h>
#include <hash.h>
#include <delay.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h> //Needed? not included elsewhere?

void *server(void* arg);

void inicia_leitura(controlo_shards* acesso_shards);

void acaba_leitura(controlo_shards* acesso_shards);

void inicia_escrita(controlo_shards* acesso_shards);

void acaba_escrita(controlo_shards* acesso_shards);

void server_read(FILE** shardfiles, buffstruct* buffer_pos, hash_head** shards, controlo_shards* acesso_shards);

char* server_get(hash_head* hash_tab, char* key);

char* server_put(FILE* f, hash_head* hash_tab, char* key, char* value);

char* server_remove(FILE* f, hash_head* hash_tab, char* key);

KV_t* server_getAll(hash_head* hash_tab, int* dim);

#endif


