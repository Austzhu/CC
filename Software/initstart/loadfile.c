/********************************************************************
	> File Name:	loadfile.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月20日 星期五 12时45分10秒
 *******************************************************************/
#include "loadfile.h"
#include "link_method.h"
//GlobalCCparam1 CCparam;
GlobalCCparam CCparamGlobalInfor;
/**
 * 拷贝字符串，遇到 \0 或 \n结束
 */
#if 0
static s32 Mstrcpy(char*des,const char* src)
{
	if(!des || !src) {return FAIL;}
	while(*src  != '\0' && *src != '\n'){
		*des++ = *src++;
	}
	*des = 0;
	return SUCCESS;
}
#endif
/**
 * 从文件中获取字符串形式的参数
 */
static s32 GetStringParam(s8*filebuf,const char *Column,char *desk)
{
	char *res = strstr(filebuf,Column);
	if(res){
		Mstrcpy(desk,res+strlen(Column)+1,'\n');
		return SUCCESS;
	}
	return FAIL;
}
/*
 * 从文件中获取int型的参数
 */
static u32 GetIntParam(s8*filebuf,const char *Column)
{
	char *res = strstr(filebuf,Column);
	if(res){
		return atoi(res+strlen(Column)+1);
	}
	return ERRORS;
}

/*
 * 把字符串转化为16进制的数，两个字符转一个16进制
 */
#if 0
static s32 StrToHex(u8 *pbDest, char *pbSrc, int nLen)
{
	char src_H,src_L;
	char des_H,des_L;
	int i;

	if(nLen%2){return FAIL;}

	for (i=0; i<nLen; ++i){
		src_H = pbSrc[2*i];
		src_L = pbSrc[2*i+1];

		des_H = toupper(src_H) - '0';
		if (des_H > 9) {des_H -= 7;}

		des_L = toupper(src_L) - '0';
		if (des_L > 9) {des_L -= 7;}

		pbDest[i] = des_H*16 + des_L;
	}
	return SUCCESS;
}
#endif

/*
 * 从文件在获取16进制的参数，
 * 不过两个连续的字符转化为一个16进制数
 */
static s32 GetHexParam(s8 *filebuf, const s8 *Column, u8 *dest, s32 nL)
{
	char *res = NULL;
	res = strstr(filebuf,Column);
	if(!res){return -1;}
	memset(dest,0,nL);
	char dsrc[32] ={0};
	memset(dsrc,0,sizeof(dsrc));
	dsrc[0]='0';
	Mstrcpy(dsrc+1,res+strlen(Column)+1,'\n');

	/* 计算要转化为16进制数的字符串个数 */
	int srclen = 0;
	char *p = res+strlen(Column)+1;
	while(*p && *p++ != '\n'){++srclen;}

	/* 如果字符串的个数为奇数，则在前面添加个‘0’ */
	if(srclen%2){
		StrToHex(dest+nL-(srclen+1)/2, (u8*)dsrc, (srclen+1)/2);
	}else{
		StrToHex(dest+nL-(srclen+1)/2, (u8*)dsrc+1, (srclen+1)/2);
	}

	return SUCCESS;
}

/*
 * 初始化全局配置参数
 */
