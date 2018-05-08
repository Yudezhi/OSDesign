int num11=0;  // record the folder num
int num22=0;  // record the file number
int GetFileNum(char * lpPath)
{
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
	//printf("szFind:%s\n",szFind );
//	getchar();
	//get the handle of source file
	HANDLE hFind = FindFirstFile(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		printf("can not find the first file!\n");
	}
	while (TRUE)
	{
		//folder
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			
			if (FindFileData.cFileName[0] != '.'|| FindFileData.cFileName[1]!='\0')	// not  .
			{
				if(FindFileData.cFileName[1] != '.' || FindFileData.cFileName[2]!='\0') // not  ..
				{
					num11++;
					// copy the complete  folder path
				strcpy(szFile, lpPath);
				strcat(szFile, FindFileData.cFileName);
				strcat(szFile, "//");
				GetFileNum(szFile);

				}//not ..
			} // not .
			else
			{
				//printf("--\n");
			}
		}// end folder

		else  // specific  file
		{
			num22++;
		}
		//
		if (!FindNextFile(hFind, &FindFileData))   //judge if a folder or file exist   true or false
			break;
	}//end while
	FindClose(hFind);
	return num11+num22;
}


