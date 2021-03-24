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
#define DELIM " \n\t"

void error_handling(char *message);
void* readThread(void*);
int sock; // global type
int block = 0;
int state = 0;
char contents[1024];
char *command; 
char *ptr;

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

	while(1)
	{
		while(block);
		memset(contents, 0, 1024);
		fgets(contents, 1024, stdin); // input
		//contents[strlen(contents)-1] = '\0'; // 맨 뒤 개행 제거

		command = strtok(contents, DELIM);

		if(!strcmp(command, "cd")) state = 1;
		else if(!strcmp(command, "pwd")) state = 2;
		else if(!strcmp(command, "ls")) state = 3;
		else if(!strcmp(command, "get")) state = 4;
		else if(!strcmp(command, "mget")) state = 5;
		else if(!strcmp(command, "put")) state = 6;
		else if(!strcmp(command, "mput")) state = 7;
		else if(!strcmp(command, "quit")) break;
		else{printf("wrong command\n"); continue;}

		// printf("%d\n", state);

		if(state == 2) write(sock, command, strlen(command)); // pwd
		else // cd, ls, get, mget, put, mput
		{
			ptr = strtok(NULL, DELIM); // 파일 이름 가져오기
			
			if (state == 5) // mget
			{
				printf("%s get? (y/n) : ", ptr);
				char input[10]={0};
				fgets(input, 10, stdin);
				
				if(*input != 'y')
				{
					continue;
				}
				strcpy(command, "get");
			}
			if(state == 7) // mput
			{
				printf("%s put? (y/n) : ", ptr);
				char input[10]={0};
				fgets(input, 10, stdin);

				if(input[0] != 'y')
				{
					continue;
				}
				strcpy(command, "put");
			}
			char buff[1024]={0,};
			sprintf(buff, "%s %s ", command, ptr); // 명령어, 파일 이름 보내기
			write(sock, buff, strlen(buff));
			
			if(state == 6 || state == 7) // put, mput
			{
				struct stat info;
				stat(ptr, &info);
				write(sock, (char*)&info.st_size, sizeof(int)); // 전체 파일 사이즈
				int fp = open(ptr, O_RDONLY);
				if(fp > 0)
				{
					while(1)
					{
						memset(buff, 0, 1024);
						int str_len = read(fp, buff, 1024);
						if(str_len > 0) write(sock, buff, str_len);
						else break;
					}
				}
				else printf("wrong file name\n");
				close(fp);
			}
		}
		block = 1; // 읽고 있는 중에는 block이 1
	}	
	close(sock);
	return 0;
}

void* readThread(void* args)
{
	while (1)
	{
		int length = 0, read_size, fd = STDOUT_FILENO;
		char type;
		char buff[1024] = {0,};
		
		read(sock, (char*)&length, sizeof(int)); // 보내는 파일 크기 읽어오기
		read(sock, &type, sizeof(char)); // 보내는 파일의 타입 읽어오기 (M or F)
		// printf("%c\n", type);

		if (type == 'F' && (state == 5 || state == 4)) // get, mget
		{
			fd = open(ptr, O_CREAT | O_TRUNC | O_WRONLY, 0744);
		}
		
		read_size = length > 1024 ? 1024 : length;
		while (read_size)
		{
			int red = read(sock, buff, read_size);
			write(fd, buff, red);
			length -= red; // 파일 크기 - 현재 읽어온 크기
			read_size = length > 1024 ? 1024 : length;
		}
		block = 0; // 다 읽었을 경우, block이 0
		if (fd != STDOUT_FILENO) close(fd);
	}
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(0);
}
