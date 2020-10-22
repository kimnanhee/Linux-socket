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

		state = 0;
		read(clnt_sock, contents, sizeof(contents)-1);
		int i;
		char command[10]={0, };
		for(i=0; i<strlen(contents); i++)
		{	
			if(contents[i] == ' ') break;
			command[i]=contents[i];
		}
		printf("%s\n", command);
		if(strcmp(command, "cd") == 0) state = 1;
		else if(strcmp(command, "ls") == 0) state = 2;
		else if(strcmp(command, "put") == 0) state = 3;
		else if(strcmp(command, "get") == 0) state = 4;
		else if(strcmp(command, "mput") == 0) state = 5;
		else if(strcmp(command, "mget") == 0) state = 6;
		
		printf("state - %d\n", state);
		if(state) write(clnt_sock, "OK", strlen("OK"));
		else write(clnt_sock, "NO", strlen("NO"));
	
		if(state == 1) // cd
		{
			sprintf(buff, "command receive, state == 1, cd fun");
			printf("%s\n", buff);
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
			sprintf(buff, "END");
			printf("%s\n", buff);
			write(clnt_sock, buff, strlen(buff));
			free(cwd);
			closedir(dir);
		}	
		// int length = read(fp, buff, sizeof(buff)-1); // read file
		// printf("%s", buff);
		// if(length > 0) write(clnt_sock, buff, length);
		// else break;
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
