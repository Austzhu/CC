/********************************************************************
	> File Name:	log.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月09日 星期一 20时49分59秒
 *******************************************************************/
#include "log.h"
#include "coordinate.h"
/* Title */
#define Title_SINGLE_light 		"Unicast 0x01"			//单播命令
#define Title_SINGLE_group		"Multicast 0x02"		//组播命令
#define Title_SINGLE_broadcast	"Broadcast 0x03"		//广播命令
#define Title_SINGLE_config		"single_config 0x14"		//单灯配置
#define Title_SINGLE_down		"single_down 0x96"		//下发终端分组
#define Title_CC_LogonOrHeartbeat 	"logon/heartbeat 0xA1"	//心跳或登入
#define Title_Server_ctrl_CC		"Server Control CC 0xA2"	//服务器控制集中器操作
#define Title_CC_Param		"CC Parameter Setting 0xA3"	//集中器参数设置
#define Title_CC_Update 		"CC Update 0XA4"		//集中器升级
#define Title_Timed_ctrl 		"Timed Control 0XC1"		//定时任务
#define Title_Latitude_ctrl		"Latitude Control 0XC2"	//经纬度控制
#define Title_Light_ctrl 		"Light Control 0XC3"		//光控
#define Title_DIDO 			"DIDO 0XE1"			//DIDO
#define Title_Alarm			"Alarm 0XF0"			//报警
#define Title_Acquisition		"Acquisition Unit 0xF6"	//采集单元
#define Title_Security			"Security Modules 0XF9"	//防盗模块
#define Title_Reply			"Response "			//应答


static int Log_fd[LogCount];

static s32 Open_log(Logfile_t logf)
{
	if(access("./Logfile",F_OK)){
		system("mkdir Logfile");
	}
	switch(logf){
		case CC:
			if( (Log_fd[logf] = open("./Logfile/Server2CC.log", O_RDWR|O_CREAT|O_APPEND)) <0 ){
				Log_fd[logf] = -1;
				debug(DEBUG_Log,"Open Server2CC.log fail!\n");
				return FAIL;
			}break;
		case Coordinate:
			if( (Log_fd[logf] = open("./Logfile/CC2Single.log", O_RDWR|O_CREAT|O_APPEND)) <0 ){
				Log_fd[logf] = -1;
				debug(DEBUG_Log,"Open CC2Single.log fail!\n");
				return FAIL;
			}break;
		case err:
		case warring:
			if( (Log_fd[logf] = open("./Logfile/err.log", O_RDWR|O_CREAT|O_APPEND)) <0 ){
				Log_fd[logf] = -1;
				debug(DEBUG_Log,"Open err.log fail!\n");
				return FAIL;
			}break;
		default:break;
	}
	return SUCCESS;
}

static void Close_log(Logfile_t logf)
{
	close(Log_fd[logf]);
}

