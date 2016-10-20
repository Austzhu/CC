/********************************************************************
	> File Name:	loadfile.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月20日 星期五 12时45分10秒
 *******************************************************************/
#include "loadfile.h"
#include "Interface.h"
//GlobalCCparam CCparamGlobalInfor;

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
	if( SUCCESS != GetHexParam(filebuf,"CCUID",pa->CCUID, sizeof(pa->CCUID)) ){
		StrToHex(pa->CCUID,(u8*)Default_CCUID, sizeof(pa->CCUID));
		debug(DEBUG_loadfile,"Get CCUID Failed, Use Default CCUID!\n");
		//Write_log(warring,"Load CCUID error!");
	} debug(DEBUG_loadfile,"CCUID:%02X %02X %02X %02X %02X %02X\n",pa->CCUID[0],pa->CCUID[1],pa->CCUID[2],pa->CCUID[3],pa->CCUID[4],pa->CCUID[5]);

	/* ServerIp */
	if( SUCCESS != GetStringParam(filebuf,"ServerIpaddr",pa->ServerIpaddr)){
		strcpy(pa->ServerIpaddr,Default_ServerIp);
		debug(DEBUG_loadfile,"Get ServerIpaddr Failed, Use Default Ip...\n");
		//Write_log(warring,"Load ServerIpaddr error, Use Default Ip...");
	}
	/* Port */
	if( (pa->ServerPort =  (short)GetIntParam(filebuf,"ServerPort")) == ERRORS){
		pa->ServerPort = Default_ServerPort;
		debug(DEBUG_loadfile,"Get ServerPort Failed, Use Default Port...\n");
		//Write_log(warring,"Load ServerPort error, Use Default Port...");
	}	debug(DEBUG_loadfile,"IP  %s:%d\n",pa->ServerIpaddr,pa->ServerPort);

	/* DebugLevel */
	if( (pa->DebugLevel = GetIntParam(filebuf,"DebugLevel")) == ERRORS){
		pa->DebugLevel = Default_debuglevel;
		debug(DEBUG_loadfile,"Get DebugLevel Failed, Use Default debug level...\n");
		//Write_log(warring,"Load DebugLevel Failed, Use Default debug level...");
	}

	/* ControlMethod */
	if( ERRORS == (pa->ControlMethod = GetIntParam(filebuf,"ControlMethod")) ){
		pa->ControlMethod = Default_Method;
		debug(DEBUG_loadfile,"Get ControlMethod Failed, Use Default control Method...\n");
		//Write_log(warring,"Load ControlMethod Failed, Use Default control Method...");
	}

	/* ItfWay */
	if( (pa->ItfWay = GetIntParam(filebuf,"ConnectType")) == ERRORS){
		pa->ItfWay = Default_ItfWay;
		debug(DEBUG_loadfile,"Get ItfWay Failed, Use Default itfway...\n");
		//Write_log(warring,"Load ItfWay Failed, Use Default itfway...");
	}

	/* HeartBCycle */
	if( (pa->HeartBCycle = GetIntParam(filebuf,"HeartBeatcycle")) == ERRORS){
		pa->HeartBCycle = Default_HeartBCycle;
		debug(DEBUG_loadfile,"Get HeartBeatcycle Failed, Use Default HeartBeatcycle...\n");
		//Write_log(warring,"Load HeartBeatcycle Failed, Use Default HeartBeatcycle...");
	}

	/* tcp/udp */
	if((pa->Is_TCP = GetIntParam(filebuf,"Connection")) == ERRORS ){
		pa->Is_TCP = Default_TCP;
		debug(DEBUG_loadfile,"Get Connection TCP or UDP Failed, Use TCP...\n");
		//Write_log(warring,"Load Connection TCP or UDP Failed, Use TCP...");
	}

	debug(DEBUG_loadfile,"DebugLevel=%d,ControlMethod=%d,ItfWay=%d,HeartBeatcycle=%d,Connection:%s\n",\
								pa->DebugLevel,pa->ControlMethod,pa->ItfWay,pa->HeartBCycle,pa->Is_TCP?"TCP":"UDP");
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
