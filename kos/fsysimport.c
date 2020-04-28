#include <fsysimport.h>	

FILE* ImportFile(int shardId, hash_head* shard)				//alocação de memória para a shard
{
	FILE* f;
	char name[15];
	char key[KV_SIZE];
	char value[KV_SIZE];
	sprintf(name, "fshardId%d.txt", shardId);
	f=fopen(name,"r+");
	
	if (f==NULL)
	{
		f=fopen(name,"w+");	
		return f;
	}
	fseek(f, 0, SEEK_END);
	long deslocamento_fim=ftell(f);
	rewind(f);
	while(ftell(f) < deslocamento_fim)
	{
		fread(key, sizeof(char), KV_SIZE, f);
		fread(value, sizeof(char), KV_SIZE, f);
		if (key[0]!='\0')
		{
			hash_import_insert(shard, key, value, ftell(f));
		}
		fseek(f, 1, SEEK_CUR);
	}	
	return f;
}




916381051


