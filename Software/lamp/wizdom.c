/******************************************************************
** 文件名:	wizdom.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2012.11
** 修改人:
** 日　期:
** 描　述:	16  进制打印
** ERROR_CODE:
**
** 版　本:	V1.0
*******************************************************************/
#include "wizdom.h"

#ifdef Config_wizdom

static int wiz_Query(struct wizdom_t*this,client_t *client,result_t *res)
{
	char buffer[12] = {res->addr,3,0,0,0,2,0};
	buffer[2] = (res->Register>>8)&0xff;
	buffer[3] = (res->Register)&0xff;
	this->crc->CRCLH_get(buffer+6,buffer,6);
	this->wiz_display("Send:",buffer,8);

	/* clear kernel recv buffer */
	this->ether_server->ser_flush(client->socket_fd);
	/* send data */
	this->ether_server->ser_send(this->ether_server,client,buffer,8);
	bzero(buffer,sizeof(buffer));
	if(SUCCESS != this->ether_server->ser_recv(this->ether_server,client,buffer,9,5000)){
		err_Print(1,"Recv error!\n");
		this->wiz_display("Recv:",buffer,9);
		return FAIL;
	}
	this->wiz_display("Recv:",buffer,9);
	if(SUCCESS != this->crc->CRCLH_check(buffer+7,buffer,7)){
		err_Print(1,"CRC check error!\n");
		return FAIL;
	}
	int *p_result = res->result;
	switch(res->Register){
		case 0x0100:break;
		case 0x0200:
			if(res->result_size >= 8){
				*p_result++ = ((buffer[3]<<8)|buffer[4])/100-40;
				*p_result = (buffer[5]<<8)|buffer[6];
				printf("temperature:%d,humidity:%d\n",*(p_result-1),*p_result);
			}	break;
		case 0x0400:break;
		case 0x0600:
			if(res->result_size >= 4){
				*p_result = ((buffer[5]<<8)|buffer[6])/100;
				printf("wind:%d\n",*p_result);
			}	break;
		case 0x0700:break;
		default:break;
	}

	return SUCCESS;
}

static int wiz_getaddr(struct wizdom_t*this)
{
	char buffer[12] = {0xff,6,0,3,0};
	this->crc->CRCLH_get(buffer+6,buffer,6);
	this->wiz_display("Send:",buffer,8);

	client_t *client = list_entry(this->ether_server->client_header->entries.next, client_t, entries);
	if(client == this->ether_server->client_header){
		printf("client list is empty\n");
		return SUCCESS;
	}
	/* clear kernel recv buffer */
	this->ether_server->ser_flush(client->socket_fd);
	this->ether_server->ser_send(this->ether_server,client,buffer,8);
	if(SUCCESS != this->ether_server->ser_recv(this->ether_server,client,buffer,8,5000)){
		err_Print(1,"Recv error!\n");
		this->wiz_display("Recv:",buffer,8);
		return FAIL;
	}
	printf("Get Addr:%02x\n",buffer[0]);
	return SUCCESS;
}

static int wiz_setaddr(struct wizdom_t*this,char addr)
{
	char buffer[12] = {0xff,6,0,2,0,addr,0};
	this->crc->CRCLH_get(buffer+6,buffer,6);
	this->wiz_display("Send:",buffer,8);

	client_t *client = list_entry(this->ether_server->client_header->entries.next, client_t, entries);
	if(client == this->ether_server->client_header){
		printf("client list is empty\n");
		return SUCCESS;
	}
	/* clear kernel recv buffer */
	this->ether_server->ser_flush(client->socket_fd);
	this->ether_server->ser_send(this->ether_server,client,buffer,8);
	if(SUCCESS != this->ether_server->ser_recv(this->ether_server,client,buffer,8,5000)){
		err_Print(1,"Recv error!\n");
		this->wiz_display("Recv:",buffer,8);
		return FAIL;
	}
	if(addr == buffer[0])
		printf("Set Addr 0x%02x success!\n",buffer[0]);
	return SUCCESS;
}

static void wiz_display(const char *Header,const char *Message,int nLen)
{
	printf("%s",Header);
	for(int i=0; i<nLen; ++i){
		printf("%02x ",*Message++);
	}	printf("\n");
}

static void wiz_release(struct wizdom_t**this)
{
	if(!this || !*this) return ;
	DELETE((*this)->ether_server,ser_release);
	DELETE((*this)->crc,CRC_release);
	FREE(*this);
}

wizdom_t *wiz_init(wizdom_t *this)
{
	wizdom_t *const fist = this;
	if(!this){
		this = malloc(sizeof(wizdom_t));
		if(!this) return NULL;
	}	bzero(this,sizeof(wizdom_t));

	this->ether_server = ser_init(NULL);
	if(!this->ether_server){
		if(!fist) FREE(this);
		return NULL;
	}
	this->crc = CRC_init(NULL);
	if(!this->crc){
		DELETE(this->ether_server,ser_release);
		if(!fist) FREE(this);
		return NULL;
	}
 	this->wiz_Query = wiz_Query;
	this->wiz_getaddr = wiz_getaddr;
	this->wiz_setaddr = wiz_setaddr;
	this->wiz_release = wiz_release;
	this->wiz_display = wiz_display;

 	return this;
}

#endif
