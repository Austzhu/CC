#ifndef __CALLBACE_FUNCTION_H__
#define __CALLBACE_FUNCTION_H__
#include "taskque.h"
#include "Interface.h"

extern s32 CallBack_Response(Node_t*,void*);
extern s32 CallBack_answer(Node_t *node,void*);
extern s32 CallBack_Reset(Node_t *node,void*);
extern s32 CallBack_Config(Node_t *node,void*);
#endif
