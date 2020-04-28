#include <hash.h>
#define HT_SIZE 10
#include <fsystem.h>

typedef list_head* hash_head;
 
int hash(char* key) 
{
    int i=0;
 
    if (key == NULL)
        return -1;
 
    while (*key != '\0') {
        i+=(int) *key;
        key++;
    }
 
    i=i % HT_SIZE;
 
    return i;
}


//Inicia um cabeçalho para a hashT e, cria uma lista ligada em cada posição
hash_head* init(){
	int i;
	hash_head* hash_tab=(hash_head*)calloc(HT_SIZE, sizeof(hash_head));			//<- ATENÇÃO FREE hash_tab;
		
	for(i=0; i<HT_SIZE; i++){
		hash_tab[i] = (hash_head) newlist();
	}
	return hash_tab;
}


//Recebe logo o ID em que entra um dado VALUE? OU tem que calcular esse ID?
char* hash_insert(FILE* f, hash_head* hash_tab, char* key, char* value)
{
	char* return_value;
	int i = hash(key);
	long deslocamento = FileWriter(f, key, value);
	list_node* node = new_node(key, value, deslocamento);
	
	return_value = remove_node(f, hash_tab[i], key);
	list_insert_node((list_head*)hash_tab[i], node);
	return return_value;
}


char* hash_procura(hash_head* hash_tab, char* key)
{
	int i = hash(key);
	char* resultado = list_procura((list_head*)hash_tab[i], key); //list_procura -> PRECSA (KEY + HEADER) -> DEVOLVE CHAR*
	
	return resultado;
}


char* hash_remove(FILE* f, hash_head* hash_tab, char* key)
{
	int i = hash(key);
	char* resultado = remove_node(f, (list_head*)hash_tab[i], key);
	
	return resultado;
}


KV_t* hash_getAll(hash_head* hash_tab, int* dim)
{
	int i=0, count=0;
	int pos=0;
	KV_t* vector;
	
	
	for(i=0; i<HT_SIZE; i++)
	{
		count+=count_list((list_head*) hash_tab[i]);
	}
	vector=(KV_t*)calloc(count, sizeof(KV_t));
	i=0;
	
	while(i<HT_SIZE)
	{
		pos=list_getAll((list_head*) hash_tab[i], vector, count, pos);
		i++;
	}
	*dim=count;
	return vector;
}

char* hash_import_insert(hash_head* hash_tab, char* key, char* value, long deslocamento)
{
	char* return_value;
	int i = hash(key);
	list_node* node = new_node(key, value, deslocamento);
	
	return_value = remove_import_node(hash_tab[i], key);
	list_insert_node((list_head*)hash_tab[i], node);
	return return_value;
}
