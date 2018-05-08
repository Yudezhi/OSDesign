#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <Windows.h>  
#include <Windowsx.h>  
#include <time.h> 
#include <cstdio>
#include <tchar.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <iomanip>
#include "conio.h"
#include "writeHistory.h"    //д����ʷ��¼����ɱ�����ص�ͷ�ļ�    ZYH

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma warning(disable: 4996)

#define WIDTH 10
#define DIV (1024*1024)
#define M  100
#define buf_size 4096  

using namespace std;

//��ʾ������ǣ��ñ�Ǳ�ʾ����Ӧ�ó�����ڴ���з��ʵ�����    YQK
inline bool TestSet(DWORD dwTarget, DWORD dwMask)
{
	return ((dwTarget &dwMask) == dwMask);
}

//�鿴������Ϣ��Ҫ    YQK
#define SHOWMASK(dwTarget,type) if(TestSet(dwTarget,PAGE_##type)){cout << "," << #type;}  

HANDLE GetProcessHandle(int ProcessID)
{
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
}

//�鿴���н��̵���Ϣ    YQK
void ShowAllProcess(int pid)
{
	PROCESSENTRY32 pe32;										//�洢������Ϣ
	pe32.dwSize = sizeof(pe32);									//��ʹ������ṹǰ�����������Ĵ�С
	PROCESS_MEMORY_COUNTERS ppsmemCounter;						//struct,�洢�����ڴ��ʹ����Ϣ�������ú���GetProcessMemoryInfo��ȡ���̵������Ϣ
	ppsmemCounter.cb = sizeof(ppsmemCounter);					//��ʼ����С
	HANDLE hProcessSnap;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);			//���վ��
	HANDLE hProcess;//���̾��
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		printf("�������̿���ʧ��.\n");
		exit(0);
	}
	//�������̿��գ�������ʾÿ�����̵���Ϣ
	BOOL bMore = Process32First(hProcessSnap, &pe32);						 //��ȡϵͳ���յ�һ�����̵���Ϣ��������ص�pe32�ṹ��
	printf("���̵Ĺ�������Ϣ:\n");
	while (bMore) {
		if (pid != -1)
		{
			if (pid == pe32.th32ProcessID)
			{
				//wcout << "��������:" << pe32.szExeFile << endl;//������Ϣ���洢��pe32�У�
				printf("�������ƣ�%ws\n", pe32.szExeFile);
				cout << "����ID:" << pe32.th32ProcessID << endl;
				cout << "�߳���:" << pe32.cntThreads << endl;
				hProcess = GetProcessHandle(pe32.th32ProcessID);
				GetProcessMemoryInfo(hProcess, &ppsmemCounter, sizeof(ppsmemCounter));//�����ڴ�ʹ����Ϣ���洢��ppsmemCounter�У�
				cout << "���ύ:" << ppsmemCounter.PagefileUsage / 1024 << " KB" << endl;
				cout << "������:" << ppsmemCounter.WorkingSetSize / 1024 << " KB" << endl;
				cout << "��������ֵ:" << ppsmemCounter.PeakWorkingSetSize / 1024 << " KB" << endl;
			}
			bMore = Process32Next(hProcessSnap, &pe32);						//��ȡϵͳ������һ��������Ϣ
		}
		else
		{
			//wcout << "��������:" << pe32.szExeFile << endl;//������Ϣ���洢��pe32�У�
			printf("�������ƣ�%ws \n", pe32.szExeFile);
			cout << "����ID:" << pe32.th32ProcessID << endl;
			cout << "�߳���:" << pe32.cntThreads << endl;
			hProcess = GetProcessHandle(pe32.th32ProcessID);
			GetProcessMemoryInfo(hProcess, &ppsmemCounter, sizeof(ppsmemCounter));	//�����ڴ�ʹ����Ϣ���洢��ppsmemCounter�У�
			cout << "���ύ:" << ppsmemCounter.PagefileUsage / 1024 << " KB" << endl;
			cout << "������:" << ppsmemCounter.WorkingSetSize / 1024 << " KB" << endl;
			cout << "��������ֵ:" << ppsmemCounter.PeakWorkingSetSize / 1024 << " KB" << endl;
			bMore = Process32Next(hProcessSnap, &pe32);						//��ȡϵͳ������һ��������Ϣ
		}
	}
	CloseHandle(hProcessSnap);												//�رտ���
}


