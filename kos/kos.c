//#include <kos.h>
#include <kos_client.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#include <buffer.h>
#include <hash.h>
#include <server.h>
#include <fsysimport.h>

int clientpointer;
int buffer_size;
buffposition** buffer;
sem_t* client_can_produce;
sem_t* server_can_consume;

int kos_init(int num_server_threads, int buf_size, int num_shards) {
	//-------------------Creating server thread pointers
	pthread_t* thr_server = (pthread_t*) malloc(sizeof(pthread_t)*num_server_threads); //vector for server threads
	pthread_mutex_t* changing_server_pointer = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)); //mutex pointer control
	changing_client_pointer = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)); //mutex pointer control
	client_can_produce = (sem_t*) malloc(sizeof(sem_t));
	server_can_consume = (sem_t*) malloc(sizeof(sem_t));
	
	sem_init(client_can_produce, 0, buf_size);
	sem_init(server_can_consume, 0, 0);
	
	clientpointer = 0;
	pthread_mutex_init(changing_client_pointer, NULL);
	pthread_mutex_init(changing_server_pointer, NULL);
	buffer_size = buf_size;
	
	//-------------------Creation of writer/reader variables
	controlo_shards* acesso_shards = (controlo_shards*) malloc(sizeof(controlo_shards));
	sem_init(&acesso_shards->sem_shards_read, 0, 0);
	sem_init(&acesso_shards->sem_shards_write, 0, 0);
	acesso_shards->n_leitores = 0;
	acesso_shards->leitores_espera = 0;
	acesso_shards->escritores_espera = 0;
	acesso_shards->em_escrita = false;
	pthread_mutex_init(&acesso_shards->trinco, NULL);
	
	//-------------------Alocating buffer
	buffer = (buffposition**) malloc(sizeof(buffposition*)*(buf_size+1));
	
	int i;
	for(i=0; i<(buf_size+1); i++){
		buffer[i] = (buffposition*) malloc(sizeof(buffposition));
		if (pthread_mutex_init(&buffer[i]->inUse, NULL)!=0)
			return -1;
		if (pthread_mutex_init(&buffer[i]->pos_exists, NULL)!=0)
			return -1;
		pthread_mutex_lock(&buffer[i]->pos_exists);
	}
	
		
	//-------------------Allocating shards
	hash_head** shards = (hash_head**) malloc(sizeof(hash_head*)*num_shards);
	FILE** f = (FILE**) calloc(num_shards, sizeof(FILE*));
	for(i=0; i<num_shards; i++){
		shards[i]=init();
		f[i] = ImportFile(i, shards[i]);
		}
	
	//-------------------Starting server threads
	int* serverpointer = (int*) malloc(sizeof(int));
	*serverpointer = 0;
	
	wrapper* wrap = (wrapper*) malloc(sizeof(wrapper));		//wrapper needed to pass 4 arguments to server thread
	
		wrap->server_pointer = serverpointer;				//pointer to be incremented
		wrap->changing_server_pointer = changing_client_pointer;//mutex for shared pointer incrementation
		wrap->shards = shards;								//all servers access all shards
		wrap->shardfiles = f;
		
		wrap->acesso_shards = acesso_shards;				//shard access control
	
	for (i=0;i<num_server_threads;i++){
		if(pthread_create(&thr_server[i], NULL, server, (void *) wrap) != 0)
			return -1;						//Failure in creating threads (pass information to client)
	}
	//

	return 0;		//Success

}
buffstruct* produce(){
	buffstruct* bufferstructure = (buffstruct*) malloc(sizeof(buffstruct));
	sem_init(&bufferstructure->client_can_consume, 0, 0);
	
	return bufferstructure;
} 

buffposition* choose_client_position(){
	pthread_mutex_lock(changing_client_pointer);	//SYNC - Avoid placing two items in the same place
		buffposition* bufferaccess = buffer[clientpointer];
		clientpointer = (clientpointer+1) % buffer_size;
	pthread_mutex_unlock(changing_client_pointer);
	return bufferaccess;
}

char* kos_get(int clientid, int shardId, char* key) {
	char*	temp_string;	//pointer to return to the client.														
	buffposition* bufferaccess;		//position this client is going to use
	buffstruct* bufferstructure;
	
	
	//------Preparing buffer--------//
	bufferstructure = produce();
	
	//-------Preparing query--------//
	bufferstructure->shardId = shardId;
	
	bufferstructure->args = (char**) calloc(2,sizeof(char*));	//WARNING: Server replaces args[1] with answer
		bufferstructure->args[0] = strdup("get");
		bufferstructure->args[1] = strdup(key);
		
	sem_wait(client_can_produce);	//waiting for buffer queue
		
	//-------Choosing position------//
	bufferaccess = choose_client_position();
	pthread_mutex_lock(&bufferaccess->inUse);		//SYNC - check if position is in use REMEMBER TO UNLOCK
	
	bufferaccess->position = bufferstructure;
	pthread_mutex_unlock(&bufferaccess->pos_exists);
	
	//-----Sending to server--------//
	sem_post(server_can_consume);								//SYNC - Tell server that info is there
	sem_wait(&bufferstructure->client_can_consume);				//SYNC - Wait for response
	
	//Accepting answer&&Cleaning buffer//
	if(bufferstructure->args[1] == NULL)
		temp_string = NULL;										//because we cant strdup(NULL)
	else{
		temp_string = strdup(bufferstructure->args[1]);
		free(bufferstructure->args[1]);						//already copied
	}
	free(bufferstructure->args);						//FREE - args vector
	free(bufferstructure);
	

	//---Sending answer to client--//
	return temp_string;
}



