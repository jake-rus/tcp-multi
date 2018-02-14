/*
 * tcp_server_pre-thread.c
 * Параллельное обслуживание с предварительным созданием потоков
 *  Created on: 07.02.2018
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
#include <pthread.h>

#define MAX_SERVER 5

typedef struct {
	int server; // порядковые номера серверов
	int ls; // дескриптор слушающего сокета
	int ws[MAX_SERVER]; // дескрипторы рабочих сокетов
} pthread_struct;

pthread_mutex_t mutex_ls; // мьютекс на работу со слушающим сокетом
pthread_mutex_t mutex_serv_count; // мьютекс на работу c номерами серверов

void* thread_server(void*args)// поток-сервер
{
	pthread_struct* serv_struct=(pthread_struct*)args;
	char buf[BUFSIZ];
	int count_client=0;
	int number_server;

	pthread_mutex_lock(&mutex_serv_count);
	if (serv_struct->server < (MAX_SERVER-1))
		number_server= serv_struct->server++;
	else number_server= serv_struct->server;
	pthread_mutex_unlock(&mutex_serv_count);

	while(count_client<100)
		{
		pthread_mutex_lock(&mutex_ls);
		serv_struct->ws[number_server] = accept(serv_struct->ls,NULL,NULL);
		pthread_mutex_unlock(&mutex_ls);
		if (serv_struct->ws[number_server] == -1) {
			perror("no accept");
			pthread_exit(1);
		}
			printf("server-%d: clients-%d\n",number_server,++count_client);
			if (recv(serv_struct->ws[number_server],buf,BUFSIZ,MSG_NOSIGNAL) >0)
					// не отправляется SIGPIPE если оборвалось соединение
				strcat(buf,"->");
				send(serv_struct->ws[number_server],buf,strlen(buf)+1,MSG_NOSIGNAL);
				usleep(200000);
				shutdown(serv_struct->ws[number_server],SHUT_RDWR);
				close(serv_struct->ws[number_server]);

		}

	pthread_exit(0);
}

int main(void)
{
	int yes=1,i;
	pthread_t tid_server[MAX_SERVER];
	pthread_mutex_t mutex; // мьютекс на работу со слушающим сокетом
	pthread_mutex_init(&mutex, NULL);// мьютекс на работу со счетчиком серверов
	struct sockaddr_in listener;
	pthread_struct server_struct; // для передачи потокам
	pthread_struct *arg_serv = &server_struct;
	pthread_mutex_init(&mutex_ls, NULL);
	pthread_mutex_init(&mutex_serv_count, NULL);
/* инициализация слушающего сокета*/
	if ((server_struct.ls = socket(AF_INET,SOCK_STREAM,0)) < 0)
		{
			perror("ls socket error");
			exit(1);
		}
	//inet_aton("127.0.0.1",&listener.sin_addr);
	listener.sin_port = htons(1234);
	listener.sin_family = AF_INET;
	listener.sin_addr.s_addr = htonl(INADDR_ANY);//0.0.0.0

/* для повторного использования порта*/
	if ( setsockopt(server_struct.ls, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
	{
	  perror("setsockopt() error");
	  exit(1);
	}

/* Подключение к порту*/
	if (bind(server_struct.ls,(struct sockaddr*) &listener, sizeof(listener)) < 0)
	{
		perror("binding error");
		exit(1);
	}
	listen(server_struct.ls,SOMAXCONN);
/* Предварительное создание потоков - рабочих сокетов*/
	server_struct.server=0;
	for (i=0;i<MAX_SERVER;i++)
		pthread_create(&tid_server[i],NULL,thread_server,arg_serv);

	for (i=0;i<MAX_SERVER;i++)
		pthread_join(tid_server[i],NULL);

	shutdown(server_struct.ls,SHUT_RDWR);
	close(server_struct.ls);
	return 0;
}
