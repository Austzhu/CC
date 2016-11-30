/******************************************************************
 ** 文件名:	ether_server.h
 ** Copyright (c) 2012-2014 *********公司技术开发部
 ** 创建人:	Austzhu
 ** 日　期:	2016.11
 ** 修改人:
 ** 日　期:
 ** 描　述:
 ** ERROR_CODE:
 **
 ** 版　本:	V1.0
*******************************************************************/
#include "ether_server.h"

#ifdef Config_TCP_Server

#define Clean_list(head,type,member)  do{\
	type *_pos = list_entry((*head)->member.next, type, member);\
	while( _pos != *head){\
		list_del_entry(&_pos->member);   free(_pos);\
		_pos = list_entry((*head)->member.next, type, member);\
	}	free(*head);  *head = NULL;\
}while(0)

static void *pthread_listen(void *args)
{
	assert_param(args,NULL);

	server_t *this =  (server_t*)args;
	pthread_detach(pthread_self());
	client_t *temp = NULL;
	socklen_t clientlen = sizeof(struct sockaddr_in);
	while(this->thread_on){
		/* if client count greater then client max sleep to wait */
		if(this->client_count >= Config_client_max) {
			sleep(1);  continue;
		}
		temp = malloc(sizeof(client_t));
		if(!temp) continue;
		bzero(temp,sizeof(client_t));
		INIT_LIST_HEAD(&temp->entries);

		temp->socket_fd = accept(this->server_fd, (struct sockaddr*)&temp->client_addr, &clientlen);
		if(temp->socket_fd <= 0){
			perror("accept");
			FREE(temp);
			continue;
		}
		/* add to list */
		pthread_mutex_lock(&this->client_lock);              //get the task lock
		list_add_tail(&temp->entries,&this->client_header->entries);
		++this->client_count;
		pthread_mutex_unlock(&this->client_lock);          //relese the task lock
		printf("new client ip:%s:%d connected,cilent count:%d\n",\
			inet_ntoa(temp->client_addr.sin_addr),htons(temp->client_addr.sin_port),this->client_count);
		/* add client ip to sqlite */
		temp = NULL;
	}
	#ifdef  Config_exitMessage
		printf("thread listen exit!\n");
	#endif
	return NULL;
}

static void *pthread_status(void *args)
{
	assert_param(args,NULL);

	server_t *this =  (server_t*)args;
	pthread_detach(this->thread_status);
	int info_len = sizeof(struct tcp_info);
	struct tcp_info info;
	client_t *client = NULL;
	while(this->thread_on){
		list_for_each_entry(client,&this->client_header->entries,entries){
			bzero(&info,sizeof(info));
			/* get tcp info */
			getsockopt(client->socket_fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&info_len);
			if(info.tcpi_state != TCP_ESTABLISHED){                   //tcp client is closed
				pthread_mutex_lock(&this->client_lock);           //get the task lock
				list_del_entry(&client->entries);
				--this->client_count;
				pthread_mutex_unlock(&this->client_lock);      //relese the task lock
				printf("new client ip:%s:%d disconnect,client count:%d\n",inet_ntoa(\
					client->client_addr.sin_addr),htons(client->client_addr.sin_port),this->client_count);
				FREE(client);
				break;
			}
		}
		sleep(1);
	}
	#ifdef  Config_exitMessage
		printf("pthreade status exit!\n");
	#endif
	return NULL;
}

static int ser_send(struct server_t *this,client_t *client,void *buffer,int size)
{
	if(!this || !client || !buffer) return FAIL;

	int res = -1;
	res = send(client->socket_fd,buffer,size,MSG_NOSIGNAL);
	if(res < 0){
		perror("send");
		return FAIL;
	}
	return res==size ? SUCCESS : res;

}

