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
	
	char buff[1024];
	int str_len;

	if(argc!=4)
	{
		printf("Usage : %s <IP> <port> <file name>\n", argv[0]);
		exit(1);
	}
	sock = socket(PF_INET,SOCK_STREAM, 0);
	if(sock == -1) error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("connect() error!");

	write(sock, argv[3], strlen(argv[3]));
	
	char write_file_path[]="content.txt";
	int fp=open(write_file_path, O_RDWR | O_CREAT);

	while(1)
	{
		memset(buff, 0, 1024);
		str_len = read(sock, buff, sizeof(buff)-1);
		if(str_len == -1) error_handling("read() error!");
		printf("%s", buff);
		if(strcmp(buff, "END")==0) break;
		else write(fp, buff, sizeof(buff));
	}
	printf("content.txt file open please\n");
	close(sock);
	return 0;
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
