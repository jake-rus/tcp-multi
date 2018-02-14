/*
 * client_single.c
 *	одиночный клиент с клавиатуры
 *  Created on: 28.01.2018
 *      Author: jake
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h> //для close()
#include <stdlib.h> //для exit()

struct sockaddr_in local;

int main(void)
{
	char buf[BUFSIZ];
	int yes=1;
	int s=socket(AF_INET,SOCK_STREAM,0);

	inet_aton("127.0.0.1",&local.sin_addr);
	local.sin_port = htons(1234);
	local.sin_family = AF_INET;

	/* для повторного использования порта*/
	if ( setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
	{
	  perror("setsockopt() error");
	  exit(1);
	}
	/*------------------------------------*/
	connect(s,(struct sockaddr*)&local,sizeof(local));

	gets(buf); sync();
	send(s,buf,strlen(buf)+1,MSG_NOSIGNAL);
	recv(s,buf,BUFSIZ,MSG_NOSIGNAL);
	printf("%s\n",buf);
	close(s);
	exit(0);
	return 0;
}
