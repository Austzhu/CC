#ifndef __KALMAN_H__
#define __KALMAN_H__
#include "include.h"

typedef struct kalman_t{
	float kal_A, kal_H, kal_Q, kal_R;
	float kal_X, kal_P;
	float (*kal_filter)(struct kalman_t*,float);
	void (*kal_release)(struct kalman_t**);
} kalman_t;

extern struct kalman_t *kalman_init(kalman_t *,int A,int H,int Q,int R);
#endif	//end of #ifndef __KALMAN_H__
