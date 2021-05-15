#include <stdio.h>
#include <string.h>
int main()
{
	char arr[1024]={0,};
	fgets(arr, 1024, stdin);
	
	char *ptr = strtok(arr, " "); // 공백을 기준으로 문자열 자르기, 포인터 반환
	printf("%s\n", ptr); // 자른 문자열 출력 
	
	int state = 0;
	if(!strcmp(ptr, "cd")) state = 1;
	else if(!strcmp(ptr, "ls")) state = 2;
	else if(!strcmp(ptr, "pwd")) state = 3;
	else if(!strcmp(ptr, "get")) state = 4;
	else if(!strcmp(ptr, "put")) state = 5;
	
	printf("%d\n", state);
	if(state == 4 || state == 5)
	{
		while(ptr = strtok(NULL, " ")) // 자른 문자열이 나오지 않을 떄까지 
		{
			printf("%s\n", ptr); // 자른 문자열 출력 
		}
	}
	
	return 0;
}
