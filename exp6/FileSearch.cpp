# include <windows.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>
# include "clone.h"
# include "GetFileNum.h"
# include <iostream>
# include <iomanip>
# include "writeHistory.h"

#define M  100

using namespace std;
struct SHM       //the struct for buf
{
	char folder[M][M];
    char file[M][M];
    int m; //the num process
	char commond [M][M];
    int folder_num;//folder number
	int file_num; //file number
	int num; //tatal files
	int x; //process bar
	
};
//================================================================
HANDLE mutex;  //handle for share
HANDLE hMap;



struct SHM *shm;  // a pointer point to share memory

//maybe initialization here**********************************


void Traversal(char * lpPath, char * desPath);
string printProg(int x);
//===============================================================
int main(int argc, char * argv[])
{
	
    
    //the m is the key and path
    int nClone=0;  //

    //struct SHM *shm;  // a pointer point to share memory

    PROCESS_INFORMATION nH[M];  //get the process info of each 

     if (argc > 1)            //to  get the  PID of the new process
	 {
	 	sscanf(argv[1], "%d", &nClone);//
	 }
    if(nClone==0)  //the main process
    {
    
        mutex = CreateSemaphore(NULL, 1, 1, "MUTEX");
        HANDLE CurrentProcess = GetCurrentProcess();//The return value is a pseudo handle to the current process

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
	
	//=======================Hello Part======================================
		printf("请输入要查找的路径数量\n");
    	scanf("%d",&shm->m); 
    //	printf("m=%d\n",shm->m);
        printf("请输入文件名和路径\n");
    	for(int i=0;i<=shm->m;i++)
    	{
    	scanf("%s",&shm->commond[i]);
    	}
	//======================Write History=====================================
		fstream fileout;
		WaitForSingleObject(history_mutex, INFINITE);
		fileout.open(path_history, ios::out | ios::ate | ios::app);
		if (fileout.is_open())
		{
			getTime();
			fileout << timeBuffer << "  在查找文件操作下执行了如下操作：" << endl;
			fileout << "查询了" << shm->m << "个路径\n";
			fileout << "查询关键字为：" << shm->commond[0] << endl;
			fileout << "检索的路径为：" << endl;
			for (int i = 1; i <= shm->m; i++)
				fileout << shm->commond[i] << endl;
			cout << endl; 
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
		}
		else
		{
			cout << "写入历史记录失败！" << endl;
			ReleaseSemaphore(history_mutex, 1, NULL);
		}
	//========================Write History End=================================
   // 	for(int i=0;i<=2;i++)
    //	{
    //	printf("%s\n",shm->commond[i]);
    //	}
   // //==============================================================
    
    //create process
    for (int i = 0; i<shm->m; i++)
		{
			nH[i] = StartClone(++nClone);
		}

//============================Process Bar==========================================================
	cout << "Searching ..." << endl;
	for(int i = 0; i<100; i++)
	{
        Sleep(100);
        //printf("\nx=%d,num=%d\n",shm->x,shm->num);
        //printf("\n\%=%d\n",((shm->x)*100)/shm->num);
    	cout << "\r" << setw(20) << printProg(shm->x) << " " << ((shm->x)*100)/shm->num << "%" << flush;
	//	cout << "\r" << setw(20)  << ((shm->x)*100)/shm->num << "%" << flush;
    }
//==========================================================================================
        // wait
	for (int i = 0; i<shm->m; i++)
			WaitForSingleObject(nH[i].hProcess, INFINITE);

		//close the handle
	for (int i = 0; i<shm->m; i++)
		{
			CloseHandle(nH[i].hProcess);  //new process handle 
			CloseHandle(nH[i].hThread);		//the father's handle 
		}
		//close the mutex
		CloseHandle(mutex);
		CloseHandle(hMap);
    }
    else if(nClone>0)
    {
    	//printf("I am p %d \n",nClone);
        mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS,0, "MUTEX");
        hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, "buffer");
		shm = (struct SHM*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, sizeof(*shm));

        WaitForSingleObject(mutex, INFINITE);
		shm->num+= GetFileNum(shm->commond[nClone]);
		ReleaseSemaphore(mutex, 1, NULL);
		//printf("shm->num:%d\n",shm->num);
       // printf("commond = %s\n",shm->commond[nClone]);
        Traversal(shm->commond[nClone], shm->commond[0]);
        //printf("P %d Tra done\n",nClone);
     

        CloseHandle(mutex);
		CloseHandle(hMap);
		//Sleep(100000);
		exit(0);
    }
    else
    {
        printf("Error!!!\n");
        printf("nClone= %d  m=%d \n",nClone,2);
        return 0;
    }
    printf("\n\nResult:\n");
    printf("\n\n%d file(s) found!\n",shm->file_num);
    for(int i=0;i<shm->file_num;i++)
   	printf("%s\n",shm->file[i]);
   	
   	printf("\n\n%d folder(s) found!\n",shm->folder_num);
	for(int i=0;i<shm->folder_num;i++)
	printf("%s\n",shm->folder[i]);	
	printf("\n\nSearch Ended!!!\n");

    return 0;
}


