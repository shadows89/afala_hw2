#include "stdio.h"
#include "stdlib.h"
#include "mylist.h"

void* print(void* data){
	printf("Data: %d\n",*(int*)data);
	return NULL;
}

int main(){
	int data = 1;
	void** res = malloc(sizeof(int*));
	void** res2 = res;
	linked_list_t** test = list_alloc();
	if(!list_insert(test,1, &data))
		printf("Insert DONE!!\n");
	if(!list_insert(test,-12, &data))
		printf("Insert DONE!!\n");
	if(list_contains(test,1))
		printf("Found It!\n");
	if(list_contains(test,-12))
		printf("Found It!\n");
	list_node_compute(test,-12,&print,res);
	res = res2;
	list_node_compute(test,1,&print,res);
	data = 10;
	list_update_node(test,-12,&data);
	res = res2;
	list_node_compute(test,-12,&print,res);
	if(!list_remove(test,-12))
		printf("Remove DONE!!\n");
	list_update_node(test,-12,&data);
	list_free(&test);
	free(res);
	return 0;
}




