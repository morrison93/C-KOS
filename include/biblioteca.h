#ifndef BIBLIOTECA_H
#define BIBLIOTECA_H 1

#include <stdio.h>
#include <stdlib.h>
#include <kos_client.h>
#include <string.h>

typedef struct list_node{
	struct list_node  *next;
	struct KV_t kv;	
	long deslocamento;	
}list_node;

typedef struct list_head{
	list_node *first;
}list_head;


list_node* new_node(char* key, char* value, long deslocamento);

void list_insert_node(list_head *list, list_node *insert_node);

list_head* newlist();

char* remove_node(FILE* f, list_head *list, char* key);	

char* remove_import_node(list_head *list, char* key);

char* list_procura(list_head *list, char* key);

int count_list(list_head* list);

int list_getAll(list_head* list, KV_t* vector, int dim, int pos);

#endif
