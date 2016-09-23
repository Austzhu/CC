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
	int status = 0;
	char str[128];
	int Port = 50119;
	if(argc > 2){
		if( !strcmp(argv[1],"-p") ){ Port = atoi(argv[2]); }
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
			sprintf(str,"ssh -i /home/Austzhu/.ssh/id_rsa -N -f -T -R %d:localhost:22 cc_nick@114.55.54.166 -y",Port);
			//printf("Cmd str:%s\n",str);
			//system(str);
			system("ssh -i /home/Austzhu/.ssh/id_rsa -N -f -T -R 50119:localhost:22 cc_nick@114.55.54.166 -y");
		}

		system("rm temp");
		if(status == 0){
			sleep(30);
			system("killall ssh");
			status = 1;
		}
		sleep(10);
	}
	return 0;
}
