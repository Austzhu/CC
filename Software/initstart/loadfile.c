/********************************************************************
	> File Name:	loadfile.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月20日 星期五 12时45分10秒
 *******************************************************************/
#include "loadfile.h"
#include "Interface.h"

static char *Mstrcpy(char*dest,const char *src,const char EOB)
{
	if(!dest || !src ) return NULL;
	char *Pdest = dest;
	while((*dest++ = *src++) != EOB);
	*--dest = '\0';
	return Pdest;
}

static u8* StrToHex(u8 *pbDest, u8 *pbSrc, int nLen)
{
	u8 s1,s2;
	for (int i=0; i<nLen; i++){
		s1 = toupper(pbSrc[2*i]) - '0';
		s2 = toupper(pbSrc[2*i+1])-'0';
		s1 -= s1>9 ? 7:0;
		s2 -= s2>9 ? 7:0;
		pbDest[i] = (s1<<4) | s2;
	}
	return pbDest;
}


/**
 * 从文件中获取字符串形式的参数
 */
static s32 GetStringParam(s8*filebuf,const char *Column,char *desk)
{
	assert_param(filebuf,NULL,FAIL);
	assert_param(Column,NULL,FAIL);
	assert_param(desk,NULL,FAIL);

	char *res = strstr(filebuf,Column);
	if(res){
		Mstrcpy(desk,res+strlen(Column)+1,'\n');
		return SUCCESS;
	}	return FAIL;
}

/*
 * 从文件中获取int型的参数
 */
static u32 GetIntParam(s8*filebuf,const char *Column)
{
	assert_param(filebuf,NULL,ERRORS);
	assert_param(Column,NULL,ERRORS);

	char *res = strstr(filebuf,Column);
	if(res){
		return atoi(res+strlen(Column)+1);
	}	return ERRORS;
}

/*
 * 从文件在获取16进制的参数，
 * 不过两个连续的字符转化为一个16进制数
 */
static s32 GetHexParam(s8 *filebuf, const s8 *Column, u8 *dest, s32 nL)
{
	assert_param(filebuf,NULL,FAIL);
	assert_param(Column,NULL,FAIL);
	assert_param(dest,NULL,FAIL);

	char *res = NULL;
	res = strstr(filebuf,Column);
	if(!res){  return FAIL;  }

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
static s32 AssignmentParam(s8 *filebuf ,void *app)
{
	assert_param(filebuf,NULL,FAIL);
	assert_param(app,NULL,FAIL);
	appitf_t *pa = app;
	/* CCUID */
	if( SUCCESS != GetHexParam(filebuf,"CCUID",pa->param.CCUID, sizeof(pa->param.CCUID)) )
		StrToHex(pa->param.CCUID,(u8*)Default_CCUID, sizeof(pa->param.CCUID));
	/* ServerIp */
	if( SUCCESS != GetStringParam(filebuf,"ServerIpaddr",pa->param.ServerIpaddr))
		strcpy(pa->param.ServerIpaddr,Default_ServerIp);
	/* Port */
	if( (pa->param.ServerPort =  (short)GetIntParam(filebuf,"ServerPort")) == ERRORS)
		pa->param.ServerPort = Default_ServerPort;
	/* DebugLevel */
	if( (pa->param.DebugLevel = GetIntParam(filebuf,"DebugLevel")) == ERRORS)
		pa->param.DebugLevel = Default_debuglevel;
	/* ControlMethod */
	if( ERRORS == (pa->param.ControlMethod = GetIntParam(filebuf,"ControlMethod")) )
		pa->param.ControlMethod = Default_Method;
	/* ItfWay */
	if( (pa->param.ItfWay = GetIntParam(filebuf,"ConnectType")) == ERRORS)
		pa->param.ItfWay = Default_ItfWay;
	/* HeartBCycle */
	if( (pa->param.HeartBCycle = GetIntParam(filebuf,"HeartBeatcycle")) == ERRORS)
		pa->param.HeartBCycle = Default_HeartBCycle;
	/* tcp/udp */
	if((pa->param.Is_TCP = GetIntParam(filebuf,"Connection")) == ERRORS )
		pa->param.Is_TCP = Default_TCP;

	debug(DEBUG_loadfile,"CCUID:");
	for(int i=0;i<6;++i) debug(DEBUG_loadfile,"%02X ",pa->param.CCUID[i]);
	debug(DEBUG_loadfile,"\n");
	debug(DEBUG_loadfile,"IP:%s:%d\n",pa->param.ServerIpaddr,pa->param.ServerPort);
	debug(DEBUG_loadfile,"DebugLevel=%d,ControlMethod=%d,ItfWay=%d,"\
		"HeartBeatcycle=%d,Connection:%s\n",pa->param.DebugLevel,pa->param.ControlMethod,\
		pa->param.ItfWay,pa->param.HeartBCycle,pa->param.Is_TCP?"TCP":"UDP");
	return SUCCESS;
}


s32 loadParam(void *app)
{
	FILE *fp = NULL;
	s8 *filebuf = NULL;
	u32 i = 0;
	assert_param(app,NULL,FAIL);
	/* 去除空行和带#号的行 且删除字段里的的空格 */
	system("grep -v '^$' " FILE_PARAM " | grep -v '^#'  | sed 's/[[:space:]]//g' > temp");

	if(fp = fopen("./temp","r") , !fp){
		debug(DEBUG_loadfile,"%s,%d:open file fail!\n",__func__,__LINE__);
		//Write_log(err,"Open Config file error!");
		return FAIL;
	}
	/* 准备好文件大小的缓存区 */
	fseek(fp,0,SEEK_END);
	debug(DEBUG_loadfile,"File size: %d\n",i=ftell(fp));
	if( filebuf = malloc(i+1), !filebuf){
		debug(DEBUG_loadfile,"%s,%d:malloc err!\n",__func__,__LINE__);
		//Write_log(err,"malloc for config file buffer fial!");
		 return FAIL;
	}
	memset(filebuf,0,i+1);
 	fseek(fp,0,SEEK_SET);
 	/* 把文件加载到filebuf中 */
 	i=0; while( !feof(fp) ){filebuf[i++] = fgetc(fp);}
 	fclose(fp);
 	system("rm -f ./temp");
 	if(SUCCESS != AssignmentParam(filebuf,app)){
 		free(filebuf);
 		//Write_log(err,"get Config error!");
 		return FAIL;
 	} free(filebuf);
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
#endif
