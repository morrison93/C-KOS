#ifndef FSYSTEM_H
#define FSYSTEM_H 1

#include <kos_client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long FileWriter(FILE* f, char* key, char* value);

void FileLineRemover(FILE* f, long deslocamento);

#endif