static s32 MakeServerLog(s8 *buf, faalpkt_t *pkg)
{
	u32 i=0;
	switch(pkg->ctrl){
		case 0X01:sprintf(buf,"%s\nData:",Title_SINGLE_light);	break;
		case 0X02:sprintf(buf,"%s\nData:",Title_SINGLE_group);	break;
		case 0X03:sprintf(buf,"%s\nData:",Title_SINGLE_broadcast);break;
		case 0X14:sprintf(buf,"%s\nData:",Title_SINGLE_config);	break;
		case 0X96:sprintf(buf,"%s\nData:",Title_SINGLE_down);	break;
		case 0XA1:sprintf(buf,"%s\nData:",Title_CC_LogonOrHeartbeat);break;
		case 0XA2:sprintf(buf,"%s\nData:",Title_Server_ctrl_CC);	break;
		case 0XA3:sprintf(buf,"%s\nData:",Title_CC_Param);	break;
		case 0XA4:sprintf(buf,"%s\nData:",Title_CC_Update);	break;
		case 0XC1:sprintf(buf,"%s\nData:",Title_Timed_ctrl);	break;
		case 0XC2:sprintf(buf,"%s\nData:",Title_Latitude_ctrl);	break;
		case 0XC3:sprintf(buf,"%s\nData:",Title_Light_ctrl);	break;
		case 0XE1:sprintf(buf,"%s\nData:",Title_DIDO);		break;
		case 0XF0:sprintf(buf,"%s\nData:",Title_Alarm);		break;
		case 0XF6:sprintf(buf,"%s\nData:",Title_Acquisition);	break;
		case 0XF9:sprintf(buf,"%s\nData:",Title_Security);	break;
		case 0X80: if(0xA1 != pkg->data[0]){
				sprintf(buf,"%s 0X%02X 0X%02X\nData:",Title_Reply,pkg->data[0],pkg->data[1]);
				break;
			     }return FAIL;
		default:sprintf(buf,"Unknow Title\nData:");		break;
	}
	for(i=0;i<12+(pkg->len);++i){
		sprintf(buf+strlen(buf),"%02X ",((s8*)pkg)[i]);
	}sprintf(buf+strlen(buf),"\n");

	return SUCCESS;
}
static s32 MakeSingleLog(s8 *buf,  Pag_Single*pkg)
{
	u32 i=0;
	u32 Cmd = 0,itpos =0;
	if(0x81 == pkg->Ctrl){
		if(pkg->Data[0] !=0){
			sprintf(buf,"%s 0X%02X%02X  Exec Fail!\nData:",Title_Reply,pkg->Cmd[0],pkg->Cmd[1]);
		}else{
			sprintf(buf,"%s 0X%02X%02X  Success!\nData:",Title_Reply,pkg->Cmd[0],pkg->Cmd[1]);
		}
		goto data;
	}
	/* 判断是否为广播或组播 */
	if(0x00 != pkg->Coordi_Addr && 0 != (pkg->Single_Addr[0] <<8 |pkg->Single_Addr[1]) ){
		Cmd = pkg->Cmd[0] <<8 | pkg->Cmd[1];
		while((itpos=0x01<<i++) != 0x10000){		//循环检测cmd每个bit位
			switch(Cmd & itpos){
				case 0x00:break;
				case 0x01:sprintf(buf,"Open Light || ");	break;
				case 0x02:sprintf(buf,"Close Light || ");	break;
				case 0x04:sprintf(buf,"Run stat || ");		break;
				case 0x08:sprintf(buf,"Light Level || ");	break;
				case 0x10:sprintf(buf,"Get Voltage || ");	break;
				case 0x20:sprintf(buf,"Get Electricity || ");	break;
				case 0x40:sprintf(buf,"Get Power || ");		break;
				case 0x80:sprintf(buf,"Others || ");		break;
				default:break;
			}
		}
		if(strlen(buf)>3){
			sprintf(buf+strlen(buf) - 3,"\nData:");
		}else{
			sprintf(buf,"Cmd=%04X\nData:",Cmd);
		}

	}else{
		if(0x00 == pkg->Coordi_Addr){
			sprintf(buf,"Broadcast\nData:");
		}else{
			sprintf(buf,"Multicast\nData:");
		}
	}
data:
	for(i=0;i<sizeof(Pag_Single);++i){
		sprintf(buf+strlen(buf),"%02X ",((s8*)pkg)[i]);
	}sprintf(buf+strlen(buf),"\n");

	return SUCCESS;
}
s32 Write_log(int cmd, ...)
{
	s8 logbuf[400]={0};
	memset(logbuf,0,sizeof(logbuf));
	struct tm *tm;
	time_t t;
	faalpkt_t 	*pkg_CC    = NULL;
	Pag_Single 	*pkg_Coor = NULL;
	va_list 	arg_ptr;
	va_start(arg_ptr, cmd);
	tm = localtime( (time(&t),&t) );
	sprintf(logbuf,"\n%04d-%02d-%02d %02d:%02d:%02d> ",tm->tm_year+1900,tm->tm_mon+1,
					tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
	switch(cmd&0xff){
		case CC:
			if(cmd&Log_err){	//写错误消息
				sprintf(logbuf+strlen(logbuf),"Error\nError Message: %s\n",va_arg(arg_ptr, char*));
				va_end(arg_ptr);
				/* 写入日志文件中 */
				Open_log(CC);
				write(Log_fd[CC],logbuf,strlen(logbuf));
				Close_log(CC);
				break;
			}
			/* 获取参数 */
			pkg_CC = va_arg(arg_ptr, faalpkt_t*);
			va_end(arg_ptr);
			/* 准备日志 */
			if(SUCCESS == MakeServerLog(logbuf+strlen(logbuf),  pkg_CC) ){
				/* 写入日志文件中 */
				Open_log(CC);
				write(Log_fd[CC],logbuf,strlen(logbuf));
				Close_log(CC);
			}break;
		case Coordinate:
			if(cmd&Log_err){
				sprintf(logbuf+strlen(logbuf),"Error\nError Message: %s\n",va_arg(arg_ptr, char*));
				va_end(arg_ptr);
				/* 写入日志文件中 */
				Open_log(Coordinate);
				write(Log_fd[Coordinate],logbuf,strlen(logbuf));
				Close_log(Coordinate);
				break;
			}
			pkg_Coor = va_arg(arg_ptr, Pag_Single*);
			va_end(arg_ptr);
			if(SUCCESS == MakeSingleLog(logbuf+strlen(logbuf),pkg_Coor) ){
				Open_log(Coordinate);
				write(Log_fd[Coordinate],logbuf,strlen(logbuf));
				Close_log(Coordinate);
			}break;
		case err:
			sprintf(logbuf+strlen(logbuf),"Error Message: %s\n",va_arg(arg_ptr, char*));
			va_end(arg_ptr);
			Open_log(err);
			write(Log_fd[err],logbuf,strlen(logbuf));
			Close_log(err);
			break;
		case warring:
			sprintf(logbuf+strlen(logbuf),"Warring Message: %s\n",va_arg(arg_ptr, char*));
			va_end(arg_ptr);
			Open_log(warring);
			write(Log_fd[warring],logbuf,strlen(logbuf));
			Close_log(warring);
			break;
		default:debug(DEBUG_Log,"In %s Can't  Identify Cmd!\n",__func__);
	}
	return SUCCESS;
}
