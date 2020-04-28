#include <biblioteca.h>
#include <fsystem.h>

list_head* newlist(){
	list_head *list;
	list = (list_head*) malloc(sizeof(list_head));
   list->first = NULL;
   return list;
}


list_node* new_node(char* key, char* value, long deslocamento)
{
	list_node *node;
	node=(list_node*) malloc(sizeof(list_node));
	strncpy(node->kv.key, key, KV_SIZE);		//ATENÇÃO: SEG FAULT POSSIBILITY!!!
	strncpy(node->kv.value, value, KV_SIZE);
	node->deslocamento = deslocamento;
	node->next=NULL;
	return node;    
}


void list_insert_node(list_head *list, list_node *insert_node)
{
	insert_node->next=list->first; 
	list->first=insert_node;
}


char* remove_node(FILE* f, list_head *list, char* key)		
{
	char* return_value=(char*)calloc(KV_SIZE,sizeof(char));			 //<- NAO ESQUECER DE FAZER FREE AO CALLOC!
	list_node *aux_node, *prev_node;
	aux_node=list->first;
	
	
	if(list->first==NULL)
	{
		free(return_value);
		return NULL;
	}
		
	if(strcmp(aux_node->kv.key,key)==0)
	{
		strncpy(return_value, aux_node->kv.value, KV_SIZE);
		list->first = aux_node->next;
		FileLineRemover(f, aux_node->deslocamento);
		free(aux_node);
		return return_value;
	}	
		
	while(strcmp(aux_node->kv.key,key)!=0)
	{
		prev_node=aux_node;
		aux_node=aux_node->next;
		if(aux_node==NULL)
		{
			free(return_value);
			return NULL;
		}
	}
	prev_node->next=aux_node->next;
	strncpy(return_value, aux_node->kv.value, KV_SIZE);
	FileLineRemover(f, aux_node->deslocamento);
	free(aux_node);
	return return_value;
}


//recebe key e retorna value;

char* list_procura(list_head *list, char* key)  // NAO ESQUECER DE FAZER FREE AOS STRDUPS 
{
	char* return_value;
	list_node *aux_procura;
	aux_procura=list->first;
		
	if(list->first==NULL)
		return NULL;
		
	if(strcmp(aux_procura->kv.key,key)==0)
	{
		return_value=strdup(aux_procura->kv.value);
		return return_value;
	}
	while(strcmp(aux_procura->kv.key,key)!=0)
	{
		aux_procura=aux_procura->next;
		if(aux_procura==NULL)
			return NULL;
	}
	return_value=strdup(aux_procura->kv.value);
	return return_value;	
}

int count_list(list_head* list)
{
	int i=0;
	list_node *aux_node;
	
	aux_node=list->first;

	while(aux_node!=NULL)
	{
		i++;
		aux_node=aux_node->next;
	}	
	
	return i;
}



int list_getAll(list_head* list, KV_t* vector, int dim, int pos)
{
	list_node* aux_node;
	
	if(list->first==NULL)
		return pos;
	
	aux_node=list->first;
	
	while(pos<dim && aux_node!=NULL)
	{
		strncpy(vector[pos].key, aux_node->kv.key, KV_SIZE);
		strncpy(vector[pos].value, aux_node->kv.value, KV_SIZE);
		pos++;
		aux_node=aux_node->next;
	}
	
	return pos;		
}

char* remove_import_node(list_head *list, char* key)		
{
	char* return_value=(char*)calloc(KV_SIZE,sizeof(char));			 //<- NAO ESQUECER DE FAZER FREE AO CALLOC!
	list_node *aux_node, *prev_node;
	aux_node=list->first;
	
	
	if(list->first==NULL)
	{
		free(return_value);
		return NULL;
	}
		
	if(strcmp(aux_node->kv.key,key)==0)
	{
		strncpy(return_value, aux_node->kv.value, KV_SIZE);
		list->first = aux_node->next;
		free(aux_node);
		return return_value;
	}	
		
	while(strcmp(aux_node->kv.key,key)!=0)
	{
		prev_node=aux_node;
		aux_node=aux_node->next;
		if(aux_node==NULL)
		{
			free(return_value);
			return NULL;
		}
	}
	prev_node->next=aux_node->next;
	strncpy(return_value, aux_node->kv.value, KV_SIZE);
	free(aux_node);
	return return_value;
}