//��ʾ�������    YQK
void ShowProtection(DWORD dwTarget)
{//�����ҳ�汣������  
	SHOWMASK(dwTarget, READONLY);
	SHOWMASK(dwTarget, GUARD);
	SHOWMASK(dwTarget, NOCACHE);
	SHOWMASK(dwTarget, READWRITE);
	SHOWMASK(dwTarget, WRITECOPY);
	SHOWMASK(dwTarget, EXECUTE);
	SHOWMASK(dwTarget, EXECUTE_READ);
	SHOWMASK(dwTarget, EXECUTE_READWRITE);
	SHOWMASK(dwTarget, EXECUTE_WRITECOPY);
	SHOWMASK(dwTarget, NOACCESS);
}

//��������ĵ�ַ�ռ�    YQK
void WalkVM(HANDLE hProcess)
{
	SYSTEM_INFO si; //ϵͳ��Ϣ�ṹ  
	ZeroMemory(&si, sizeof(si));    //��ʼ��  
	GetSystemInfo(&si); //���ϵͳ��Ϣ  

	MEMORY_BASIC_INFORMATION mbi;   //���������ڴ�ռ�Ļ�����Ϣ�ṹ  
	ZeroMemory(&mbi, sizeof(mbi));  //���仺���������ڱ�����Ϣ  

									//ѭ������Ӧ�ó����ַ�ռ�  
	LPCVOID pBlock = (LPVOID)si.lpMinimumApplicationAddress;
	while (pBlock < si.lpMaximumApplicationAddress)
	{
		//�����һ�������ڴ�����Ϣ  
		if (VirtualQueryEx(
			hProcess,   //��صĽ���  
			pBlock,     //��ʼλ��  
			&mbi,       //������  
			sizeof(mbi)) == sizeof(mbi))    //���ȵ�ȷ�ϣ����ʧ�ܷ���0  
		{
			//�����Ľ�β���䳤��  
			LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;
			TCHAR szSize[MAX_PATH];
			//������ת�����ַ���  
			StrFormatByteSize(mbi.RegionSize, szSize, MAX_PATH);

			//��ʾ���ַ�ͳ���  
			cout.fill('0');
			cout << hex << setw(8) << (DWORD)pBlock << "-" << hex << setw(8) << (DWORD)pEnd << (_tcslen(szSize) == 7 ? "(" : "(") << szSize << ")";

			//��ʾ���״̬  
			switch (mbi.State)
			{
			case MEM_COMMIT:
				printf("���ύ");
				break;
			case MEM_FREE:
				printf("����");
				break;
			case MEM_RESERVE:
				printf("��Ԥ��");
				break;
			}

			//��ʾ����  
			if (mbi.Protect == 0 && mbi.State != MEM_FREE)
			{
				mbi.Protect = PAGE_READONLY;
			}
			ShowProtection(mbi.Protect);

			//��ʾ����  
			switch (mbi.Type)
			{
			case MEM_IMAGE:
				printf(", Image");
				break;
			case MEM_MAPPED:
				printf(", Mapped");
				break;
			case MEM_PRIVATE:
				printf(", Private");
				break;
			}

			//�����ִ�е�ӳ��  
			TCHAR szFilename[MAX_PATH];
			if (GetModuleFileName(
				(HMODULE)pBlock,            //ʵ�������ڴ��ģ����  
				szFilename,                 //��ȫָ�����ļ�����  
				MAX_PATH) > 0)               //ʵ��ʹ�õĻ���������  
			{
				//��ȥ·������ʾ  
				PathStripPath(szFilename);
				printf(", Module:%s", szFilename);
			}

			printf("\n");
			//�ƶ���ָ���Ի����һ����  
			pBlock = pEnd;
		}
	}
}

