#ifndef BUFFER_H
#define BUFFER_H 1

#include <semaphore.h>
#include <pthread.h>
#include <hash.h>
#include <kos_client.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct buffstruct{
	sem_t client_can_consume;
	char** args;
	int shardId;
	int* dim;
	KV_t* kv; 
} buffstruct;

typedef struct controlo_shards{
	sem_t sem_shards_read;			//Iniciado a zero
	sem_t sem_shards_write;			//Iniciado a zero

	int n_leitores;			//Numero de servidores a aceder a SHARDs, em MODO LEITURA.
	int leitores_espera;		//Numero de servidores a aguardar permissão para aceder a SHARDs, em MODO LEITURA.
	int escritores_espera;		//Numero de servidores a aguardar permissão para aceder a SHARDs, em MODE ESCRITA.

	bool em_escrita;	//FLAG indicadora do tipo de fluxo (Leitura ou escrita).  0-Em leitura.   1-Em escrita.

	pthread_mutex_t trinco;		//Trinco usado para bloquear ou desbloquear uma dada SHARD.
}controlo_shards;

typedef struct buffposition{
	buffstruct* position;
	pthread_mutex_t inUse;
	pthread_mutex_t pos_exists;
} buffposition;

typedef struct wrapper{
	hash_head** shards;
	int* server_pointer;
	pthread_mutex_t* changing_server_pointer;
	FILE** shardfiles;
	
	controlo_shards* acesso_shards;
}wrapper;

extern sem_t* client_can_produce;
extern int buffer_size;
extern sem_t* server_can_consume;
extern buffposition** buffer;

pthread_mutex_t* changing_client_pointer;

#endif