static s32 AssignmentParam(s8 *filebuf)
{
	if(!filebuf){return FAIL;}
	memset( &CCparamGlobalInfor, 0, sizeof(CCparamGlobalInfor) );
	/* CCUID */
	if( SUCCESS != GetHexParam(filebuf,"CCUID",CCparamGlobalInfor.CCUID, sizeof(CCparamGlobalInfor.CCUID)) ){
		StrToHex(CCparamGlobalInfor.CCUID,(u8*)Default_CCUID, 6);
		debug(DEBUG_loadfile,"Get CCUID Failed, Use Default CCUID!\n");
	}
	debug(DEBUG_loadfile,"CCUID:%02X %02X %02X %02X %02X %02X\n",CCparamGlobalInfor.CCUID[0],CCparamGlobalInfor.CCUID[1],
					CCparamGlobalInfor.CCUID[2],CCparamGlobalInfor.CCUID[3],CCparamGlobalInfor.CCUID[4],CCparamGlobalInfor.CCUID[5]);
	/* ServerIp */
	if( SUCCESS != GetStringParam(filebuf,"ServerIpaddr",CCparamGlobalInfor.ServerIpaddr)){
		strcpy(CCparamGlobalInfor.ServerIpaddr,Default_ServerIp);
		debug(DEBUG_loadfile,"Get ServerIpaddr Failed, Use Default Ip...\n");
	}
	/* Port */
	if( (CCparamGlobalInfor.ServerPort =  (short)GetIntParam(filebuf,"ServerPort")) == ERRORS){
		CCparamGlobalInfor.ServerPort = Default_ServerPort;
		debug(DEBUG_loadfile,"Get ServerPort Failed, Use Default Port...\n");
	}
	debug(DEBUG_loadfile,"IP  %s:%d\n",CCparamGlobalInfor.ServerIpaddr,CCparamGlobalInfor.ServerPort);
	/* DebugLevel */
	if( (CCparamGlobalInfor.DebugLevel = GetIntParam(filebuf,"DebugLevel")) == ERRORS){
		CCparamGlobalInfor.DebugLevel = Default_debuglevel;
		debug(DEBUG_loadfile,"Get DebugLevel Failed, Use Default debug level...\n");
	}
	/* ControlMethod */
	if( ERRORS == (char)(CCparamGlobalInfor.ControlMethod = GetIntParam(filebuf,"ControlMethod")) ){
		CCparamGlobalInfor.ControlMethod = Default_Method;
		debug(DEBUG_loadfile,"Get ControlMethod Failed, Use Default control Method...\n");
	}
	/* ItfWay */
	if( (CCparamGlobalInfor.ItfWay = GetIntParam(filebuf,"ConnectType")) == ERRORS){
		CCparamGlobalInfor.ItfWay = Default_ItfWay;
		debug(DEBUG_loadfile,"Get ItfWay Failed, Use Default itfway...\n");
	}
	/* HeartBCycle */
	if( (CCparamGlobalInfor.HeartBCycle = GetIntParam(filebuf,"HeartBeatcycle")) == ERRORS){
		CCparamGlobalInfor.HeartBCycle = Default_HeartBCycle;
		debug(DEBUG_loadfile,"Get HeartBeatcycle Failed, Use Default HeartBeatcycle...\n");
	}
	/* tcp/udp */
	if((CCparamGlobalInfor.Is_TCP = GetIntParam(filebuf,"Connection")) == ERRORS ){
		CCparamGlobalInfor.Is_TCP = Default_TCP;
		debug(DEBUG_loadfile,"Get Connection TCP or UDP Failed, Use TCP...\n");
	}

	debug(DEBUG_loadfile,"DebugLevel=%d,ControlMethod=%d,ItfWay=%d,HeartBeatcycle=%d,Connection:%s\n",CCparamGlobalInfor.DebugLevel,
		CCparamGlobalInfor.ControlMethod,CCparamGlobalInfor.ItfWay,CCparamGlobalInfor.HeartBCycle,CCparamGlobalInfor.Is_TCP?"TCP":"UDP");

	return SUCCESS;

}

s32 loadParam(void)
{
	FILE *fp = NULL;
	s8 *filebuf = NULL;
	u32 i = 0;
	/* 去除空行和带#号的行 且删除字段里的的空格 */
	system("grep -v '^$' " FILE_PARAM " | grep -v '^#'  | sed 's/[[:space:]]//g' > temp");

	if(fp = fopen("./temp","r") , !fp){
		debug(DEBUG_loadfile,"%s,%d:open file fail!\n",__func__,__LINE__);
		return FAIL;
	}
	/* 准备好文件大小的缓存区 */
	fseek(fp,0,SEEK_END);
	debug(DEBUG_loadfile,"File size: %d\n",i=ftell(fp));
	if( filebuf = malloc(i+1), !filebuf){
		debug(DEBUG_loadfile,"%s,%d:malloc err!\n",__func__,__LINE__);
		 return FAIL;
	}
	memset(filebuf,0,i+1);
 	fseek(fp,0,SEEK_SET);
 	i=0;
 	/* 把文件加载到filebuf中 */
 	while( !feof(fp) ){filebuf[i++] = fgetc(fp);}
 	fclose(fp);
 	system("rm -f ./temp");

 	if(SUCCESS != AssignmentParam(filebuf)){
 		free(filebuf);
 		return FAIL;
 	}
 	free(filebuf);
 	return SUCCESS;
}

