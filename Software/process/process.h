/******************************************************************
** 文件名:	porccess.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __PROCESS_H__
#define __PROCESS_H__
#include "include.h"
#include "taskque.h"
#include "Interface.h"

extern int opt_reset(void);
extern int opt_tmtick(uint8_t*);
extern int opt_tmquery(uint8_t *buf,int bufsize);
extern int opt_version(uint8_t *buf,int size);

extern int Config_Single(uint8_t *,appitf_t *);
extern int Config_Coordi(uint8_t *,appitf_t *);
extern int Config_delete(uint8_t *,appitf_t *);
extern int Config_tunnel(uint8_t *,appitf_t *);
extern int Config_task(uint8_t *,appitf_t *);
extern int Config_tasklist(uint8_t *,appitf_t *);

#endif
