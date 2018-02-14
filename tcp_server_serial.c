/*
 * tcp_server_serial.c
 * Последовательное обслуживание
 *  Created on: 28.01.2018
 *      Author: jake
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>

// Перевод сокета в неблокирующий режим
int set_nonblock(int fd)
{
	int flags;
#if defined(O_NONBLOCK)
	if (-1 == (flags = fcntl(fd,F_GETFL,0)))
		flags =0;
	return fcntl(fd,F_SETFL,flags|O_NONBLOCK);
#else
	flags =1;
	return ioctl(fd,FIONBIO,&flags);
#endif
}
//---------------------------------------
int main(void)
{
	int ss = socket(AF_INET,SOCK_STREAM,0);
	int cs;
	int yes=1;
	int count_client=0;
	char buf[BUFSIZ];
	struct sockaddr_in local;
	struct timeval tv;

	//inet_aton("127.0.0.1",&local.sin_addr);
	local.sin_port = htons(1234);
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);//0.0.0.0

	//set_nonblock(ss);

	/* для повторного использования порта*/
	if ( setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
	{
	  perror("setsockopt() error");
	  exit(1);
	}
	/*------------------------------------*/
	/* таймер ожидания для сокета ss на чтение и запись
	tv.tv_sec = 30;
	tv.tv_usec = 0;
	if ( setsockopt(ss, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv)) == -1 )
		{
		  perror("setsockopt(time) error");
		  exit(1);
		}
	if ( setsockopt(ss, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) == -1 )
			{
			  perror("setsockopt(time) error");
			  exit(1);
			}
	/*--------------------------------------------------*/
	if (bind(ss,(struct sockaddr*) &local, sizeof(local)) < 0)
	{
		perror("binding error");
		exit(1);
	}
	listen(ss,SOMAXCONN);

	while(1)
	{
		if ((cs = accept(ss,NULL,NULL)) > 0)
			if (recv(cs,buf,BUFSIZ,MSG_NOSIGNAL) >0)
				// не отправляется SIGPIPE если оборвалось соединение
				{printf("%d\n",++count_client);
				strcat(buf,"->");
				send(cs,buf,strlen(buf)+1,MSG_NOSIGNAL);
				sleep(1);
				shutdown(cs,SHUT_RDWR);
				close(cs);}
	}
	shutdown(ss,SHUT_RDWR);
	close(ss);
	return 0;
}
