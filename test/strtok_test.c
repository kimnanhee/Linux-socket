#include <stdio.h>
#include <string.h>
int main()
{
	char arr[1024]={0,};
	fgets(arr, 1024, stdin);
	
	char *ptr = strtok(arr, " "); // ������ �������� ���ڿ� �ڸ���, ������ ��ȯ
	printf("%s\n", ptr); // �ڸ� ���ڿ� ��� 
	
	int state = 0;
	if(!strcmp(ptr, "cd")) state = 1;
	else if(!strcmp(ptr, "ls")) state = 2;
	else if(!strcmp(ptr, "pwd")) state = 3;
	else if(!strcmp(ptr, "get")) state = 4;
	else if(!strcmp(ptr, "put")) state = 5;
	
	printf("%d\n", state);
	if(state == 4 || state == 5)
	{
		while(ptr = strtok(NULL, " ")) // �ڸ� ���ڿ��� ������ ���� ������ 
		{
			printf("%s\n", ptr); // �ڸ� ���ڿ� ��� 
		}
	}
	
	return 0;
}
