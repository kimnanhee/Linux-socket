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
#define DELIM " \n\t"

int serv_sock; // global type
int clnt_sock;
int block = 1;
int state;
char contents[1024];
char *command;
char *ptr;

void nh_send(char* buf, int size);
void error_handling(char *message);
void nh_cd();
void nh_ls();
void nh_get();
void nh_put();

int main(int argc, char *argv[])
{
	if(argc != 2)
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

	while(1)
	{
		memset(contents, 0, 1024);
		int str_len = read(clnt_sock, contents, 1024);
		printf("%s %ld", contents, strlen(contents));

		command = strtok(contents, DELIM);

		if(!strcmp(command, "cd")) state = 1;
		else if(!strcmp(command, "pwd")) state = 2;
		else if(!strcmp(command, "ls")) state = 3;
		else if(!strcmp(command, "get")) state = 4;
		else if(!strcmp(command, "put")) state = 6;

		if(state == 2) // pwd
		{
			char cwd[1024]={0,};
			getcwd(cwd, 1024);
			cwd[strlen(cwd)] = '\n';
			nh_send(cwd, strlen(cwd));
		}
		else if(state == 1) // cd
		{
			ptr = strtok(NULL, DELIM); // 경로 가져오기
			nh_cd(ptr);
		}
		else if(state == 3) // ls
		{
			nh_ls(ptr);
		}
		else if(state == 4) // get, mget
		{
			ptr = strtok(NULL, DELIM);
			nh_get();
		}
		else if(state == 6) // put
		{
			ptr = strtok(NULL, DELIM);
			nh_put(ptr);
			nh_send("put finished\n", strlen("put finished\n"));
		}
	}
	return 0;
}
void nh_cd()
{
	// printf("in cd\n");
	if(chdir(ptr) < 0)
	{
		nh_send("cd fail\n", strlen("cd fail\n"));
	}
	else
	{
		nh_send("cd success\n", strlen("cd success\n"));
	}
}
void nh_ls()
{
	// printf("in ls\n");
	char buff[1024];
	char cwd[1024];
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	struct stat buf;

	printf("getcwd in ls\n");
	getcwd(cwd, 1024);
	if((dir = opendir(cwd)) == NULL) printf("dir error\n");
	
	printf("inode mode size name\n");
	while((entry = readdir(dir)) != NULL)
	{
		if (entry->d_ino != 0)
		{
			lstat(entry->d_name, &buf);
			sprintf(buff, "%s%s | %6ld %6o %8ld %s\n", (S_ISREG(buf.st_mode) ? "FILE" : ""), (S_ISDIR(buf.st_mode) ? "DIR " : ""), buf.st_ino, buf.st_mode, buf.st_size, entry->d_name);
			printf("%s", buff);
			nh_send(buff, strlen(buff));
		}
	}
	closedir(dir);
}

void nh_put()
{
	int length;
	int fp = open(ptr, O_RDWR | O_TRUNC | O_CREAT, 0744);
	char buff[1024];
	read(clnt_sock, (char*)&length, sizeof(int));
	int read_size = length > 1024 ? 1024 : length;
	while (read_size)
	{
		int red = read(clnt_sock, buff, read_size);
		write(fp, buff, red);
		length -= red;
		read_size = length > 1024 ? 1024 : length;
	}
	close(fp);
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void nh_send(char* buf, int size) // 배열에 저장되어 있는 문자열 보내기
{
	char type = 'M';
	write(clnt_sock, (char*)&size, sizeof(int));
	write(clnt_sock, &type, sizeof(char));
	write(clnt_sock, buf, size);	
}

void nh_get() // 파일 보내기
{
	char* path = ptr;
	char type = 'F';
	int fp = open(path, O_RDONLY); // 파일 열기
	char buff[1024];
	if(fp > 0)
	{
		struct stat info;
		stat(path, &info);
		info.st_size;

		write(clnt_sock, (char*)&info.st_size, sizeof(int)); // 전체 파일 사이즈
		write(clnt_sock, &type, sizeof(char)); // 파일 형식

		while(1)
		{
			memset(buff, 0, 1024);
			int str_len = read(fp, buff, 1024);
			// printf("%s", buff);
			if(str_len > 0) write(clnt_sock, buff, str_len);
			else break;
		}
		printf("get finish\n");
	}
	else // 파일 열 수 없을 때
	{
		memset(buff, 0, 1024);
		strcpy(buff, "can not get file\n");
		printf("%s", buff);
		nh_send(buff, strlen(buff));
	}
	close(fp);	
}