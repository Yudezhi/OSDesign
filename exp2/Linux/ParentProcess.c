#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>    //fork()   execv()
#include <sys/time.h>  //time
#include <stdlib.h>   //exit()
#include <sys/wait.h>

int main (int argc,char* argv[])
{
	if(argc==1)
	{
		printf("The Parameters you input can not be konwn!\n");
		return (-1);
	}
	argv[2]=NULL;
	struct timeval stime,etime;
	gettimeofday(&stime,NULL); //get the time of start

	pid_t pid;  //get the value of fork()   return 

	if((pid=fork())<0)
	{
	printf("fork() fuction fork error.\n");
	exit (-1);
	}

	//printf("%d\n",pid);
	if(pid > 0)
	{
	wait(0);
	gettimeofday(&etime,NULL);//get the time of end
	
	long long time=(etime.tv_sec-stime.tv_sec)*1000000LL + (etime.tv_usec-stime.tv_usec);
	printf("us: %ld\n",time);
	printf("The new process used %lld ms\n",time/1000);
	}

	//====================================Create ChildProcess======================================
	//printf("%d\n",pid);
	if(pid==0)
	{

	//printf("%s\n",argv[1]);
	execv(argv[1],argv+1);
	exit(0);  //exit ChildProcess
	}
	
	exit (0);
}
