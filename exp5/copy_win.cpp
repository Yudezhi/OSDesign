#include <stdio.h>  
#include <windows.h>
#include <iostream>
#include <string>


using namespace std;

int FileCopy(char * root, char * des)
{
	HANDLE hroot, hdes;
	DWORD dwroot, dwdes;

	//
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(root, &FindFileData);

	//
	LONG size = (FindFileData.nFileSizeHigh * (MAXDWORD + 1)) + FindFileData.nFileSizeLow;  //The size of the file is equal to
	char * lpBuffer = new char[size];

	//
	hroot = CreateFile(root, //file path include name
				GENERIC_READ,//read
				0, //withoutshare
				NULL, //pointer
				OPEN_EXISTING, //there must exist 
				 FILE_ATTRIBUTE_NORMAL,//default
				  NULL);//without templete


	hdes = CreateFile(des, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, //overwrite if exist
					 FILE_ATTRIBUTE_NORMAL, NULL);

	//read and write
	ReadFile(hroot,//handle
		 lpBuffer, //buffer
		 size,//size
		  &dwroot,//read
		   NULL);
	WriteFile(hdes,//handle
			 lpBuffer,//buffer
			  dwroot,
			   &dwdes,
			    NULL);

	//
	if (dwroot != dwdes)
	{
		cout << "Write Error!" << endl;
		return -1;
	}

	//end file copy
	CloseHandle(hroot);
	CloseHandle(hdes);
	return 0;
}

//
void Traversal(char * lpPath, char * desPath)
{
	char save_path[200];
	char szFile[MAX_PATH] = { 0 };  //source folder and its sub files
	char desFile[MAX_PATH] = { 0 };
	char szFind[MAX_PATH];   //the source folder
	char root[MAX_PATH];
	char des[MAX_PATH];

	WIN32_FIND_DATA FindFileData;
	WIN32_FIND_DATA FindFileData2;
	/*
	typedef struct _WIN32_FIND_DATA { 
  DWORD dwFileAttributes; //FILE_ATTRIBUTE_DIRECTORY   -->Indicates that the handle identifies a directory.
  FILETIME ftCreationTime; 
  FILETIME ftLastAccessTime; 
  FILETIME ftLastWriteTime; 
  DWORD nFileSizeHigh; 
  DWORD nFileSizeLow; 
  DWORD dwOID; 
  TCHAR cFileName[MAX_PATH]; 
} WIN32_FIND_DATA; 
*/

	strcpy(szFind, lpPath);
	strcat(szFind, "*.*");
	printf("szFind:%s\n",szFind );
	getchar();
	//get the handle of source file
	HANDLE hFind = FindFirstFile(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		printf("can not find the first file!\n");
		return ;
	}



	while (TRUE)
	{
		//folder
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//
			printf("%s\n",FindFileData.cFileName );
			//printf("%c\n",FindFileData.cFileName[1] );
			
			if (FindFileData.cFileName[0] != '.'|| FindFileData.cFileName[1]!='\0')	//以空字符结尾的字符串，即文件的名称。 not  .
			{
				if(FindFileData.cFileName[1] != '.' || FindFileData.cFileName[2]!='\0') // not  ..
				{

					// copy the complete  folder path
				strcpy(szFile, lpPath);
				strcat(szFile, FindFileData.cFileName);
				strcat(szFile, "//");

				strcpy(desFile, desPath);
				strcat(desFile, FindFileData.cFileName);
				strcat(desFile, "//");
				//create 
				if (FindFirstFile(desFile, &FindFileData2) == INVALID_HANDLE_VALUE)
				{
					CreateDirectory(desFile, NULL);
				}
				//
				Traversal(szFile, desFile);
				}
			}
			else
			{
				printf("--\n");
			}
		}
		else  // specific  file
		{
			//source file path include file name 
			strcpy(root, lpPath);
			strcat(root, FindFileData.cFileName);


			strcpy(des, desPath);
			strcat(des, FindFileData.cFileName);

			// copy file 
			if (FileCopy(root, des) == 0)
				cout << root << " copyed." << endl;

		}
		//
		if (!FindNextFile(hFind, &FindFileData))   //judge if a folder or file exist   true or false
			break;
	}//end while


	FindClose(hFind);
}






int main(int argc, char * argv[])
{
	char * root = argv[1];
	char * des = argv[2];
	
	int len_1 =strlen(argv[1]);
	int len_2 = strlen(argv[2]);
	if(root[len_1-1]!='\\'&& root[len_1-1]!='/')  //copy a file only
		{
			string a=root;
			int pos=a.find_last_of("/\\");
			a= a.substr(pos+1); //a is a filename
			string b=des;
			b+=a; // b is the new path include name
			//const char * A = a.c_str();
			char * B = (char *)b.c_str();
			FileCopy(root,B);
			printf("file finished!\n");
			return 0;
		}
	if(des[len_2-1]=='\\'||des[len_2-1]=='/')
	{
		des[len_2-1]='\0';
	}

	WIN32_FIND_DATA FindFileData;  //a  truct to store the file info 
	HANDLE hFind;
	hFind = FindFirstFile(des, &FindFileData);
   	if (hFind == INVALID_HANDLE_VALUE) 
	{
		//in :filename   out: filedata 
		if(CreateDirectory(des, NULL))
		{
			printf("Create folder successfully!\n");
		}
		else
		{
			printf("Create folder failed!\n");
		}
	}
	//
	getchar();
	des[len_2-1]='\\';
	Traversal(root, des);
}
