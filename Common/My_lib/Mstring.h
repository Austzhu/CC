#ifndef __Mstring_h__
#define __Mstring_h__
typedef unsigned char 		BYTE;	

/**********************************************************
 * 	parameter(s): [OUT] pbDest - 输出缓冲区
 *	[IN] pbSrc: 	字符串
 *	[IN] nLen: 	16进制数的字节数(字符串的长度/2)
 * 	return :	 	返回16进制的头指针
 * 	remarks : 	将字符串转化为16进制数
***********************************************************/
extern BYTE* StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen);
extern int *StrToHex_v2(int *pbDest, BYTE *pbSrc);

/**********************************************************
 * 	parameter(s): [OUT] pbDest - 存放目标字符串
 *	[IN] pbSrc: 	输入16进制数的起始地址
 *	[IN] nLen: 	16进制数的字节数
 * 	return :	 	返回字符串的头指针
 * 	remarks : 	将16进制数转化为字符串
***********************************************************/
//extern BYTE* HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen);
extern char *HexTostr(unsigned int Hex,char *dest);
extern char *HexTostr_V2(unsigned int Hex,char *dest);
extern BYTE* HexToStr_v3(BYTE *pbDest, BYTE *pbSrc, int nLen);
extern char *IntToStr(int n,char*s);

extern char Mtoupper(char *ch);
extern int Mstrlen(const char *s,char EOB);
extern void reverse(char *str);
extern int Matoi(const char *s);
extern char *Mitoa(int n,char *s);
extern char *Mstrcpy(char*dest,const char *src,const char EOB);
extern char *FloatToStr(double dd,char *dest);
extern void Mmemset(char *dest,char c,int len);
#endif

