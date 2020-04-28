#include <server.h>

//Leituras
void inicia_leitura(controlo_shards* acesso_shards)
{
	pthread_mutex_lock(&acesso_shards->trinco);		//LOCK MUTEX
	if (acesso_shards->em_escrita || acesso_shards->escritores_espera > 0) {
		acesso_shards->leitores_espera++;
		pthread_mutex_unlock(&acesso_shards->trinco);		//UNLOCK MUTEX
		sem_wait(&acesso_shards->sem_shards_read);
		pthread_mutex_lock(&acesso_shards->trinco);		//LOCK MUTEX
	}
	else acesso_shards->n_leitores++;
	pthread_mutex_unlock(&acesso_shards->trinco);			//UNLOCK MUTEX
}

void acaba_leitura(controlo_shards* acesso_shards)
{
	pthread_mutex_lock(&acesso_shards->trinco);			//LOCK MUTEX
	acesso_shards->n_leitores--;
	if (acesso_shards->n_leitores == 0 && acesso_shards->escritores_espera > 0){
		sem_post(&acesso_shards->sem_shards_write);
		acesso_shards->em_escrita=true;
		acesso_shards->escritores_espera--;}
	pthread_mutex_unlock(&acesso_shards->trinco);			//UNLOCK MUTEX
}

//Escrita
void inicia_escrita(controlo_shards* acesso_shards)
{
	pthread_mutex_lock(&acesso_shards->trinco);			//LOCK MUTEX
	if (acesso_shards->em_escrita || acesso_shards->n_leitores > 0) {
		acesso_shards->escritores_espera++;
		pthread_mutex_unlock(&acesso_shards->trinco);			//UNLOCK MUTEX
		sem_wait(&acesso_shards->sem_shards_write);
		pthread_mutex_lock(&acesso_shards->trinco);			//LOCK MUTEX
	}
	acesso_shards->em_escrita = true;
	pthread_mutex_unlock(&acesso_shards->trinco);			//UNLOCK MUTEX
}

void acaba_escrita(controlo_shards* acesso_shards)
{
	pthread_mutex_lock(&acesso_shards->trinco);			//LOCK MUTEX
	acesso_shards->em_escrita = false;
	if (acesso_shards->leitores_espera > 0){
		int i;
		for (i=0; i<acesso_shards->leitores_espera; i++) {
			sem_post(&acesso_shards->sem_shards_read);
			acesso_shards->n_leitores++; }
		acesso_shards->leitores_espera-=i;}
	else if (acesso_shards->escritores_espera > 0){
		sem_post(&acesso_shards->sem_shards_write);
		acesso_shards->em_escrita=true;
		acesso_shards->escritores_espera--;
	}
	pthread_mutex_unlock(&acesso_shards->trinco);			//UNLOCK MUTEX
}

char* server_get(hash_head* hash_tab, char* key)
{	
	return hash_procura(hash_tab, key);
}


char* server_put(FILE* f, hash_head* hash_tab, char* key, char* value)
{
	return hash_insert(f, hash_tab, key, value);
}


char* server_remove(FILE* f, hash_head* hash_tab, char* key)
{
	return hash_remove(f, hash_tab, key);
}


KV_t* server_getAll(hash_head* hash_tab, int* dim)
{
	return hash_getAll(hash_tab, dim);
}

void server_read(FILE** shardfiles, buffstruct* buffer_pos, hash_head** shards, controlo_shards* acesso_shards)
{
	if(strcmp(buffer_pos->args[0], "get")==0)
	{
		char* return_value;
		inicia_leitura(acesso_shards);		//inicio
		return_value=server_get(shards[buffer_pos->shardId], buffer_pos->args[1]);	//String or NULL
		acaba_leitura(acesso_shards);	//Fim
		free(buffer_pos->args[1]);
		if(return_value==NULL)
			buffer_pos->args[1]=NULL;
		else{
		buffer_pos->args[1]=strdup(return_value);
		free(return_value);
		}
	}
	
	if (strcmp(buffer_pos->args[0], "put")==0)
	{
		char* return_value;
		inicia_escrita(acesso_shards);	//Comeca escrita
		return_value=server_put(shardfiles[buffer_pos->shardId], shards[buffer_pos->shardId], buffer_pos->args[1], buffer_pos->args[2]);	//NULL or String
		acaba_escrita(acesso_shards);	//Termina escrita
		free(buffer_pos->args[1]);
		free(buffer_pos->args[2]);
		if(return_value==NULL)
			buffer_pos->args[1]=NULL;
		else{
		buffer_pos->args[1] = strdup(return_value);
		buffer_pos->args[2]=NULL;
		}
	}
	
	if (strcmp(buffer_pos->args[0], "rem")==0)
	{
		char* return_value;
		inicia_escrita(acesso_shards);	//comeca escrita
		return_value=server_remove(shardfiles[buffer_pos->shardId], shards[buffer_pos->shardId], buffer_pos->args[1]);	//String or NULL
		acaba_escrita(acesso_shards);	//Termina escrita
		free(buffer_pos->args[1]);
		if(return_value==NULL)
			buffer_pos->args[1]=NULL;
		else{
		buffer_pos->args[1]= strdup(return_value);
		}
	}
	
	if (strcmp(buffer_pos->args[0], "gak")==0)
	{
		KV_t* k;
		inicia_leitura(acesso_shards);	//Inicio
		k=server_getAll(shards[buffer_pos->shardId], buffer_pos->dim);
		acaba_leitura(acesso_shards);	//Fim
		buffer_pos->kv = k;		//retornar k ao buffer_pos;
	}
	free(buffer_pos->args[0]);		//This is no longer needed (so kos doesn't have to free it)
}




void* server(void* arg)	//arg is of type wrapper* and should be decoded
{
	buffstruct* access_buffer;
	buffposition* buffer_pos;
	FILE** shardfiles;
	
	//--Unwrapping arg recieved--//
	wrapper* wrap = (wrapper*) arg;
	hash_head **shards;
	int* server_pointer;
	pthread_mutex_t* changing_server_pointer;
	controlo_shards* acesso_shards;
	
	shardfiles = wrap->shardfiles;
	acesso_shards = wrap->acesso_shards;
	shards = wrap->shards;
	changing_server_pointer = wrap->changing_server_pointer;
	server_pointer = wrap->server_pointer;
	
	while(1){
	sem_wait(server_can_consume);	//SYNC - Wait for client.
	pthread_mutex_lock(changing_server_pointer);
		buffer_pos = buffer[*server_pointer];
		*server_pointer = (*server_pointer+1) % buffer_size;
	pthread_mutex_unlock(changing_server_pointer);
	//pthread_mutex_lock(&buffer_pos->pos_exists);
	access_buffer = buffer_pos->position;
	buffer_pos->position = NULL;


	pthread_mutex_unlock(&buffer_pos->inUse);
	sem_post(client_can_produce);
	
	server_read(shardfiles, access_buffer, shards, acesso_shards);
	//delay();				//To stress synchronization
	sem_post(&access_buffer->client_can_consume);	//SYNC - Tell client access is ready
	}
	
}

