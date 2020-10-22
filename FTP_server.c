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

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;

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

	char contents[1024]; 
	char buff[1024];
	int state = 0;
	while(1)
	{
		memset(contents, 0, 1024);

		state = 0; // command number
		read(clnt_sock, contents, sizeof(contents)-1);
		int i, re = 0;
		char command[10]={0, };
		for(i=0; i<strlen(contents); i++)
		{	
			if(contents[i] == ' ') re = i;
			
			if(re == 0) command[i] = contents[i];
			else contents[i-re-1] = contents[i];
		}
		for(i=strlen(contents)-re-1; i<strlen(contents); i++)
		{
			contents[i]='\0';
		}

		printf("\n\n\ncommand - %s\n", command);
		printf("contents - %s\n", contents);

		if(strcmp(command, "cd") == 0) state = 1;
		else if(strcmp(command, "ls") == 0) state = 2;
		else if(strcmp(command, "put") == 0) state = 3;
		else if(strcmp(command, "get") == 0) state = 4;
		else if(strcmp(command, "mput") == 0) state = 5;
		else if(strcmp(command, "mget") == 0) state = 6;
		printf("state - %d\n", state);
		
		if(state == 1) // cd
		{	
			printf("before cd - %s\n", getcwd(buff, 1024));
			if(chdir(contents) < 0) // fail
			{
				memset(buff, 0, strlen(buff));
				sprintf(buff, "fail to cd\n");
				write(clnt_sock, buff, strlen(buff));
			}
			else
			{
				memset(buff, 0, strlen(buff));
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
		else if(state == 3); // put
		else if(state == 4);
		else if(state == 5);
		else if(state == 6);
		
		sprintf(buff, "EOC"); // end check string
		write(clnt_sock, buff, strlen(buff));
		printf("%s\n", buff);
	}
	close(clnt_sock);
	close(serv_sock);
	return 0;
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
