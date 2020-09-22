// ./big_file_client 127.0.0.1 5000 b.txt

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

	if(argc!=4)
	{
		printf("Usage : %s <IP> <port> <file name>\n", argv[0]);
		exit(1);
	}
	sock = socket(PF_INET,SOCK_STREAM, 0);
	if(sock == -1) error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr)); // 초기화 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("connect() error!");

	write(sock, argv[3], strlen(argv[3])); // file path 보내기 
	
	char write_file_path[]="content.txt"; // 받은 코드 저장 파일 
	int fp=open(write_file_path, O_RDWR | O_CREAT); // file open 

	while(1)
	{
		memset(buff, 0, 1024); // 초기화  
		int str_len = read(sock, buff, sizeof(buff));
		// printf("%s", buff); // 출력 
		if(str_len <= 0) break; // 받은 값이 있을 때까지 
		write(fp, buff, str_len); // 파일에 쓰기 
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
