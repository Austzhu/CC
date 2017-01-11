#include "kalman.h"

#define DEF_KAL_A 	1
#define DEF_KAL_H 	1
#define DEF_KAL_Q 	0.25
#define DEF_KAL_R 	0.7
#define DEF_KAL_CFT 	1000.0

static float kal_filter(struct kalman_t *this, float kal_Z)
{
	if(!this) return 0.0;

	float P_now = this->kal_A * this->kal_P + this->kal_Q;					//Pnow = A*Plast +Q
	float Kg = (P_now*this->kal_H) / ( P_now*this->kal_H + this->kal_R );	//kg = Pnow*H/(Pnow*H+R)
	float X_now = this->kal_A*this->kal_X + \
		Kg*(kal_Z - this->kal_A*this->kal_H*this->kal_X);					//Xn = A*Xlast + Kg(Zx - A*H*Xlast);
	//debug(DEBUG_kalman,"kg=%f\n",Kg);
	/* update P,X */
	this->kal_P = (1-Kg)*P_now;
	this->kal_X = X_now;
	return X_now;
}

void kal_release(struct kalman_t **this)
{
	if(!this || !*this)	return ;
	FREE(*this);
}

struct kalman_t *kalman_init(kalman_t *this,int A,int H,int Q,int R)
{
	kalman_t *pthis = this;
	if(!pthis){
		this = (kalman_t*)malloc(sizeof(kalman_t));
		if(!this) return NULL;
	}
	bzero(this,sizeof(kalman_t));
	this->kal_A 	=   (A == 0)	? DEF_KAL_A : A/DEF_KAL_CFT;
	this->kal_H 	=   (H == 0)	? DEF_KAL_H : H/DEF_KAL_CFT;
	this->kal_Q 	=   (Q == 0)	? DEF_KAL_Q : Q/DEF_KAL_CFT;
	this->kal_R 	=   (R == 0)	? DEF_KAL_R : R/DEF_KAL_CFT;
	this->kal_filter = kal_filter;
	this->kal_release = kal_release;
	debug(DEBUG_kalman,"kalman config:kal_A=%f,kal_H=%f,"\
		"kal_Q=%f,kal_R=%f\n",this->kal_A,this->kal_H,this->kal_Q,this->kal_R);
	return this;
}