//����PID��ѯ����������Ϣ    YQK
void QuerySingleProcess()
{
	int lineX = 0, lineY = 0;
	int flag = 0;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);	//���վ��
	HANDLE hProcess = NULL;													//���̾��
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot ����ʧ��.\n");
		exit(0);
	}
	cout << "�������ID����ѯ���̵��ڴ�ֲ��ռ䣺" << endl;
	int PID = 0;
	cin >> PID;
	hProcess = GetProcessHandle(PID);
	if (hProcess == NULL)
	{
		cout << "���̲����ڣ� " << endl;
		//Sleep(10000);
		//exit(0);
		QuerySingleProcess();

	}
	else
	{
		ShowAllProcess(PID);
		WalkVM(hProcess);
		Sleep(1000);
	}
	CloseHandle(hProcess);													//�رս���

	fstream fileout;
	WaitForSingleObject(history_mutex, INFINITE);
	fileout.open(path_history, ios::out | ios::ate | ios::app);
	getTime();
	fileout << timeBuffer << "  ��ѯ�Ľ��̺�Ϊ��" << PID << endl;
	fileout.close();
	ReleaseSemaphore(history_mutex, 1, NULL);
}

//����PID��ֹ��������		YQK
void KillProcess()
{
	int lineX = 0, lineY = 0;
	int flag = 0;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);	//���վ��
	HANDLE hProcess = NULL;													//���̾��
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot ����ʧ��.\n");
		exit(0);
	}
	cout << "�������ID���������̣�" << endl;
	int PID = 0;
	cin >> PID;
	hProcess = GetProcessHandle(PID);
	if (hProcess == NULL)
	{
		cout << "���̾����ȡʧ�ܣ�" << endl;
		//exit(0);
	}
	else
	{
		DWORD ret = TerminateProcess(hProcess, 0);
		if (ret == 0)
		{
			cout << "ɱ���̳��ִ���" << endl;
			cout << GetLastError << endl;
		}
		else
		{
			cout << "�����Ѿ�ɱ��!" << endl;
		}
	}

	//Sleep(1000);
	CloseHandle(hProcess);								//�رս���
	fstream fileout;
	WaitForSingleObject(history_mutex, INFINITE);
	fileout.open(path_history, ios::out | ios::ate | ios::app);
	getTime();
	fileout << timeBuffer << "  ��ֹ�Ľ��̺�Ϊ��" << PID << endl;
	fileout.close();
	ReleaseSemaphore(history_mutex, 1, NULL);

}

//�鿴�ڴ���Ϣ	YQK
void ShowMemory(void)
{
	MEMORYSTATUSEX mem_statusex;
	mem_statusex.dwLength = sizeof(mem_statusex);
	//��ȡϵͳ�ڴ���Ϣ  
	GlobalMemoryStatusEx(&mem_statusex);
	cout << "�ڴ���Ϣ��" << endl;
	cout << "�����ڴ��ʹ����Ϊ��" << mem_statusex.dwMemoryLoad << "%" << endl;
	cout << "�����ڴ��������Ϊ��" << (float)mem_statusex.ullTotalPhys / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "���õ������ڴ�Ϊ��" << (float)mem_statusex.ullAvailPhys / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "�ܵĽ����ļ�Ϊ��" << (float)mem_statusex.ullTotalPageFile / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "���õĽ����ļ�Ϊ��" << (float)mem_statusex.ullAvailPageFile / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "�����ڴ��������Ϊ��" << (float)mem_statusex.ullTotalVirtual / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "���õ������ڴ�Ϊ��" << (float)mem_statusex.ullAvailVirtual / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "�����ֶε�����Ϊ��" << mem_statusex.ullAvailExtendedVirtual << "Byte" << endl;
}

