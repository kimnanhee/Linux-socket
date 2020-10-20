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

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
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

	char write_file_path[]="content.txt";
	int fp=open(write_file_path, O_RDWR | O_CREAT | O_TRUNC);
	
	char command[20], contents[1024], buff[1024];
	while(1)
	{
		memset(command, 0, 20);
		memset(contents, 0, 1024);
		scanf("%s %[^\n]s", command, contents); // user input
		if(command == "quit") break;
		write(sock, command, strlen(command)); // write socket
		write(sock, contents, strlen(contents));
		
		while(1) // read text all
		{
			memset(buff, 0, 1024);
			int str_len = read(sock, buff, sizeof(buff)-1);
			printf("%s", buff);
			if(str_len <= 0) break;
		}
		printf("\n\n");
	}
	close(sock);
	return 0;
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
