#include "Mstring.h"

char Mtoupper(char *ch)
{
	if(*ch >= 'a' && *ch <= 'z'){
		*ch -= 'a'-'A';
	}
	return *ch;
}
/**********************************************************
 * 	parameter(s): [OUT] pbDest - 输出缓冲区
 *	[IN] pbSrc: 	字符串
 *	[IN] nLen: 	16进制数的字节数(字符串的长度/2)
 * 	return :	 	返回16进制的头指针
 * 	remarks : 	将字符串转化为16进制数
***********************************************************/

BYTE* StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen)
{
	char h1,h2;
	BYTE s1,s2;
	int i;
	for (i=0; i<nLen; i++){
		h1 = pbSrc[2*i];
		h2 = pbSrc[2*i+1];
		s1 = Mtoupper(&h1) - 0x30;
		if (s1 > 9) 
			s1 -= 7;

		s2 = Mtoupper(&h2) - 0x30;
		if (s2 > 9) 
			s2 -= 7;

		pbDest[i] = s1*16 + s2;
	}
	return pbDest;
}

int *StrToHex_v2(int *pbDest, BYTE *pbSrc)
{
	int i=0;
	int *dest = pbDest;
	char temp = 0;
	*pbDest = 0;

	while(pbSrc[i]){
		
		if( (temp = Mtoupper((char*)&pbSrc[i++]) - '0') >9){temp -= 7;}
		*pbDest = *pbDest*16 + temp;	
		if(i%8 == 0){
			*++pbDest = 0;
		}
	}
	return dest;
	
}

/**********************************************************
 * 	parameter(s): [OUT] pbDest - 存放目标字符串
 *	[IN] pbSrc: 	输入16进制数的起始地址
 *	[IN] nLen: 	16进制数的字节数
 * 	return :	 	返回字符串的头指针
 * 	remarks : 	将16进制数转化为字符串
***********************************************************/
 #if 1
BYTE* HexToStr_v3(BYTE *pbDest, BYTE *pbSrc, int nLen)
{
	char	dl,dh;
	int i;
	for (i=0; i<nLen; i++){
		dh = 48 + pbSrc[i] / 16;
		dl = 48 + pbSrc[i] % 16;
		if (dh > 57) 		dh +=  7;
		if (dl > 57) 		dl  += 7;

		pbDest[i*2] = dh;
		pbDest[i*2+1] = dl;
	}
	pbDest[nLen*2] = '\0';
	return pbDest;
}
#endif

char *HexTostr(unsigned int Hex,char *dest)
{
	int i=0;
	int temp = 0;
	do{
		if( (temp = Hex%16 + '0') > '9'){
			temp += 7;
		}
		dest[i++] = temp;
	}while(Hex /= 16);
	dest[i] = 0;
	reverse(dest);

	return dest;
}
char *HexTostr_V2(unsigned int hex, char *dest)
{
	char table[] = "0123456789abcdef";
	char *pdest = dest;
	while(hex){
		*dest++ = table[hex&0x0f];
		hex >>= 4;
	}*dest = '\0';
	reverse(pdest);	//逆序字符串
	return pdest;
}

int Mstrlen(const char *s,char EOB)
{
	const char *p = s;
	while(*p++ != EOB);
	return p-s-1;
}

void reverse(char *str)
{
	char tempc;
	char *Start = str;
	char *End 	= str+Mstrlen(str,'\0')-1;
	while(Start < End){
		tempc = *End;
		*End  = *Start;
		*Start= tempc;
		--End,++Start;
	}
}

int Matoi(const char *s)
{
	int sig = 1;
	int n   = 0;
	while(*s == ' ' || *s == '-'){
		if(*s++ == '-'){sig = -1;}
	}
	while(*s >= '0' && *s <= '9'){
		n = n*10 + *s++ - '0';
	}
	return n*sig;
}

char *Mitoa(int n,char *s)
{
	int sig;
	char *ps = s;
	sig = n<0?n=-n,-1:1;
	do{
		*s++ = n%10 + '0';
	}while(n /=10);
	if(sig<0){*s++ = '-';}
	*s = 0;
	reverse(ps);
	return ps;
}

char *IntToStr(int n,char*s)
{
	int sig;
	char *ps = s;
	sig = n<0?n=-n,-1:1;
	char table[] = "0123456789";
	while(n){
		*s++ = table[n%10];
		n /= 10;
	}
	if(sig<0){*s++ = '-';}
	*s = '\0';
	reverse(ps);
	return ps;	
}

char *Mstrcpy(char*dest,const char *src,const char EOB)
{
	char *Pdest = dest;
	while((*dest++ = *src++) != EOB);
	*--dest = '\0';
	return Pdest;
}

char *FloatToStr(double dd,char *dest)
{
	int sig = dd<0?dd*=-1,-1:1;
	char *pdest = (char*)0;
	int integer = (int)dd;					//整数部分
	int deci = (dd-integer)*1000000.0 + 0.1;//小数部分
	Mitoa(integer*sig,dest);		//整数转换
	pdest = dest + Mstrlen(dest,'\0');
	*pdest++ = '.';
	while(! (deci%10)){	//去除小数后面多余的0
		deci /=10;
	}
	Mitoa(deci,pdest);
	return dest;

}
void Mmemset(char *dest,char c,int len)
{
	while(len--){
		*dest++ = c;
	}
}
