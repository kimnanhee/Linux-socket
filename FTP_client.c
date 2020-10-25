// FTP client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
void error_handling(char *message);
void* readThread(void*);
int sock; // socket pointer

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr;
	
	if(argc!=3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	sock = socket(PF_INET,SOCK_STREAM, 0); // make socket
	if(sock == -1) error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("connect() error!");

	pthread_t tid;
	pthread_create(&tid, NULL, readThread, NULL); // make thread
	char contents[1024];
	printf("enter <command> <contents>\n");
	printf("enter the \"quit\" break\n\n");
	while(1)
	{
		memset(contents, 0, strlen(contents));
		fgets(contents, 1024, stdin); // input
		if(strcmp(contents, "quit") == 0) break;
		write(sock, contents, strlen(contents)); // write socket
	}
	close(sock);
	return 0;
}

void* readThread(void* args)
{
	while (1)
	{
		char buff[1024] = {0,};
		int str_len = read(sock, buff, 1024);
		if (str_len == 0) // server down
		{
			printf("Connection Refused\n");
			exit(0);
		}
		printf("%s", buff);
	}
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
