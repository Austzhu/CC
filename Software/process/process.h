#ifndef __PROCESS_H__
#define __PROCESS_H__
#include "include.h"

extern int Reset2DoFunctions(void);
extern int time_tick(u8*);
extern int Query_time(u8 *buf,int bufsize);
extern int CC_Inquire_Version(u8 *buf,int size);

#endif
