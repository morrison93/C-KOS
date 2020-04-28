#include <fsystem.h>

long FileWriter(FILE* f, char* key, char* value)
{
	char fkey[KV_SIZE], fvalue[KV_SIZE];
	
	strncpy(fkey,key, KV_SIZE);
	strncpy(fvalue,value, KV_SIZE);
	
	fseek(f, 0, SEEK_END);
	long return_deslocamento=ftell(f);
	
	fwrite(fkey , sizeof(char), KV_SIZE, f);
	fwrite(fvalue , sizeof(char), KV_SIZE, f);
	fprintf(f, "\n");
	
	return return_deslocamento;
}


void FileLineRemover(FILE* f, long deslocamento)
{
	char empty[KV_SIZE*2];
	strncpy(empty,"\0", KV_SIZE*2);
	
	fseek(f, deslocamento, SEEK_SET);
	fwrite(empty, sizeof(char), (KV_SIZE*2), f);
}


