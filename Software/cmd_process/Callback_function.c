#include "Callback_function.h"

 s32 CallBack_ShortResponse(Node_t *node)
{
	assert_param(node,NULL,FAIL);
	return SUCCESS;
}

s32 CallBack_LongResponse(Node_t *node)
{
	assert_param(node,NULL,FAIL);
	return SUCCESS;
}
s32 CallBack_answer(Node_t *node)
{
	assert_param(node,NULL,FAIL);
	int size = node->package[1]+2;
	printf("********************\n");
	printf("node data:");
	for(int i=0;i< size;++i){
		printf("%02x ",node->package[i]);
	}printf("\n");
	printf("********************\n");
	return SUCCESS;
}
