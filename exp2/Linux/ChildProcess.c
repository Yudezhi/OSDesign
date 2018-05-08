#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main()

{
	
	printf("Hi,I am Mars,Do not response!\n");
	printf("My PID is %d \n",getpid());
	printf("My Parent PID is %d \n",getppid());
	//getchar();
	sleep(3);
	return 0;
}
