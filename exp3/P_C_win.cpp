# include <windows.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>



# define  PROCESS_NUM  7
# define  BUF_SIZE     4

# define  P_NUM        3
# define  C_NUM        4

# define  P_CIRCLE     4
# define  C_CIRCLE	   3

# define  LETTER_NUM   3


# define  TIME_DELAY   3000



PROCESS_INFORMATION StartClone(int nCloneID);  //return the value of new and main process

struct SHM       //the struct for buf
{
	int read;
	int write;
	char buf[BUF_SIZE];
	int num;          
};
char name[LETTER_NUM]={'C','H','M'};    
//get a random number to sleep
int get_random()
{
	int number;

	srand((UINT) GetCurrentTime());  //a random seed,you will treat sometimes by this fuction

	number = rand() % TIME_DELAY;
	return number;
}

int main(int argc, char* argv[])
{   srand((unsigned)(time(NULL)));
	int  nClone = 0;
	int i, time, j;
	HANDLE SEM_FULL;    //handle for share
	HANDLE SEM_EMPTY;
	HANDLE SEM_MUTEX;
	HANDLE hMap;
		
	PROCESS_INFORMATION nH[PROCESS_NUM];// an array to get the info of each process
	struct SHM *shm;

	 if (argc > 1)            //to  get the  PID of the new process
	 {
	 	sscanf(argv[1], "%d", &nClone);//
	}
	
	if (nClone == 0) //  the main process
	{
	 
		//create semaphore
		SEM_FULL = CreateSemaphore(NULL, 0, BUF_SIZE, "FULL");  //NULL,can not inherit bu the child process
		SEM_EMPTY = CreateSemaphore(NULL, BUF_SIZE, BUF_SIZE, "EMPTY");
		SEM_MUTEX = CreateSemaphore(NULL, 1, 1, "MUTEX");

		HANDLE CurrentProcess = GetCurrentProcess();//The return value is a pseudo handle to the current process

		/*Creates or opens a named or unnamed file mapping object for a specified file.
		here is a pre create,it means you just hand on a request

		*/

		hMap = CreateFileMapping(INVALID_HANDLE_VALUE,  //just a handle ,without any point
												NULL,  //the default secrity
										PAGE_READWRITE,//the authority you have on the map
													0, //high bit size,set 0 as usual
										sizeof(*shm),  //the size you want
										"buffer"       // the share name
								); 

		shm = (struct SHM*)MapViewOfFile(hMap, //the handle createfilemapping  returned
								FILE_MAP_WRITE,//the type of acess to file map,follow the arg[3] of create
								 0, 		//A high-order DWORD of the file offset where the view begins.
								 0, 		//A low-order DWORD of the file offset where the view is to begin. 
								 sizeof(*shm)); //the size of map file
		//return the address of map file if succeed,the format trans is necessary
		  
		  shm->read=0;
		  shm->write=0;
		  shm->num =0;

		strcpy(shm->buf,"----");  //initallization
		

		//clone the process
		for (i = 0; i<PROCESS_NUM; i++)
		{
			nH[i] = StartClone(++nClone);
		}

		// wait
		for (i = 0; i<PROCESS_NUM; i++)
			WaitForSingleObject(nH[i].hProcess, INFINITE);

		//close the handle
		for (i = 0; i<PROCESS_NUM; i++)
		{
			CloseHandle(nH[i].hProcess);  //new process handle 
			CloseHandle(nH[i].hThread);		//the father's handle 
		}
		//close the mutex
		CloseHandle(SEM_MUTEX);
		CloseHandle(hMap);
		CloseHandle(SEM_EMPTY);
		CloseHandle(SEM_FULL);
	}
	else if (nClone > 0 && nClone < P_NUM+1)     //Producer
	{
		//Sleep(1);   //for more caual  looks like
		//return the handle if succeed   all possible access,maybe lower   ,true is inherite
		SEM_EMPTY = OpenSemaphore(SEMAPHORE_ALL_ACCESS,0 ,"EMPTY");
		SEM_FULL = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 0, "FULL");
		SEM_MUTEX = OpenSemaphore(SEMAPHORE_ALL_ACCESS,0, "MUTEX");

		hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, "buffer");
		shm = (struct SHM*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, sizeof(*shm));

		for (i = 0; i<P_CIRCLE; i++)
		{    
			// P 
			Sleep(2);
			WaitForSingleObject(SEM_EMPTY, INFINITE);
			WaitForSingleObject(SEM_MUTEX, INFINITE);

			//sleep
			time = get_random();
			Sleep(time);


        //  get the time 
			SYSTEMTIME curtime;
			GetSystemTime(&curtime);

			//get a random for select letter
			int rannum=get_random()%LETTER_NUM;


			shm->buf[shm->write]=name[rannum];
			shm->write = (shm->write+1)%BUF_SIZE;
			shm->num++;   //for record
			
			// print the list and time  as well as sepcfic info
			printf("[%02d]\t%02d:%02d:%02d\t",
				shm->num, curtime.wHour + 8, curtime.wMinute, curtime.wSecond);

			if(i==P_CIRCLE-1)
				printf("*");
			else
				printf(" ");
			printf("P%d puts %c  -->", nClone,name[rannum]);
			printf("  buf: ");
			for (j = 0; j<BUF_SIZE; j++)
			{
				printf("%c ", shm->buf[j]);
			}
			printf("\n");

			//V
			ReleaseSemaphore(SEM_MUTEX, 1, NULL);
			ReleaseSemaphore(SEM_FULL, 1, NULL); //semaphore ,step, pointer to previous semaphore if you need
		}

		//all has ended
		CloseHandle(SEM_MUTEX);
		CloseHandle(SEM_EMPTY);
		CloseHandle(SEM_FULL);
		CloseHandle(hMap);
		

		exit(0);

	}
	else if (nClone>P_NUM && nClone< C_NUM+P_NUM+1)    //customer
	{
		//add semaphore
		SEM_EMPTY = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 0, "EMPTY");
		SEM_FULL = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 0, "FULL");
		SEM_MUTEX = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 0, "MUTEX");

		hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, "buffer");
		shm = (struct SHM*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, sizeof(*shm));

		for (i = 0; i<C_CIRCLE; i++)
		{
			WaitForSingleObject(SEM_FULL, INFINITE);
			WaitForSingleObject(SEM_MUTEX, INFINITE);

			//sleep
			time = get_random();
			Sleep(time);

			SYSTEMTIME curtime;
			GetSystemTime(&curtime);
             char sb=shm->buf[shm->read];
		    shm->buf[shm->read]='-';
			shm->read=(shm->read + 1)%BUF_SIZE;
			shm->num++;//for record
			//print
			printf("[%02d]\t%02d:%02d:%02d\t", 
				shm->num,curtime.wHour + 8, curtime.wMinute, curtime.wSecond);
			if(i==C_CIRCLE-1)
				printf("*");
			else
				printf(" ");
			printf("C%d gets %c  -->", nClone- P_NUM ,sb);
			printf("  buf: ");
			for (j = 0; j<4; j++)
			{
				printf("%c ", shm->buf[j]);
			}
			printf("\n");

			//release semaphore
			ReleaseSemaphore(SEM_MUTEX, 1, NULL);
			ReleaseSemaphore(SEM_EMPTY, 1, NULL);

			
		}

		//close handle
		CloseHandle(SEM_MUTEX);
		CloseHandle(SEM_EMPTY);
		CloseHandle(SEM_FULL);
		CloseHandle(hMap);
		exit(0); //exist successfully
	}

	//system("pause");
	printf("\nEVERTHING HAS BEEN DOEN SUCCESSFULLY!\n");
	return 0;

}

// clone the process function
PROCESS_INFORMATION StartClone(int nCloneID)
{
	// get 
	TCHAR szFilename[MAX_PATH];
	GetModuleFileName(NULL, szFilename, MAX_PATH);//NULL   filepathname,getthe filepathname include .exe
	//printf("%s\n",szFilename );

	//command line with name and ID
	TCHAR szCmdLine[MAX_PATH];
	sprintf(szCmdLine, "\"%s\" %d", szFilename, nCloneID);  //commond line
	//printf("%s\n",szCmdLine );

	STARTUPINFO si;   // si  for createprocess,for more to see startupinfo on csdn

	ZeroMemory(reinterpret_cast <void*> (&si), sizeof(si));
	// size of struct  in byte
	si.cb = sizeof(si);

	//output  include the current and its' father
	PROCESS_INFORMATION pi;     
	BOOL bCreateOK = CreateProcess(NULL,szCmdLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);					
		if(!bCreateOK)
		{
			printf("CreateProcess Failed %d\n",GetLastError());
		}						
	return pi;
}