s32 Replace_char(char*buf, char c,char rc)
{
	if(!buf){return FAIL;}
	while(*buf){
		if(c == *buf++){ *(buf) = rc; }
	}
	return SUCCESS;
}

#if 0
s32 SetToBuf(char*Pbuf,int Cmd,...)
{
	u32 len = strlen(Pbuf);
	void *Pdata = NULL;
	va_list list;
	va_start(list,Cmd);
	switch(Cmd){
		case CUID:
			Pdata = va_arg(list,void*);
			va_end(list);
			sprintf(Pbuf+len,"%s=%02x %02x %02x %02x %02x %02x\n",CCID,((char*)Pdata)[0],((char*)Pdata)[1],((char*)Pdata)[2],
												((char*)Pdata)[3],((char*)Pdata)[4],((char*)Pdata)[5]);
			break;
		case DUGLEVEL:
			Pdata = va_arg(list,void*);
			va_end(list);
			sprintf(Pbuf+len,"%s=%d\n",Duglev,*(u8*)Pdata);
			break;
		case METHOD:
			break;
		case ITWAY:
			Pdata = va_arg(list,void*);
			va_end(list);
			sprintf(Pbuf+len,"%s=%d\n",ConType,*(u8*)Pdata);
			break;
		case CYCLE:
			Pdata = va_arg(list,void*);
			va_end(list);
			sprintf(Pbuf+len,"%s=%d\n",Beatcycle,*(u8*)Pdata);
			break;
		case IP:
			Pdata = va_arg(list,void*);
			va_end(list);
			sprintf(Pbuf+len,"%s=%s\n",ServerIp,(s8*)Pdata);
			break;
		case PORT:
			Pdata = va_arg(list,void*);
			va_end(list);
			sprintf(Pbuf+len,"%s=%d\n",SPort,*(u16*)Pdata);
			break;
		default:break;
	}
	return SUCCESS;
}
#endif

s32 SaveFile(int cmd, const char *Column, const void *Content)
{
	char cmdline[128] = {0};
	u8 *p = NULL;
	memset(cmdline,0,sizeof(cmdline));
	switch(cmd){
		case _u8_1:sprintf(cmdline,"sed -i 's/%s.*/%s=%d/' %s ",Column,Column,*(u8*)Content,FILE_PARAM);break;
		case _u8_2:sprintf(cmdline,"sed -i 's/%s.*/%s=%d/' %s ",Column,Column,*(u16*)Content,FILE_PARAM);break;
		case _u8_6://CCUID,6个u8的16进制转换成字符
			p = (u8*)Content;
			sprintf(cmdline,"sed -i 's/%s.*/%s=%02x %02x %02x %02x %02x %02x/' %s ",Column,Column,*p,*(p+1),*(p+2),*(p+3),*(p+4),*(p+5),FILE_PARAM);break;
		case _u8_s:sprintf(cmdline,"sed -i 's/%s.*/%s=%s/' %s ",Column,Column,(char*)Content,FILE_PARAM);break;
	}
	//debug(DEBUG_loadfile,"Cmdline:%s\n",cmdline);
	return system(cmdline) == -1?FAIL:SUCCESS;
}
s32 SaveParam(void)
{
	/* 备份原来的配置文件 */
	system("cp " FILE_PARAM " ./config/fileparam.bak -rfd");
	SaveFile(_u8_6,"CCUID",CCparamGlobalInfor.CCUID);
	SaveFile(_u8_1,"DebugLevel",&CCparamGlobalInfor.DebugLevel);
	SaveFile(_u8_1,"ConnectType",&CCparamGlobalInfor.ItfWay);
	SaveFile(_u8_1,"HeartBeatcycle",&CCparamGlobalInfor.HeartBCycle);
	SaveFile(_u8_2,"ServerPort",&CCparamGlobalInfor.ServerPort);
	SaveFile(_u8_s,"ServerIpaddr",CCparamGlobalInfor.ServerIpaddr);
	return SUCCESS;
}
