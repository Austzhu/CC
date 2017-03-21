#ifndef __MYTHREAD_H__
#define __MYTHREAD_H__
#include "include.h"


typedef struct MyThread{
	void (*func)(void*);
} MyThread_t;

#endif
