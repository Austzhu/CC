/******************************************************************
** 文件名:	operate.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.12
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __COMMUNICATE_H__
#define __COMMUNICATE_H__
#include "include.h"

struct param_t;
typedef struct opt_param_t{
	int 	fd;
	u8 *r_buf;
	int 	r_ptr;
	int 	r_len;
	struct param_t *param;	//全局设置参数
} opt_param_t;

typedef struct operate_t {
	int32_t Point_flag;
	void *opt_Itfway;		//指向具体通讯方式的类
	struct opt_param_t  opt_param;
	int	  (*opt_connect)(struct operate_t*);
	int	  (*opt_logon)(struct operate_t*);
	int 	  (*opt_send)(struct operate_t*,u8 *buffer,int size);
	int	  (*opt_heartbeat)(struct operate_t*);
	int	  (*opt_getchar)(struct operate_t*,u8*);
	int	  (*opt_recv)(struct operate_t*,u8*,int);
	void (*opt_relese)(struct operate_t*);
} operate_t;

extern operate_t *operate_init(struct operate_t*,struct param_t*);

#endif