//WIN API�õ���ǰconsole ��(x,y)    YQK
void console_gotoxy(int x, int y)
{
	// �õ���ǰconsole�ľ��
	HANDLE hc = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD cursor = { x, y };
	//�����µ�cursorλ�� 
	SetConsoleCursorPosition(hc, cursor);
}
//WIN API���õ�ǰconsole ��(x, y)    YQK
void console_getxy(int& x, int& y)
{
	// �õ���ǰconsole�ľ��
	HANDLE hc = GetStdHandle(STD_OUTPUT_HANDLE);
	// ��Ļ��������Ϣ 
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	//�õ���Ӧ��������Ϣ
	GetConsoleScreenBufferInfo(hc, &csbi);
	x = csbi.dwCursorPosition.X;
	y = csbi.dwCursorPosition.Y;
}

//�鿴ϵͳ��Ϣ    YQK
void ShowSystem()
{
	int lineX = 0, lineY = 0;
	int flag = 0;
	PERFORMANCE_INFORMATION perfor_info;
	perfor_info.cb = sizeof(perfor_info);
	while (!kbhit())
	{
		//ʹ��win api���ƻ�����ˢ�����
		if (flag == 0)
		{
			console_getxy(lineX, lineY);
			flag++;
		}
		else
		{
			console_gotoxy(lineX, lineY);
		}
		GetPerformanceInfo(&perfor_info, sizeof(perfor_info));
		cout << "��ҳ��С: " << perfor_info.PageSize / 1024 << "KB" << endl;
		cout << "ϵͳ�ύ��ҳ������: " << perfor_info.CommitTotal << " Pages" << endl;
		cout << "ϵͳ�ύ��ҳ������: " << perfor_info.CommitLimit << " Pages" << endl;
		cout << "ϵͳ�ύ��ҳ���ֵ: " << perfor_info.CommitPeak << " Pages" << endl;
		cout << "��ҳ����������ڴ�����: " << perfor_info.PhysicalTotal << " Pages" << endl;
		cout << "��ҳ����������ڴ������: " << perfor_info.PhysicalAvailable << " Pages" << endl;
		cout << "ϵͳ�����ڴ�ռ��: " << (perfor_info.PhysicalTotal - perfor_info.PhysicalAvailable)*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "ϵͳ�����ڴ����: " << perfor_info.PhysicalAvailable*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "ϵͳ�����ڴ�����: " << perfor_info.PhysicalTotal*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "ϵͳ���������� " << perfor_info.PhysicalAvailable << " Pages" << endl;
		cout << "ϵͳ�ں��ڴ�ռ��ҳ�������� " << perfor_info.KernelTotal << " Pages" << endl;
		cout << "ϵͳ�ں��ڴ�ռ�ݷ�ҳҳ������ " << perfor_info.KernelNonpaged << " Pages" << endl;
		cout << "ϵͳ�ں��ڴ�ռ�ݲ���ҳҳ������ " << perfor_info.KernelPaged << " Pages" << endl;
		cout << "ϵͳ��������� " << perfor_info.HandleCount << " Pages" << endl;
		cout << "ϵͳ���������� " << perfor_info.ProcessCount << " Pages" << endl;
		cout << "ϵͳ�߳������� " << perfor_info.ThreadCount << " Pages" << endl;
		if (getchar())
			break;
	}
}

//�鿴�洢����Ϣ    YQK
void ShowMemoryUnit(void)
{
	PERFORMANCE_INFORMATION pi;
	pi.cb = sizeof(pi);
	GetPerformanceInfo(&pi, sizeof(pi));
	cout << "�洢����Ϣ" << endl;
	cout << "�ṹ��Ĵ�СΪ: " << pi.cb << "B" << endl;
	cout << "ϵͳ��ǰ�ύ��ҳ������: " << pi.CommitTotal << endl;
	cout << "ϵͳ��ǰ���ύ�����ҳ������: " << pi.CommitLimit << endl;
	cout << "ϵͳ��ʷ�ύҳ���ֵ: " << pi.CommitPeak << endl;
	cout << "��ҳ������������ڴ�: " << pi.PhysicalTotal << endl;
	cout << "��ǰ���õ������ڴ�Ϊ: " << pi.PhysicalAvailable << endl;
	cout << "ϵͳCache������Ϊ: " << pi.SystemCache << endl;
	cout << "�ڴ�����(��ҳ)Ϊ: " << pi.KernelTotal << endl;
	cout << "��ҳ�صĴ�СΪ: " << pi.KernelPaged << endl;
	cout << "�Ƿ�ҳ�صĴ�СΪ: " << pi.KernelNonpaged << endl;
	cout << "ҳ�Ĵ�СΪ: " << pi.PageSize << endl;
	cout << "�򿪵ľ������Ϊ: " << pi.HandleCount << endl;
	cout << "���̸���Ϊ: " << pi.ProcessCount << endl;
	cout << "�̸߳���Ϊ: " << pi.ThreadCount << endl;
}