//=======================================Traversal.h=======================================================
#include <stdio.h>  
#include <windows.h>
#include <iostream>
#include <string>

#include <iomanip>
//#include "GetFileNum.h"

#define M  100
using namespace std;



string printProg(int x)
{
	
		x=((x+1)*100)/shm->num; 
	
    string s;
    s="[";
    for (int i=1;i<=(100/2);i++)
    {
        if (i<=(x/2) || x==100)
            s+="=";
        else if (i==(x/2))
            s+=">";
        else
            s+=" ";
    }

    s+="]";
    return s;
}

int FolderSearch(char * root, char * des)  //root is the file path,des is the file name
{
	WaitForSingleObject(mutex, INFINITE);
		shm->x++;
    ReleaseSemaphore(mutex, 1, NULL);
	Sleep(1000);
	string filepath=root;
	string b=des;

	//printf("des:%s\n", des);

	size_t found = filepath.find(b);  // find with substr
	if(found != string::npos)
	{
	
		//printf("find folder:%s\n",root);
        WaitForSingleObject(mutex, INFINITE);
		strcpy(shm->folder[shm->folder_num++],root);
        ReleaseSemaphore(mutex, 1, NULL);
		
	}
	//getchar();
	return 1;
}
int FileSearch(char * root, char * des)  //root is the file path,des is the file name
{
	WaitForSingleObject(mutex, INFINITE);
			shm->x++;
    ReleaseSemaphore(mutex, 1, NULL);
	Sleep(1000);
	string filepath=root;
	string b=des;

	int pos= filepath.find_last_of("/\\");
	string filename =" ";
	filename= filepath.substr(pos+1, string::npos);  //get the file name

	// printf("pos:%d\n",pos );
	// printf("root:%s\n",root);
	// printf("des:%s\n", des);
	// //printf("--%s--\n",filename);
	// cout<< "--"<<filename<<"---" <<endl;


	size_t found = filepath.find(b);  // find with substr
	if(found != string::npos)
	{
		
		//printf("find file:%s\n",root);
        WaitForSingleObject(mutex, INFINITE);
		strcpy(shm->file[shm->file_num++],root);
        ReleaseSemaphore(mutex, 1, NULL);

	}
	//getchar();
	
	return 1;
}

//
void Traversal(char * lpPath, char * desPath)
{
	//printf("path= %s,key = %s\n",lpPath,desPath);
	//Sleep(1000);
	char save_path[200];
	char szFile[MAX_PATH] = { 0 };  //source folder and its sub files
	char desFile[MAX_PATH] = { 0 };
	char szFind[MAX_PATH];   //the source folder
	char root[MAX_PATH];
	char des[MAX_PATH];

	WIN32_FIND_DATA FindFileData;
	WIN32_FIND_DATA FindFileData2;
												
	strcpy(szFind, lpPath);
	strcat(szFind, "*.*");
//	printf("szFind:%s\n",szFind );
//	getchar();
	//get the handle of source file
	HANDLE hFind = FindFirstFile(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		printf("can not find the first file!\n");
		return ;
	}

	while (TRUE)
	{
		//printf("true!\n");
		
	//	cout << "\r" << setw(20) << printProg(x) << " " << ((x+1)*100)/num << "%" << flush;

		//folder
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			
			
			if (FindFileData.cFileName[0] != '.'|| FindFileData.cFileName[1]!='\0')	// not  .
			{
				if(FindFileData.cFileName[1] != '.' || FindFileData.cFileName[2]!='\0') // not  ..
				{

					// copy the complete  folder path
				strcpy(szFile, lpPath);
				strcat(szFile, FindFileData.cFileName);
				strcat(szFile, "//");

				FolderSearch(szFile,desPath);
				
				//create destinate folder 
				if (FindFirstFile(desFile, &FindFileData2) == INVALID_HANDLE_VALUE)
				{
					//CreateDirectory(desFile, NULL);
					//cout<<"meet a folder!"<<endl;
				}
				//
				Traversal(szFile, desPath);

				}//not ..

			} // not .
			else
			{
				//printf("--\n");
			}
		}// end folder

		else  // specific  file
		{
			//source file path include file name 
			strcpy(root, lpPath);
			strcat(root, FindFileData.cFileName);


			// search file 
			if (FileSearch(root, desPath) == 0)
				cout << root << "filesearch return 0" << endl;

		}
		//
		if (!FindNextFile(hFind, &FindFileData))   //judge if a folder or file exist   true or false
			break;
	}//end while


	FindClose(hFind);
}
