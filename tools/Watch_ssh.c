/********************************************************************
	> File Name:	Waych_ssh.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com
	> Created Time:	2016年09月23日 星期五 09时52分07秒
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
 #include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


int main(int argc, char const *argv[])
{
	int fd = -1;
	int stat = 0;
	char str[128];
	int Port = atoi(getenv("rshPort"));
	if(Port <50000 || Port > 60000){
		Port = 50119;
	}
	while(1){
		system("ps | grep \"ssh\" | grep -v \"grep\"> temp");
		if(0 >  (fd = open("temp",O_RDWR) )){ perror("open"); }
		memset(str,0,sizeof(str));
		read(fd, str, sizeof(str)-1 );
		if(strlen(str)  > 10){
			printf("reverse ssh is  start!\n");
		}else{
			printf("reverse ssh is not start!\n");
			memset(str,0,sizeof(str));
			sprintf(str,"ssh -i /home/Austzhu/.ssh/id_rsa -N -f -T -R %d:localhost:22 %s -y",Port,getenv("rshServer"));
			printf("Cmd str:%s\n",str);
			system(str);
		}
		system("rm temp");
		sleep(30);
		if(!stat){ system("killall ssh");stat = 1; }
	}
	return 0;
}
