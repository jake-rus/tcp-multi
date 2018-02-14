/*
 * tcp_server_serial.c
 * Параллельное обслуживание с предварительным созданием процессов
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

#define MAX_SERVER 5

int main(void)
{
	int ss = socket(AF_INET,SOCK_STREAM,0);
	int cs;
	int yes=1;
	int count_client=0;
	int server;
	pid_t pid_server[MAX_SERVER];
	char buf[BUFSIZ];
	struct sockaddr_in local;
	struct timeval tv;

	//inet_aton("127.0.0.1",&local.sin_addr);
	local.sin_port = htons(1234);
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);//0.0.0.0

	/* для повторного использования порта*/
	if ( setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
	{
	  perror("setsockopt() error");
	  exit(1);
	}
	/*------------------------------------*/
	if (bind(ss,(struct sockaddr*) &local, sizeof(local)) < 0)
	{
		perror("binding error");
		exit(1);
	}
	listen(ss,SOMAXCONN);

	for (server=0;server<MAX_SERVER;server++) {
		if (0!=(pid_server[server]=fork())){
		printf("server %d\n",server);
		while(count_client<1000)
		{
			if ((cs = accept(ss,NULL,NULL)) > 0)
				if (recv(cs,buf,BUFSIZ,MSG_NOSIGNAL) >0)
					// не отправляется SIGPIPE если оборвалось соединение
					{printf("%d\n",++count_client);
					strcat(buf,"->");
					send(cs,buf,strlen(buf)+1,MSG_NOSIGNAL);
					shutdown(cs,SHUT_RDWR);
					close(cs);}
		}
		exit(0);
	} else waitpid(pid_server[server]);}
	shutdown(ss,SHUT_RDWR);
	close(ss);
	return 0;
}
