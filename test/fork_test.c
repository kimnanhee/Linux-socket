/* "[�ҽ� 2] fork ����" */  

#include <unistd.h>
#include <stdio.h>
 
int main(void) 
{          
     int pid;		// Process ID
     int count;	// ���� ����
     
     count = 0;
     
     pid = fork();
     
     if(pid == -1)	/* ���μ��� ���� ���� �� */
     {
     	return 1;
     }
     else  if(pid != 0)	 /* �θ� ���μ��� */
     {   	
      	count++;
      	printf("This is parent.. : %d\n", count);
     }

     if(pid == 0)	 /* �ڽ� ���μ��� */
     {
     	count++;
     	printf("This is child.. : %d\n", count);
     }
     
     return 0;
}