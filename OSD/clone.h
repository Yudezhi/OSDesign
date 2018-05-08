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
