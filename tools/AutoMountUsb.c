#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/mount.h>

#define UEVENT_BUFFER_SIZE 2048

static int init_hotplug_sock(void)
{
	const int buffersize = 1024;
	int Sfd = 0;
	struct sockaddr_nl snl;
	memset(&snl,0,sizeof(snl));

	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	if((Sfd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT)) < 0){
		perror("socket!");
		return -1;
	}
	setsockopt(Sfd, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));
	if(bind(Sfd, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl)) < 0){
		perror("bind");
		close(Sfd);
		return -1;
	}
	return Sfd;
}

static const char *find_block(const char *buffer,const char *coulmn)
{
	if(buffer == NULL || coulmn == NULL){
		return NULL;
	}
	const char *Pcoul = NULL;
	while(*buffer != '\0'){
		Pcoul = coulmn;
		while(*buffer == *Pcoul ){
			++buffer;
			++Pcoul;
			if(*Pcoul == '\0' ) {
				if(*(buffer+1) != '\0')
				return buffer+1;
			}
		}
		buffer++;
	}
	return NULL;

}

int main(int argc, char* argv[])
{
	static int Is_Mount = 0;
	int hotplug_sock = init_hotplug_sock();
	char buf[UEVENT_BUFFER_SIZE * 2] ;
	const char *PRET =NULL;
	char shell[128];
	memset(shell,0,sizeof(shell));
	while(1) {
		memset(buf,0,sizeof(buf));
		recv(hotplug_sock, &buf, sizeof(buf), 0);
		PRET = find_block(buf,"block/sd");
		if(PRET){
			if(!Is_Mount){
				sleep(1);
				/* umount系统知道挂载的目录 */
				memset(shell,0,sizeof(shell));
				sprintf(shell,"umount /dev%s ",PRET);
				printf("shell:%s\n", shell);
				system(shell);
				/* 挂载到指定目录 */
				memset(shell,0,sizeof(shell));
				sprintf(shell,"mount  /dev%s /root/usb/ ",PRET);
				printf("shell:%s\n", shell);
				system(shell);
				/* 执行U盘里的脚本 */
				system("/root/usb/Update.sh");
				Is_Mount = 1;
			}else{
				system("umount /root/usb");
				Is_Mount = 0;
			}
		}
	}
	return 0;
}
