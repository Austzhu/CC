#include "process.h"

int Reset2DoFunctions(void)
{
	return SUCCESS;
}

int time_tick(u8 *package)
{
	assert_param(package,NULL,FAIL);
	/* 开发板设置时间格式
	 * date 051114542016.59  月 日 时 分 年.秒
	 * 把下发的时间格式化成字符串
	 */
	s8 	set_time[32]={0};
	#ifdef Config_EC_6018
		sprintf(set_time,"date %04u%02u%02u%02u%02u.%02u",\
			package[0]+2000,package[1],package[2],package[4],package[5],package[6]);
	#else
		sprintf(set_time,"date %02u%02u%02u%02u%04u.%02u",\
			package[1],package[2],package[4],package[5],package[0]+2000,package[6]);
	#endif
	//debug(DEBUG_chktime,"Time tick cmd:%s\n",set_time);
	system(set_time);
	return SUCCESS;
}

int Query_time(u8 *buf,int bufsize)
{
	assert_param(buf,NULL,FAIL);

	time_t SystemTime;
	struct tm TimeRTC;
	struct tm TimeSystem;

	memset(buf,0,bufsize);
	memset(&TimeRTC,0,sizeof(struct tm));
	memset(&TimeSystem,0,sizeof(struct tm));
	time(&SystemTime);
	TimeSystem = *localtime(&SystemTime);
	int  fd = open("/dev/rtc0",O_RDONLY,0);
	if(fd < 0) return FAIL;
	if(ioctl(fd,RTC_RD_TIME,&TimeRTC)){
		close(fd); 	return FAIL;
	}	close(fd);

	*buf++ = 0x51; *buf++ = 0x12; *buf++ = 0x80;
	*buf++ = 0x10; *buf++ = 0xA2; *buf++ = 0x04;
	/* system time */
	*buf++ = TimeSystem.tm_year%100;
	*buf++ = TimeSystem.tm_mon+1;
	*buf++ = TimeSystem.tm_mday;
	*buf++ = TimeSystem.tm_wday;
	*buf++ = TimeSystem.tm_hour;
	*buf++ = TimeSystem.tm_min;
	*buf++ = TimeSystem.tm_sec;
	/* get rtc time */
	*buf++ = TimeRTC.tm_year%100;
	*buf++ = TimeRTC.tm_mon+1;
	*buf++ = TimeRTC.tm_mday;
	*buf++ = TimeRTC.tm_wday;
	*buf++ = TimeRTC.tm_hour;
	*buf++ = TimeRTC.tm_min;
	*buf++ = TimeRTC.tm_sec;
	return SUCCESS;
}

int CC_Inquire_Version(u8 *buf,int size)
{
	assert_param(buf,NULL,FAIL);
	memset(buf,0,size);
	*buf++ = 0x51; *buf++ = 0x24; *buf++ = 0x80;
	*buf++ = 0x22; *buf++ = 0xA2; *buf++ = 0x05;
	sprintf((char*)buf,"%s",VERSION_NUMBER);
	return SUCCESS;
}
