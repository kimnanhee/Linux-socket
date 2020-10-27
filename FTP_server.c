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

void error_handling(char *message);
void* readThread(void*);
void get(char file_name[100]);
void put(char file_name[100]);

int serv_sock; // global type
int clnt_sock;
int state = 0; // command number
char contents[1024];
int block = 1;
char set_direct[1000]={0,}; // get file direct

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	if(argc!=2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

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
	char buff[1024];
	while(1)
	{
		while (block);
		if(state == 1) // cd
		{	
			printf("before cd - %s\n", getcwd(buff, 1024));
			printf("in cd func - %s %ld\n", contents, strlen(contents));
			if(chdir(contents) < 0) // fail
			{
				memset(buff, 0, strlen(buff));
				sprintf(buff, "fail to cd\n");
				write(clnt_sock, buff, strlen(buff));
			}
			else
			{
				memset(buff, 0, 1024);
				sprintf(buff, "%s\n", getcwd(buff, 1024));
				write(clnt_sock, buff, strlen(buff));
				printf("after cd - %s\n", getcwd(buff, 1024));
			}
		}
		else if(state == 2) // ls
		{
			char *cwd = (char *)malloc(sizeof(char)*1000);
			memset(cwd, 0, strlen(cwd));
			DIR *dir = NULL;
			struct dirent *entry = NULL;
			struct stat buf;

			getcwd(cwd, 1000);
			if((dir = opendir(cwd)) == NULL) printf("dir error\n");
			
			printf("inode mode size name\n");
			while((entry = readdir(dir)) != NULL)
			{
				lstat(entry->d_name, &buf);
				// if(S_ISREG(buf.st_mode)) printf("FILE ");
				// else if(S_ISDIR(buf.st_mode)) printf("DIR  ");
				// else printf("???  ");
				// printf("%6ld %6o %7ld %s\n", buf.st_ino, buf.st_mode, buf.st_size, entry->d_name); // file information
				sprintf(buff, "%s%s | %6ld %6o %8ld %s\n", (S_ISREG(buf.st_mode) ? "FILE" : ""), (S_ISDIR(buf.st_mode) ? "DIR " : ""), buf.st_ino, buf.st_mode, buf.st_size, entry->d_name);
				printf("%s", buff);
				write(clnt_sock, buff, strlen(buff));
			}
			free(cwd);
			closedir(dir);
		}
		else if(state == 3) // put
		{
			put(contents);
		}
		else if(state == 4) // get
		{
			get(contents);
		}
		else if(state == 5) // mput
		{
			if(strcmp(contents, "*") == 0)
			{
				printf("mput *\n");
			}
			else
			{
				char file_name[100]={0,};
				int i, re=0;
				for(i=0; i<strlen(contents); i++)
				{
					if(contents[i] == ' ')
					{
						re = i+1;
						printf("%s\n", file_name);
						put(file_name);
						memset(file_name, 0, 100);
					}
					file_name[i-re] = contents[i];
				}
				put(file_name);
			}
		}
		else if(state == 6) // mget
		{
			if(strcmp(contents, "*") == 0)
			{
				printf("mget *\n");
			}
			else // parsing and get()
			{
				char file_name[100]={0,};
				int i, re=0;
				//printf("%s %ld\n", contents, strlen(contents));
				for(i=0; i<strlen(contents); i++)
				{
					if(contents[i] == ' ')
					{
						re = i+1;
						printf("%s\n", file_name);
						get(file_name);
						memset(file_name, 0, 100);
					}
					file_name[i-re] = contents[i];
				}
				get(file_name);
			}
		}
		else if(state == 7)
		{
			memset(buff, 0, 1024); 
			sprintf(buff, "%s\n", getcwd(buff, 1024));
			printf("%s", buff);
			write(clnt_sock, buff, strlen(buff));
		}
		state=0;
	}
	close(clnt_sock);
	close(serv_sock);
	return 0;
}
void get(char file_name[100])
{
	int fp = open(file_name, O_RDONLY); // read file
	char buff[1024];
	if(fp > 0)
	{	
		char save_file[1024]={0,};
		sprintf(save_file, "%s/%s", set_direct, file_name); // set한 위치에 복사
		printf("save_file : %s\n", save_file);
		int wfp = open(save_file, O_RDWR | O_CREAT | O_TRUNC, 0744); // write file
		if(wfp > 0)
		{
			while(1)
			{
				memset(buff, 0, strlen(buff));
				int length = read(fp, buff, 1024);
				printf("%s", buff);
				if(length > 0) 
				{
					write(wfp, buff, length);
					write(clnt_sock, buff, length);
				}
				else break;
			}
			memset(buff, 0, 1024);
			sprintf(buff, "finish get file\n");
			write(clnt_sock, buff, strlen(buff));
		}
		else 
		{
			memset(buff, 0, 1024);
			sprintf(buff, "error get file\n");
			write(clnt_sock, buff, strlen(buff));
		}
		close(wfp);
	}
	else
	{
		sprintf(buff, "%s is wrong file name\n", file_name);
		printf("%s", buff);
		write(clnt_sock, buff, strlen(buff));
	}
	close(fp);
}
void put(char file_name[100])
{
	char save_file[1024]={0,};
	sprintf(save_file, "%s/%s", set_direct, file_name);
	int wfp = open(save_file, O_RDONLY);
	char buff[1024];
	if(wfp > 0)
	{
		int fp = open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0744);
		if(fp > 0)
		{
			while(1)
			{
				memset(buff, 0, 1024);
				int length = read(wfp, buff, 1024);
				printf("%s", buff);
				if(length > 0)
				{
					write(fp, buff, length);
					write(clnt_sock, buff, length);
				}
				else break;
			}
			memset(buff, 0, 1024);
			sprintf(buff, "finish put file\n");
			write(clnt_sock, buff, strlen(buff));
		}
		else 
		{
			memset(buff, 0, 1024);
			sprintf(buff, "error put file\n");
			write(clnt_sock, buff, strlen(buff));
		}
		close(fp);
			
	}
	else 
	{
		sprintf(buff, "%s is wrong file name\n", file_name);
		printf("%s", buff);
		write(clnt_sock, buff, strlen(buff));
	}
	close(wfp);
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
			printf("save file derect - %s\n", cwd);
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
