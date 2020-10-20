// FTP server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	
	str_len = read(clnt_sock, file_path, sizeof(file_path)-1); // read open file path

	char command[20], contents[1024]; 
	int str_len, state = 0;
	while(1)
	{
		memset(command, 0, 20);
		memset(contents, 0, 1024);

		state = 0;
		read(clnt_sock, command, sizeof(command)-1);
		if(strcpy(command, "cd") == 0) state = 1;
		else if(strcpy(command, "ls") == 0) state = 2;
		else if(strcpy(command, "put") == 0) state = 3;
		else if(strcpy(command, "get") == 0) state = 4;
		else if(strcpy(command, "mput") == 0) state = 5;
		else if(strcpy(command, "mget") == 0) state = 6;

		if(state) write(clnt_sock, "Y", strlen("Y"));
		else write(clnt_sock, "N", strlne("N");

		read(clnt_sock, contents, sizeof(contents)-1);
		
		int length = read(fp, buff, sizeof(buff)-1); // read file
		// printf("%s", buff);
		if(length > 0) write(clnt_sock, buff, length);
		else break;
	}
	close(fp); // close file
	
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
