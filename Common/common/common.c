#include "common.h"
#include "include.h"
char MessageBuffer[MessageBuffersize];


char* Hex2Str(char*dest,const u8 *src,int size)
{
	assert_param(dest,NULL);
	assert_param(src,NULL);

	char *pdest = dest;
	char table[] = {"0123456789ABCDEF"};
	for(int i=0;i<size;++i){
		*pdest++ = table[(*src>>4)&0x0f];
		*pdest++ = table[*src&0x0f];
		++src;
	}
	return dest;
}

u8 *Str2Hex(u8 *dest,const char *src)
{
	assert_param(dest,NULL);
	assert_param(src,NULL);

	int size = strlen(src);
	u8 s1 = 0,s2 = 0;
	u8 *pdest = dest;
	for(int i=0,len=size/2;i<len;++i){
		s1 = toupper(src[2*i]) - '0';
		s2 = toupper(src[2*i+1])-'0';
		s1 -= s1 > 9 ? 7 : 0;
		s2 -= s2 > 9 ? 7 : 0;
		*pdest++ =  (s1<<4) | s2;
	}
	if(size%2){
		s1 = toupper(*(src+size-1)) - '0';
		s1 -= s1 > 9 ? 7 : 0;
		*pdest = s1;
	}
	return dest;
}

void msleep(u32 ms)
{
	struct timeval tv;
	ms *= 1000;
	tv.tv_sec   = ms/1000000;
	tv.tv_usec = ms%1000000;
	select(0, NULL,NULL,NULL,&tv);
}


int get_check_sum(void *pkg,int size)
{
	assert_param(pkg,FAIL);
	u8 chk = 0;
	u8 *pchar = pkg;
	for(int i=0;i<size;++i) chk += *pchar++;
	*pchar++ = chk;
	*pchar = 0x16;
	return SUCCESS;
}
