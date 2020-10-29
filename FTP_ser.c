// FTP server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

int serv_sock; // global type
int clnt_sock;

void error_handling(char *message);
void* readThread(void*);

int main(int argc, char *argv[])
{
	if(argv != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(0);
	}
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
	
	serv_sock = socket(PF_INET, SOCK_STREAM, 0); // make socket
	if(serv_sock == -1) error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("bind() error"); // bind error
	if(listen(serv_sock, 5)==-1) error_handling("listen() error"); // listen error

	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	if(clnt_sock == -1) error_handling("accept() error");

	pthread_t tid;
	pthread_create(&tid, NULL, readThread, NULL); // make thread
}
void* readThread(void* args)
{
	while(1)
	{
		block = 1;
		memset(contents, 0, 1024);
		int str_len = read(clnt_sock, contents, 1024);
		char command[10]={0,};
		int i, re=0;
		for(i=0; i<strlen(contents); i++)
		{
			if(contents[i]==' ' && re==0) re = i;

			if(re == 0) command[i] = contents[i];
			else contents[i-re-1] = contents[i];
		}
		for(i=strlen(contents)-re-1; i<strlen(contents); i++)
		{
			contents[i] = '\0';
		}
		contents[strlen(contents)-1] = '\0';

		if(strcmp(command, "cd") == 0) state = 1;
                else if(strcmp(command, "ls") == 0) state = 2;
                else if(strcmp(command, "put") == 0) state = 3;
                else if(strcmp(command, "get") == 0) state = 4;
                else if(strcmp(command, "mput") == 0) state = 5;
	        else if(strcmp(command, "mget") == 0) state = 6;
		else if(strcmp(command, "pwd") == 0) state = 7;
		else if(strcmp(command, "set") == 0) 
		{
			char cwd[1024]={0,};
			strcpy(set_direct, getcwd(cwd, 1024));
			sprintf(cwd, "save file direct - %s\n", set_direct);
			printf("%s", cwd);
			write(clnt_sock, cwd, strlen(cwd));
		}
		printf("command - %s\n", command);
		printf("contents - %s %ld\n", contents, strlen(contents));
		printf("state - %d\n", state);
		block = 0;
		while(state);
	}
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
