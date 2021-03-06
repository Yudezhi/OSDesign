// win_memory.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include<windows.h>
#include <psapi.h>  //getperformence
#include<iostream>
#include<shlwapi.h>
#include<iomanip>
#include<tchar.h>
#include<tlhelp32.h>
#include<cstdlib> 
#include<CString>
#include <ATLComTime.h> //COledatetime

#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"shlwapi.lib")

using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//the protection flag   //292
inline bool TestSet(DWORD dwTarget, DWORD dwMask)
{
	return ((dwTarget &dwMask) == dwMask);
}

#define SHOWMASK(dwTarget,type)\
if (TestSet(dwTarget, PAGE_##type))\
{std::cout << "," << #type; }

//显示保护标记，表示允许应用程序对内存访问的类型  //page 292
DWORD GetKernelModePercentage(const FILETIME& ftKernel, const FILETIME& ftUser)
{
	ULONGLONG qwKernel = (((ULONGLONG)ftKernel.dwHighDateTime) << 32) + ftKernel.dwLowDateTime;
	ULONGLONG qwUser = (((ULONGLONG)ftUser.dwHighDateTime) << 32) + ftUser.dwLowDateTime;

	ULONGLONG qwTotal = qwKernel + qwUser;
	DWORD dwPct = 0;
	if (qwTotal != 0)
		dwPct = (DWORD)(((ULONGLONG)100 * qwKernel) / qwTotal);
	return(dwPct);
}

//定义页面保护方式  //page 292
void ShowProtection(DWORD dwTarget)
{
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

//查看用户输入进程的内存使用信息
void WalkVM(HANDLE hProcess)
{
	printf("==============================Parent.exe===================================\n");
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(si));
	GetSystemInfo(&si);

	MEMORY_BASIC_INFORMATION mbi;  //进程虚拟内存的基本信息结构
	ZeroMemory(&mbi, sizeof(mbi));	//分配缓冲

	//从进程的起始地址开始循环整个地址空间
	LPCVOID pBlock = (LPVOID)si.lpMinimumApplicationAddress;
	while (pBlock < si.lpMaximumApplicationAddress)
	{
		//从当前地址开始获得下一块内存信息
		if (VirtualQueryEx(
			hProcess,  // connected process  P293 
			pBlock, //start position 
			&mbi,//buffer 
			sizeof(mbi)) == sizeof(mbi))  //ensure the length 
		{
			//块结束地址
			LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;
			TCHAR szSize[MAX_PATH];
			//块长度
			StrFormatByteSize(mbi.RegionSize, szSize, MAX_PATH);

			//打印块地址和长度
			std::cout.fill('0');
			std::cout << hex << setw(8) << (DWORD)pBlock
				<< "-" << hex << setw(8) << (DWORD)pEnd
				<< (_tcslen(szSize) == 7 ? "(" : "(") << szSize << ")";

			//块的状态
			switch (mbi.State)
			{
			case MEM_COMMIT:printf ("|-Committed-|"); break;
			case MEM_FREE:printf   ("|---Free----|"); break;
			case MEM_RESERVE:printf("|--Reserved-|"); break;
			}

			//块的保护
			if (mbi.Protect == 0 && mbi.State != MEM_FREE)
			{
				mbi.Protect = PAGE_READONLY;
			}
			ShowProtection(mbi.Protect);

			//块的类型
			switch (mbi.Type)
			{
			case MEM_IMAGE:printf   (" ||Image  "); break;
			case MEM_MAPPED:printf  (" ||Mapped "); break;
			case MEM_PRIVATE:printf (" ||Private"); break;
			}

			//除去路径并显示
			TCHAR szFilename[MAX_PATH];
			if (GetModuleFileName(
				(HMODULE)pBlock,//handle of virtual memory 
				szFilename,  //filename 
				MAX_PATH) > 0) //the length of buffer really
			{
				PathStripPath(szFilename);
				std::cout<<"  || Module:" <<szFilename;

			}
			printf("\n");
			//下一块
			pBlock = pEnd;
		}
	}
	getchar();  //pause
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//打印进程列表以及相关信息
void ShowProcessList()
{
	//系统当前所有进程的快照
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //ID  flag

	PROCESSENTRY32 pe;   //a struct named tagPROCESSENTRY32W  ,include some dword information,a struct to store process indeed
	ZeroMemory(&pe, sizeof(pe));
	pe.dwSize = sizeof(pe);

	//快照中第一个进程
	std::cout<<"==============================进程列表=============================================\n";
	std::cout << "NUM" << "\t  PID" << "\t\tName" << "\t\t\t\t\t\tUsge(kernel) %"<<"\t\t TIME" << endl;
	std::cout << "---------------------------------------------------------------------------------\n";
	BOOL bMore = Process32First(hSnapshot, &pe);  //handle , out_handle   return !0 to be fun
	int count = 1;

	//循环至快照没有下一个进程
	while (bMore)
	{
		//打开当前进程句柄
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);  //desired Access , inherit, ID ->return !0
		if (hProcess != NULL)
		{
			
			printf("%02d", count++);

			//当前进程的时间
			FILETIME ftCreation, ftKernelMode, ftUserMode, ftExit;
			GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernelMode, &ftUserMode);


			//核心态下消耗的时间百分比
			DWORD dwPctKernel = GetKernelModePercentage(ftKernelMode, ftUserMode);
			//打印进程号，进程所在的程序名，核心态下消耗的时间占总时间百分比
			printf("\t%6ld", pe.th32ProcessID);
			//wcout << "\t"<<pe.th32ProcessID;

			printf("\t\t%-40S", pe.szExeFile);
			//wcout << "\t"<<pe.szExeFile;

			wcout << "\t\t"<< dwPctKernel << "% \t\t";
			//std::cout << &ftCreation << endl;

			//COleDateTime dt(ftCreation);

			//CString s = dt.Format("%Y-%m-%d %H:%M:%S");
			//CString s = dt.Format(ftCreation);
			SYSTEMTIME stCreation;
			FileTimeToSystemTime(&ftCreation, &stCreation);
			printf("%d-%02d-%02d  %02d:%02d:%02d \n", stCreation.wYear, stCreation.wMonth, stCreation.wDay, 
											stCreation.wHour+8, stCreation.wMinute, stCreation.wSecond);

			//std::cout << " 虚拟内存的大小为:" << (float)pmc.WorkingSetSize / 1024 << "KB" << endl;

			CloseHandle(hProcess);
		}
		//快照中的下一个进程
		bMore = Process32Next(hSnapshot, &pe);
	}

}

//打印内存状态
void ShowMemory()
{
	//获取当前系统信息
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(si));
	GetSystemInfo(&si);

	//获取内存的页面大小信息
	//TCHAR szPageSize[MAX_PATH];
	//StrFormatByteSize(si.dwPageSize, szPageSize, MAX_PATH);  //_out   szPageSize

	//获取系统的内存信息
	MEMORYSTATUSEX lpBuffer;  //  a struct for sysinfo
	lpBuffer.dwLength = sizeof(lpBuffer);
	GlobalMemoryStatusEx(&lpBuffer);
	printf("==============================内存信息===================================\n");
	//打印内存占用百分比
	printf("Physics Memory Use Percent :%ld%%\n", lpBuffer.dwMemoryLoad);


	//打印物理内存总量
	printf("Physics Memory Totally: %.2fGB.\n", (float)lpBuffer.ullTotalPhys / 1024 / 1024 / 1024);


	//打印物理内存剩余量
	printf("The free Of Physics Memory: %.2fGB.\n", (float)lpBuffer.ullAvailPhys / 1024 / 1024 / 1024);
	
	//打印分页文件总量
	printf("总的交换文件为:%.2fGB.\n", (float)lpBuffer.ullTotalPageFile / 1024 / 1024 / 1024);

	
	//打印分页文件剩余量
	
	printf("可用的交换文件为：%.2fGB.\n", (float)lpBuffer.ullAvailPageFile / 1024 / 1024 / 1024);


	//打印虚拟内存总量
	printf("虚拟内存的总容量为：%.2fGB.\n", (float)lpBuffer.ullTotalVirtual / 1024 / 1024 / 1024);

	//打印虚拟内存剩余量
	printf("可用的虚拟内存为：%.2fGB.\n", (float)lpBuffer.ullAvailVirtual / 1024 / 1024 / 1024);

	printf("内存页的大小为：%dKB.\n", (int)si.dwPageSize / 1024);
	//打印虚拟内存页面大小
	//std::cout << "Virtual memory page size:       " << szPageSize << endl;

	std::cout.fill('0');
	//打印最低地址和最高地址
	std::cout << "进程可用地址空间的最小内存地址：" << si.lpMinimumApplicationAddress <<"H"<< endl;
	std::cout << "进程可用地址空间的最大内存地址:" << si.lpMaximumApplicationAddress <<"H"<< endl;
	std::cout << "能够保留地址空间区域的最小单位: " << si.dwAllocationGranularity / 1024 << "KB" << endl;
	printf("------------------------------------------------------\n");


	//=======================================================================================
	PERFORMANCE_INFORMATION pi;
	pi.cb = sizeof(pi);
	GetPerformanceInfo(&pi, sizeof(pi));
	printf("----------------系统的存储器当前情况------------------\n");
	cout << "结构体的大小为: " << pi.cb << "B" << endl;
	cout << "系统当前提交的页面总数: " << pi.CommitTotal << endl;
	cout << "系统当前可提交的最大页面总数: " << pi.CommitLimit << endl;
	cout << "系统历史提交页面峰值: " << pi.CommitPeak << endl;
	cout << "按页分配的总物理内存: " << pi.PhysicalTotal << endl;
	cout << "当前可用的物理内存为: " << pi.PhysicalAvailable << endl;
	cout << "系统Cache的容量为: " << pi.SystemCache << endl;
	cout << "内存总量(按页)为: " << pi.KernelTotal << endl;
	cout << "分页池的大小为: " << pi.KernelPaged << endl;
	cout << "非分页池的大小为: " << pi.KernelNonpaged << endl;
	cout << "页的大小为: " << pi.PageSize << endl;
	cout << "打开的句柄个数为: " << pi.HandleCount << endl;
	cout << "进程个数为: " << pi.ProcessCount << endl;
	cout << "线程个数为: " << pi.ThreadCount << endl;
	printf("------------------------------------------------------\n");


}

void main(int argc, char *argv[])
{
	int num;
	HANDLE processhd;
	ShowMemory();       //打印内存状态
	ShowProcessList();    //打印进程列表
	std::cout << "please input the id of process:" << endl;    //提示用户输入想要查看的进程的进程号
	cin >> num;
	processhd = OpenProcess(PROCESS_ALL_ACCESS, false, num);  //进程号对应的进程句柄
	WalkVM(processhd);    //查看用户输入进程的内存使用信息

	getchar();
}