static int ser_recv(struct server_t *this,client_t *client,void *buffer,int size, int timeout)
{
	if(!this || !client || !buffer) return FAIL;

	int res = -1;
	timeout *= 1000;
	struct timeval timeo;
	timeo.tv_sec   = timeout/1000000;
	timeo.tv_usec = timeout%1000000;
	setsockopt(client->socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
	res = recv(client->socket_fd,buffer,size,MSG_NOSIGNAL);
	return res==size ? SUCCESS : (res>0 ? RECV_NOTEN:res);
}


static int ser_keepalive(int fd)
{
	keepalive_t keepalive;
	bzero(&keepalive,sizeof(keepalive_t));

	keepalive.keepalive_on = 1;
	keepalive.keepalive_idle = Config_keepidle;
	keepalive.keepalive_inteval = Config_keepinterval;
	keepalive.keepalive_count = Config_keepcount;

	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,&keepalive.keepalive_on,sizeof(int));
	setsockopt(fd, SOL_TCP, TCP_KEEPIDLE,&keepalive.keepalive_idle,sizeof(int));
	setsockopt(fd, SOL_TCP, TCP_KEEPINTVL,&keepalive.keepalive_inteval,sizeof(int));
	setsockopt(fd, SOL_TCP, TCP_KEEPCNT,&keepalive.keepalive_count,sizeof(int));

	return SUCCESS;
}


static int ser_start(struct server_t*this,u16 port)
{
	if(!this) return FAIL;
	printf("start ether server!\n");
	this->server_fd = socket(AF_INET, SOCK_STREAM,0);
	if(this->server_fd < 0){
		perror("socket");
		return FAIL;
	}
	this->server_addr.sin_family = AF_INET;
	this->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->server_addr.sin_port = htons(port);

	/* set keepalive param */
	ser_keepalive(this->server_fd);
	/* 设置地址重用 */
	int reuse_addr = Config_ReuseAddr;
	setsockopt(this->server_fd,SOL_SOCKET,SO_REUSEADDR,&reuse_addr,sizeof(int));
	/* bind addr */
	if( bind(this->server_fd, (struct sockaddr*)&this->server_addr, sizeof(this->server_addr)) < 0){
		perror("bind");
		return FAIL;
	}
	/* listen ether */
	if(listen(this->server_fd,Config_client_max) < 0){
		perror("listen");
		return FAIL;
	}
	this->thread_on = 1;
	pthread_create(&this->thread_listen,NULL,pthread_listen,this);
	pthread_create(&this->thread_status,NULL,pthread_status,this);
	if(this->thread_listen < 0 || this->thread_status < 0){
		err_Print(DEBUG_server,"pthread_create error!\n");
		return FAIL;
	}
	return SUCCESS;
}

static void ser_flush(int socketfd)
{
	char buffer[128] ={0};
	int kbuf_len = 0;
	do{
		recv(socketfd, buffer,sizeof(buffer),MSG_DONTWAIT);
		ioctl(socketfd,FIONREAD,&kbuf_len);
	}while(kbuf_len >0);
}

static int ser_stop(struct server_t*this)
{
	if(!this) return FAIL;
	this->thread_on = 0; 				//stop pthread_status and pthread_listen
	sleep(1);
	if(this->server_fd > 0){
		shutdown(this->server_fd,SHUT_RDWR);
		close(this->server_fd);
		this->server_fd = -1;
	}
	client_t *client = list_entry(this->client_header->entries.next, client_t, entries);

	while( client != this->client_header ){

		if(client->socket_fd > 0) {
			close(client->socket_fd);
			client->socket_fd = -1;
		}
		list_del_entry(&client->entries);
		FREE(client);
		client = list_entry(this->client_header->entries.next, client_t, entries);
	}

	return SUCCESS;
}

static void ser_release(server_t **this)
{
	if(!this || !*this) return ;
	(*this)->ser_stop(*this);
	FREE((*this)->client_header);
	FREE(*this);
}

 server_t *ser_init(server_t *ser)
{
	server_t *const temp = ser;
	if(!temp){
		ser = malloc(sizeof(server_t));
		if(!ser) return NULL;
	}	bzero(ser,sizeof(server_t));

	pthread_mutex_init(&ser->client_lock,NULL);
	ser->client_header = malloc(sizeof(client_t));
	if(!ser->client_header) {
		if(!temp)FREE(ser);
		return NULL;
	}
	bzero(ser->client_header,sizeof(client_t));
	INIT_LIST_HEAD(&ser->client_header->entries);
	ser->thread_on = 0;
	/* init server api */
	ser->ser_release = ser_release;
	ser->ser_send = ser_send;
	ser->ser_recv = ser_recv;
	ser->ser_start = ser_start;
	ser->ser_stop = ser_stop;
	ser->ser_flush = ser_flush;
	return ser;
}

#endif  //end of #ifdef Config_TCP_Server
