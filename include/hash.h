#ifndef HASH_H
#define HASH_H 1

#include <biblioteca.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>


typedef list_head* hash_head;

hash_head* init();

char* hash_insert(FILE* f, hash_head* hash_tab, char* key, char* value);

char* hash_import_insert(hash_head* hash_tab, char* key, char* value, long deslocamento);

char* hash_procura(hash_head* hash_tab, char* key);

char* hash_remove(FILE* f, hash_head* hash_tab, char* key);

KV_t* hash_getAll(hash_head* hash_tab, int* dim);

#endif