char* kos_put(int clientid, int shardId, char* key, char* value) {
	char* temp_string;
	buffposition* bufferaccess;		//position this client is going to use
	buffstruct* bufferstructure;
	
	
	//------Preparing buffer--------//
	bufferstructure = produce();
	
	//-------Preparing query--------//
	bufferstructure->shardId = shardId;
	
	bufferstructure->args = (char**) calloc(3,sizeof(char*));		//WARNING: Server should free these memories.
		bufferstructure->args[0] = strdup("put");
		bufferstructure->args[1] = strdup(key);
		bufferstructure->args[2] = strdup(value);
		
	sem_wait(client_can_produce);	//waiting for buffer queue
	
	//-------Choosing position------//
	bufferaccess = choose_client_position();
	pthread_mutex_lock(&bufferaccess->inUse);		//SYNC - check if position is in use REMEMBER
	
	bufferaccess->position = bufferstructure;
	pthread_mutex_unlock(&bufferaccess->pos_exists);
	
	//-----Sending to server--------//
	sem_post(server_can_consume);								//SYNC - Tell server that info is there
	sem_wait(&bufferstructure->client_can_consume);				//SYNC - Wait for responsee
	
	//Cleaning buffer&&Accepting answer//
	if(bufferstructure->args[1] == NULL)
		temp_string = NULL;											//because we cant strdup(NULL)
	else{
		temp_string = strdup(bufferstructure->args[1]);
		free(bufferstructure->args[1]);							//already copied
	}
	free(bufferstructure->args);									//FREE - args vector
	
	
	return temp_string;
}

char* kos_remove(int clientid, int shardId, char* key) {
	char* temp_string;
	buffposition* bufferaccess;		//position this client is going to use
	buffstruct* bufferstructure;
	
	
	//------Preparing buffer--------//
	bufferstructure = produce();
	
	
	bufferstructure->shardId = shardId;
	
	//------Peparing query----------//
	bufferstructure->args = (char**) calloc(2,sizeof(char*));		//WARNING: Server should free these memories.
		bufferstructure->args[0] = strdup("rem");
		bufferstructure->args[1] = strdup(key);
		
	sem_wait(client_can_produce);	//waiting for buffer queue
	
	//-------Choosing position------//
	bufferaccess = choose_client_position();
	pthread_mutex_lock(&bufferaccess->inUse);		//SYNC - check if position is in use REMEMBER TO UNLOCK
	
	bufferaccess->position = bufferstructure;
	pthread_mutex_unlock(&bufferaccess->pos_exists);
	
	//-----Sending to server--------//
	sem_post(server_can_consume);								//SYNC - Tell server that info is there
	sem_wait(&bufferstructure->client_can_consume);				//SYNC - Wait for responsee
	
	//Cleaning buffer&&Accepting answer//
	if(bufferstructure->args[1] == NULL)
		temp_string = NULL;											//because we cant strdup(NULL)
	else{
		temp_string = strdup(bufferstructure->args[1]);
		free(bufferstructure->args[1]);							//already copied
	}
	free(bufferstructure->args);									//FREE - args vector
	
	
	return temp_string;
}

KV_t* kos_getAllKeys(int clientid, int shardId, int* dim) {
	KV_t* kv_pointer;
	buffposition* bufferaccess;		//position this client is going to use
	buffstruct* bufferstructure;
	
	//------Preparing buffer--------//
	bufferstructure = produce();
	
	//-------Preparing Query--------//
	bufferstructure->shardId = shardId;
	bufferstructure->dim = dim;									//passing the pointer to dim as argument (server directly alters dim)
	bufferstructure->args = (char**) calloc(1,sizeof(char*));
		bufferstructure->args[0] = strdup("gak");
	
		//-------Choosing position------//
	bufferaccess = choose_client_position();
	pthread_mutex_lock(&bufferaccess->inUse);		//SYNC - check if position is in use REMEMBER TO UNLOCK
	
	bufferaccess->position = bufferstructure;
	pthread_mutex_unlock(&bufferaccess->pos_exists);
	
	sem_wait(client_can_produce);	//waiting for buffer queue
	
	//-----Sending to server--------//
	sem_post(server_can_consume);								//SYNC - Tell server that info is there
	sem_wait(&bufferstructure->client_can_consume);				//SYNC - Wait for responsee

	//Accepting answer&&Clearing buffer//
	kv_pointer = bufferstructure->kv;
	bufferstructure->kv = NULL; //To make sure no one messes with this by accident
	free(bufferstructure->args);									//FREE - args vector

	return kv_pointer;
}


