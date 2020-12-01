/* "[�ҽ� 1] signal ���� */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void call(int sig)
{
	/* SIGINT ���� ��� */
	printf("I got signal %d\n", sig);	
	
	/* �ñ׳� ���� : �߻� �� �ñ׳��� ������ */
	(void)signal(SIGINT, SIG_DFL);
}

int main()
{
	/* �ñ׳� ���� : �߻� �� call �Լ��� ȣ�� �� */
	(void)signal(SIGINT, call);	
	
	while(1)
	{
		printf("Hello World\n");
		sleep(1);
	}
}
