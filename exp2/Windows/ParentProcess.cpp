#include <iostream>
#include <windows.h> 
#include <stdio.h>
#define MAX_BYTE 20

using namespace std;

int main(int argc, char *argv[])
{

	if (argc != 2) //get the parameter number 
	{
		printf("The Parameters you input can not be known.\n");
		return -1;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb=sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	ULARGE_INTEGER stms, etms;  //unsigned 64 bit integer
	SYSTEMTIME stime, etime;	//year to ms
	FILETIME sftime, eftime;	//ns


	GetSystemTime(&stime);//get current system time and cout
	cout << "Start time:" << stime.wHour + 8 << ":" << stime.wMinute << ":" << stime.wSecond << "." << stime.wMilliseconds << endl;

	if (CreateProcess(NULL,// No module name (use command line)
					argv[1], // Command line
					NULL,// Process handle not inheritable
					NULL,// Thread handle not inheritable	
					false, // Set handle inheritance to FALSE
					CREATE_NEW_CONSOLE,// No creation flags or new
					NULL,// Use parent's environment block
					NULL, // Use parent's starting directory 
					&si, // Pointer to STARTUPINFO structure
					&pi)  // Pointer to PROCESS_INFORMATION structure
		)
	{
		
		printf("New PID is :%d\n", pi.dwProcessId);//print the information of process
        printf("Main PID is:%d\n", pi.dwThreadId);
        

		// WaitForSingleObject( pi.hProcess, INFINITE );//if to closed at once

    	// // Close process and thread handles. 
   		//  CloseHandle( pi.hProcess );
   		//  CloseHandle( pi.hThread );

		DWORD dw = WaitForSingleObject(pi.hProcess, INFINITE);//ensure correct
		switch(dw)
		{
			case WAIT_OBJECT_0:
				GetSystemTime(&etime);  //when end   pay attentoin to UTC+8
				cout << "End time:" << etime.wHour + 8 << ":" << etime.wMinute << ":" << etime.wSecond << "." << etime.wMilliseconds << endl;
				
				SystemTimeToFileTime(&stime, &sftime); //converts the system time to file time format
				SystemTimeToFileTime(&etime, &eftime);
				
				
				stms.LowPart = sftime.dwLowDateTime;
				stms.HighPart = sftime.dwHighDateTime;

				etms.LowPart = eftime.dwLowDateTime;
				etms.HighPart = eftime.dwHighDateTime;
				
				cout << "The new process ran " << (etms.QuadPart - stms.QuadPart)/10000 << "ms" << endl;
				break;
			case WAIT_FAILED:
				cout << "The new process failed to wait" << endl;
				break;
		}
	}


	else
	{
		cout << "Failed to create new process!" << endl;
		return -1;
	}

	system("pause");
	return 0;
}