//���Ƶ����ļ�    ZYH
void CopyFile(char * fsource, char * ftarget)           //����Ϊ������ͬ·������ͬ�ļ���
{
	WIN32_FIND_DATA lpfindfiledata;

	HANDLE hfind = FindFirstFile(fsource, &lpfindfiledata);    //�ҵ�Դ�ļ�

	HANDLE hsource = CreateFile(fsource,        //ָ����ļ�����ָ��  
		GENERIC_READ | GENERIC_WRITE,           //ָ������ķ�������(ͨ������д���Ϸ��ʿ���ʵ���ļ��Ķ�д)  
		FILE_SHARE_READ,                        //���������̹����ļ��ķ�ʽ  
		NULL,                                   //ָ��ȫ���Ե�ָ��(NULL��ʾĬ�ϰ�ȫ����)  
		OPEN_ALWAYS,                            //ָ����δ����ļ�(OPEN_ALWAYSΪ����ļ���������ļ������򴴽��µ��ļ�)  
		FILE_ATTRIBUTE_NORMAL,                  //ָ���ļ������Ժͱ�־(FILE_ATTRIBUTE_NORMAL����ΪĬ������)  
		NULL);                                  //���ڸ����ļ����  

	HANDLE htarget = CreateFile(ftarget,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	LONG size = lpfindfiledata.nFileSizeLow - lpfindfiledata.nFileSizeHigh;   //Դ�ļ��Ĵ�С  

	DWORD wordbit;            //һ��һ�ִ��뻺����


	int *BUFFER = new int[size];         	//����������


	ReadFile(hsource,         //ָ��Ҫ�����ļ����  
		BUFFER,               //ָ���Ŵ��ļ��������ݵĻ������ĵ�ַָ��  
		size,                 //Ҫ���ļ������ֽ���  
		&wordbit,             //���ʵ�ʴ��ļ��ж����ֽ����ı�����ַ  
		NULL);                //ͬ����ʽ������ΪNULL  

	WriteFile(htarget,        //ָ��Ҫд���ļ�����ľ��  
		BUFFER,               //ָ��Ҫд���ļ������ݻ�����ָ��  
		size,                 //Ҫд���ļ����ֽ���  
		&wordbit,             //ʵ��д����ֽ���  
		NULL);                //ͬ����ʽ������ΪNULL  

	FindClose(hfind);
	CloseHandle(hsource);
	CloseHandle(htarget);
}

//�����������ļ��е����ļ��������ļ�    ZYH
void mycp(char * fsource, char * ftarget)
{
	WIN32_FIND_DATA lpfindfiledata;
	char source[buf_size];
	char target[buf_size];
	lstrcpy(source, fsource);
	lstrcpy(target, ftarget);
	lstrcat(source, "\\*");                 //���������ļ�
	lstrcat(target, "\\");
	HANDLE hfind = FindFirstFile(source, &lpfindfiledata);    //���ҵ�һ��Դ�ļ�
	if (hfind != INVALID_HANDLE_VALUE)
	{
		while (FindNextFile(hfind, &lpfindfiledata) != 0)    //������ǰĿ¼�µ������ļ�
		{
			if ((lpfindfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)     //����ҵ���һ��Ŀ¼ 
			{
				if ((strcmp(lpfindfiledata.cFileName, ".") != 0) && (strcmp(lpfindfiledata.cFileName, "..") != 0))  //�����ǵ�ǰĿ¼��Ŀ¼
				{
					memset(source, '0', sizeof(source));
					lstrcpy(source, fsource);
					lstrcat(source, "\\");
					lstrcat(source, lpfindfiledata.cFileName);    //��source��дΪ��ǰѭ��Ŀ¼\\���ҵ�����Ŀ¼
					lstrcat(target, lpfindfiledata.cFileName);    //��target��дΪ��ǰѭ��Ŀ¼\\���ҵ�����Ŀ¼
					CreateDirectory(target, NULL);                //��Ŀ��Ŀ¼�´�����Ŀ¼  
					mycp(source, target);                         //����ݹ飬��ʼ������Ŀ¼        
					lstrcpy(source, fsource);                     //�ڸ������ļ��н����󣬻ص���ǰѭ��Ŀ¼������������һ���ļ����ļ���
					lstrcat(source, "\\");
					lstrcpy(target, ftarget);
					lstrcat(target, "\\");
				}
			}
			else                            //����ҵ���һ���ļ�
			{
				memset(source, '0', sizeof(source));
				lstrcpy(source, fsource);
				lstrcat(source, "\\");
				lstrcat(source, lpfindfiledata.cFileName);
				lstrcat(target, lpfindfiledata.cFileName);
				CopyFile(source, target);                  //�����ļ����ƺ���  
				lstrcpy(source, fsource);
				lstrcat(source, "\\");
				lstrcpy(target, ftarget);
				lstrcat(target, "\\");
			}
		}
	}
	else
	{
		printf("����ָ���ļ�·�����ļ�ʧ��!\n");
	}
}

//�ƶ��ļ���������   ZYH
void myRename(char * oldname, char * newname)
{
	if (!MoveFile(oldname/*����*/, newname/*����*/))
	{
		printf("�ƶ��ļ���������\n", GetLastError());
	}
	else
	{
		printf("�ƶ��ļ��ɹ�\n");
	}
}

//ɾ���ļ�    ZYH
void myDelete(char * fileName)
{
	if (!DeleteFile(fileName))
	{
		printf("ɾ���ļ���������\n");
	}
	else
	{
		printf("ɾ���ļ��ɹ�\n");
	}
}

//�����������������������switch���ò�ͬ�Ĺ��ܺ���    ZYH
int main(void)
{
	fstream fileout;
	WIN32_FIND_DATA lpfindfiledata;
	char source[100];
	char target[100];

	//����������У�����0ʱ�˳�
	while (1)
	{
		//��������
		cout << endl;
		cout << "==========================================================================" << endl << endl;
		cout << "\t\t\t����������ִ����Ӧ���ܣ�" << endl;
		cout << "\t1�������ļ�" << endl;
		cout << "\t2�������ļ�" << endl;
		cout << "\t3���ƶ��ļ�" << endl;
		cout << "\t4��ɾ���ļ�" << endl;
		cout << "\t5����ѯ���н���" << endl;
		cout << "\t6����ѯָ������" << endl;
		cout << "\t7����ָֹ������" << endl;
		cout << "\t8����ȡ�ڴ���Ϣ" << endl;
		cout << "\t9����ȡϵͳ��Ϣ" << endl;
		cout << "\t10����ȡ�洢����Ϣ" << endl;
		//������
		cout << "\t11�������ʷ��¼" << endl;
		cout << "\t0���˳�" << endl << endl;
		cout << "===========================All Rights Reserved=============================";
		cout << endl;
		int x;
		cin >> x;

		//����0ʱ�˳�����
		if (x == 0)
		{
			writeHistory(x);
			createBackup(path_history, path_backup);
			break;
		}
		else
			switch (x)
			{
			//ִ�в����ļ��Ĺ���
			case 1:
				writeHistory(x);

				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);
				ZeroMemory(&pi, sizeof(pi));

				if (!CreateProcess(NULL,   // No module name (use command line)
					"FileSearch.exe",             // Command line
					NULL,           // Process handle not inheritable
					NULL,           // Thread handle not inheritable
					FALSE,          // Set handle inheritance to FALSE
					0,              // No creation flags
					NULL,           // Use parent's environment block
					NULL,           // Use parent's starting directory 
					&si,            // Pointer to STARTUPINFO structure
					&pi)           // Pointer to PROCESS_INFORMATION structure
					)
				{
					printf("CreateProcess failed.\n");
					continue;
				}
				cout << "the child has been created.\n";
				WaitForSingleObject(pi.hProcess, INFINITE);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				break;

			//ִ�и����ļ��Ĺ���
			case 2:
				writeHistory(x);
				cout << "������ϣ�����Ƶ�Դ�ļ���Դ�ļ��У�" << endl;
				cin >> source;
				cout << "������ϣ�����Ƶ���Ŀ��·����" << endl;
				cin >> target;
				if (FindFirstFile(source, &lpfindfiledata) == INVALID_HANDLE_VALUE)
				{
					printf("����Դ�ļ�·��ʧ��!\n");
				}
				if (FindFirstFile(target, &lpfindfiledata) == INVALID_HANDLE_VALUE)
				{
					CreateDirectory(target, NULL);//ΪĿ���ļ�����Ŀ¼  
				}
				mycp(source, target);
				cout << "���Ƴɹ���" << endl;

				WaitForSingleObject(history_mutex, INFINITE);
				fileout.open(path_history, ios::out | ios::ate | ios::app);
				getTime();
				fileout << timeBuffer << "  ���Ƶ�Դ�ļ���Դ�ļ���Ϊ��" << source << endl;
				fileout << timeBuffer << "���Ƶ���Ŀ��·��Ϊ��" << target << endl;
				fileout.close();
				ReleaseSemaphore(history_mutex, 1, NULL);
				break;

			//ִ���ƶ��ļ����������Ĺ���
			case 3:
				writeHistory(x);
				cout << "������ϣ���ƶ����ļ���" << endl;
				cin >> source;
				cout << "�������ƶ�Ŀ��·�����ļ��������֣�" << endl;
				cin >> target;
				myRename(source, target);

				WaitForSingleObject(history_mutex, INFINITE);
				fileout.open(path_history, ios::out | ios::ate | ios::app);
				getTime();
				fileout << timeBuffer << "  �ƶ�ǰ���ļ�Ϊ��" << source << endl;
				fileout << timeBuffer << "�ƶ�����ļ�Ϊ��" << target << endl;
				fileout.close();
				ReleaseSemaphore(history_mutex, 1, NULL);
				break;

			//ִ��ɾ���ļ��Ĺ���
			case 4:
				writeHistory(x);
				cout << "������ϣ��ɾ�����ļ�����" << endl;
				cin >> source;
				myDelete(source);

				WaitForSingleObject(history_mutex, INFINITE);
				fileout.open(path_history, ios::out | ios::ate | ios::app);
				getTime();
				fileout << timeBuffer << "  ɾ�����ļ�Ϊ��" << source << endl;
				fileout.close();
				ReleaseSemaphore(history_mutex, 1, NULL);
				break;

			//ִ�в�ѯ���н�����Ϣ�Ĺ���
			case 5:
				writeHistory(x);
				ShowAllProcess(-1);
				break;

			//ִ�в�ѯ����������Ϣ�Ĺ���
			case 6:
				writeHistory(x);
				QuerySingleProcess();
				break;

			//ִ����ֹ���̵Ĺ���
			case 7:
				writeHistory(x);
				KillProcess();
				break;

			//ִ�в鿴�ڴ���Ϣ�Ĺ���
			case 8:
				writeHistory(x);
				ShowMemory();
				break;

			//ִ�в鿴ϵͳ��Ϣ�Ĺ���
			case 9:
				writeHistory(x);
				ShowSystem();
				break;

			//ִ�в鿴�洢����Ϣ�Ĺ���
			case 10:
				writeHistory(x);
				ShowMemoryUnit();
				break;

			//ִ�������ʷ��¼�Ĺ���
			case 11:
				writeHistory(x);
				readHistory();
				break;

			//����Ƿ������
			default:
				cout << "������������޷�ʶ��" << endl;
				break;
			}
	}
}
