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
#include "writeHistory.h"    //写入历史记录与完成备份相关的头文件    ZYH

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma warning(disable: 4996)

#define WIDTH 10
#define DIV (1024*1024)
#define M  100
#define buf_size 4096  

using namespace std;

//显示保护标记，该标记表示允许应用程序对内存进行访问的类型    YQK
inline bool TestSet(DWORD dwTarget, DWORD dwMask)
{
	return ((dwTarget &dwMask) == dwMask);
}

//查看进程信息需要    YQK
#define SHOWMASK(dwTarget,type) if(TestSet(dwTarget,PAGE_##type)){cout << "," << #type;}  

HANDLE GetProcessHandle(int ProcessID)
{
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
}

//查看所有进程的信息    YQK
void ShowAllProcess(int pid)
{
	PROCESSENTRY32 pe32;										//存储进程信息
	pe32.dwSize = sizeof(pe32);									//在使用这个结构前，先设置它的大小
	PROCESS_MEMORY_COUNTERS ppsmemCounter;						//struct,存储进程内存的使用信息，便于用函数GetProcessMemoryInfo获取进程的相关信息
	ppsmemCounter.cb = sizeof(ppsmemCounter);					//初始化大小
	HANDLE hProcessSnap;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);			//快照句柄
	HANDLE hProcess;//进程句柄
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		printf("创建进程快照失败.\n");
		exit(0);
	}
	//遍历进程快照，轮流显示每个进程的信息
	BOOL bMore = Process32First(hProcessSnap, &pe32);						 //获取系统快照第一个进程的信息，结果返回到pe32结构里
	printf("进程的工作集信息:\n");
	while (bMore) {
		if (pid != -1)
		{
			if (pid == pe32.th32ProcessID)
			{
				//wcout << "进程名称:" << pe32.szExeFile << endl;//进程信息（存储于pe32中）
				printf("进程名称：%ws\n", pe32.szExeFile);
				cout << "进程ID:" << pe32.th32ProcessID << endl;
				cout << "线程数:" << pe32.cntThreads << endl;
				hProcess = GetProcessHandle(pe32.th32ProcessID);
				GetProcessMemoryInfo(hProcess, &ppsmemCounter, sizeof(ppsmemCounter));//进程内存使用信息（存储于ppsmemCounter中）
				cout << "已提交:" << ppsmemCounter.PagefileUsage / 1024 << " KB" << endl;
				cout << "工作集:" << ppsmemCounter.WorkingSetSize / 1024 << " KB" << endl;
				cout << "工作集峰值:" << ppsmemCounter.PeakWorkingSetSize / 1024 << " KB" << endl;
			}
			bMore = Process32Next(hProcessSnap, &pe32);						//获取系统快照下一个进程信息
		}
		else
		{
			//wcout << "进程名称:" << pe32.szExeFile << endl;//进程信息（存储于pe32中）
			printf("进程名称：%ws \n", pe32.szExeFile);
			cout << "进程ID:" << pe32.th32ProcessID << endl;
			cout << "线程数:" << pe32.cntThreads << endl;
			hProcess = GetProcessHandle(pe32.th32ProcessID);
			GetProcessMemoryInfo(hProcess, &ppsmemCounter, sizeof(ppsmemCounter));	//进程内存使用信息（存储于ppsmemCounter中）
			cout << "已提交:" << ppsmemCounter.PagefileUsage / 1024 << " KB" << endl;
			cout << "工作集:" << ppsmemCounter.WorkingSetSize / 1024 << " KB" << endl;
			cout << "工作集峰值:" << ppsmemCounter.PeakWorkingSetSize / 1024 << " KB" << endl;
			bMore = Process32Next(hProcessSnap, &pe32);						//获取系统快照下一个进程信息
		}
	}
	CloseHandle(hProcessSnap);												//关闭快照
}


//显示保护标记    YQK
void ShowProtection(DWORD dwTarget)
{//定义的页面保护类型  
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

//遍历程序的地址空间    YQK
void WalkVM(HANDLE hProcess)
{
	SYSTEM_INFO si; //系统信息结构  
	ZeroMemory(&si, sizeof(si));    //初始化  
	GetSystemInfo(&si); //获得系统信息  

	MEMORY_BASIC_INFORMATION mbi;   //进程虚拟内存空间的基本信息结构  
	ZeroMemory(&mbi, sizeof(mbi));  //分配缓冲区，用于保存信息  

									//循环整个应用程序地址空间  
	LPCVOID pBlock = (LPVOID)si.lpMinimumApplicationAddress;
	while (pBlock < si.lpMaximumApplicationAddress)
	{
		//获得下一个虚拟内存块的信息  
		if (VirtualQueryEx(
			hProcess,   //相关的进程  
			pBlock,     //开始位置  
			&mbi,       //缓冲区  
			sizeof(mbi)) == sizeof(mbi))    //长度的确认，如果失败返回0  
		{
			//计算块的结尾及其长度  
			LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;
			TCHAR szSize[MAX_PATH];
			//将数字转换成字符串  
			StrFormatByteSize(mbi.RegionSize, szSize, MAX_PATH);

			//显示块地址和长度  
			cout.fill('0');
			cout << hex << setw(8) << (DWORD)pBlock << "-" << hex << setw(8) << (DWORD)pEnd << (_tcslen(szSize) == 7 ? "(" : "(") << szSize << ")";

			//显示块的状态  
			switch (mbi.State)
			{
			case MEM_COMMIT:
				printf("已提交");
				break;
			case MEM_FREE:
				printf("空闲");
				break;
			case MEM_RESERVE:
				printf("已预留");
				break;
			}

			//显示保护  
			if (mbi.Protect == 0 && mbi.State != MEM_FREE)
			{
				mbi.Protect = PAGE_READONLY;
			}
			ShowProtection(mbi.Protect);

			//显示类型  
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

			//检验可执行的映像  
			TCHAR szFilename[MAX_PATH];
			if (GetModuleFileName(
				(HMODULE)pBlock,            //实际虚拟内存的模块句柄  
				szFilename,                 //完全指定的文件名称  
				MAX_PATH) > 0)               //实际使用的缓冲区长度  
			{
				//除去路径并显示  
				PathStripPath(szFilename);
				printf(", Module:%s", szFilename);
			}

			printf("\n");
			//移动块指针以获得下一个块  
			pBlock = pEnd;
		}
	}
}

//输入PID查询单个进程信息    YQK
void QuerySingleProcess()
{
	int lineX = 0, lineY = 0;
	int flag = 0;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);	//快照句柄
	HANDLE hProcess = NULL;													//进程句柄
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot 调用失败.\n");
		exit(0);
	}
	cout << "输入进程ID，查询进程的内存分布空间：" << endl;
	int PID = 0;
	cin >> PID;
	hProcess = GetProcessHandle(PID);
	if (hProcess == NULL)
	{
		cout << "进程不存在！ " << endl;
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
	CloseHandle(hProcess);													//关闭进程

	fstream fileout;
	WaitForSingleObject(history_mutex, INFINITE);
	fileout.open(path_history, ios::out | ios::ate | ios::app);
	getTime();
	fileout << timeBuffer << "  查询的进程号为：" << PID << endl;
	fileout.close();
	ReleaseSemaphore(history_mutex, 1, NULL);
}

//输入PID终止单个进程		YQK
void KillProcess()
{
	int lineX = 0, lineY = 0;
	int flag = 0;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);	//快照句柄
	HANDLE hProcess = NULL;													//进程句柄
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot 调用失败.\n");
		exit(0);
	}
	cout << "输入进程ID，结束进程：" << endl;
	int PID = 0;
	cin >> PID;
	hProcess = GetProcessHandle(PID);
	if (hProcess == NULL)
	{
		cout << "进程句柄获取失败！" << endl;
		//exit(0);
	}
	else
	{
		DWORD ret = TerminateProcess(hProcess, 0);
		if (ret == 0)
		{
			cout << "杀进程出现错误！" << endl;
			cout << GetLastError << endl;
		}
		else
		{
			cout << "进程已经杀死!" << endl;
		}
	}

	//Sleep(1000);
	CloseHandle(hProcess);								//关闭进程
	fstream fileout;
	WaitForSingleObject(history_mutex, INFINITE);
	fileout.open(path_history, ios::out | ios::ate | ios::app);
	getTime();
	fileout << timeBuffer << "  终止的进程号为：" << PID << endl;
	fileout.close();
	ReleaseSemaphore(history_mutex, 1, NULL);

}

//查看内存信息	YQK
void ShowMemory(void)
{
	MEMORYSTATUSEX mem_statusex;
	mem_statusex.dwLength = sizeof(mem_statusex);
	//获取系统内存信息  
	GlobalMemoryStatusEx(&mem_statusex);
	cout << "内存信息：" << endl;
	cout << "物理内存的使用率为：" << mem_statusex.dwMemoryLoad << "%" << endl;
	cout << "物理内存的总容量为：" << (float)mem_statusex.ullTotalPhys / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "可用的物理内存为：" << (float)mem_statusex.ullAvailPhys / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "总的交换文件为：" << (float)mem_statusex.ullTotalPageFile / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "可用的交换文件为：" << (float)mem_statusex.ullAvailPageFile / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "虚拟内存的总容量为：" << (float)mem_statusex.ullTotalVirtual / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "可用的虚拟内存为：" << (float)mem_statusex.ullAvailVirtual / 1024 / 1024 / 1024 << "GB" << endl;
	cout << "保留字段的容量为：" << mem_statusex.ullAvailExtendedVirtual << "Byte" << endl;
}

//WIN API得到当前console 的(x,y)    YQK
void console_gotoxy(int x, int y)
{
	// 得到当前console的句柄
	HANDLE hc = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD cursor = { x, y };
	//设置新的cursor位置 
	SetConsoleCursorPosition(hc, cursor);
}
//WIN API设置当前console 的(x, y)    YQK
void console_getxy(int& x, int& y)
{
	// 得到当前console的句柄
	HANDLE hc = GetStdHandle(STD_OUTPUT_HANDLE);
	// 屏幕缓冲区信息 
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	//得到相应缓冲区信息
	GetConsoleScreenBufferInfo(hc, &csbi);
	x = csbi.dwCursorPosition.X;
	y = csbi.dwCursorPosition.Y;
}

//查看系统信息    YQK
void ShowSystem()
{
	int lineX = 0, lineY = 0;
	int flag = 0;
	PERFORMANCE_INFORMATION perfor_info;
	perfor_info.cb = sizeof(perfor_info);
	while (!kbhit())
	{
		//使用win api控制缓冲区刷新输出
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
		cout << "分页大小: " << perfor_info.PageSize / 1024 << "KB" << endl;
		cout << "系统提交的页面总数: " << perfor_info.CommitTotal << " Pages" << endl;
		cout << "系统提交的页面限制: " << perfor_info.CommitLimit << " Pages" << endl;
		cout << "系统提交的页面峰值: " << perfor_info.CommitPeak << " Pages" << endl;
		cout << "按页分配的物理内存总数: " << perfor_info.PhysicalTotal << " Pages" << endl;
		cout << "按页分配的物理内存可用量: " << perfor_info.PhysicalAvailable << " Pages" << endl;
		cout << "系统物理内存占用: " << (perfor_info.PhysicalTotal - perfor_info.PhysicalAvailable)*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "系统物理内存可用: " << perfor_info.PhysicalAvailable*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "系统物理内存总数: " << perfor_info.PhysicalTotal*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "系统缓存总量： " << perfor_info.PhysicalAvailable << " Pages" << endl;
		cout << "系统内核内存占据页面总数： " << perfor_info.KernelTotal << " Pages" << endl;
		cout << "系统内核内存占据分页页面数： " << perfor_info.KernelNonpaged << " Pages" << endl;
		cout << "系统内核内存占据不分页页面数： " << perfor_info.KernelPaged << " Pages" << endl;
		cout << "系统句柄总量： " << perfor_info.HandleCount << " Pages" << endl;
		cout << "系统进程总量： " << perfor_info.ProcessCount << " Pages" << endl;
		cout << "系统线程总量： " << perfor_info.ThreadCount << " Pages" << endl;
		if (getchar())
			break;
	}
}

//查看存储器信息    YQK
void ShowMemoryUnit(void)
{
	PERFORMANCE_INFORMATION pi;
	pi.cb = sizeof(pi);
	GetPerformanceInfo(&pi, sizeof(pi));
	cout << "存储器信息" << endl;
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
}

//复制单个文件    ZYH
void CopyFile(char * fsource, char * ftarget)           //输入为两个不同路径的相同文件名
{
	WIN32_FIND_DATA lpfindfiledata;

	HANDLE hfind = FindFirstFile(fsource, &lpfindfiledata);    //找到源文件

	HANDLE hsource = CreateFile(fsource,        //指向打开文件名的指针  
		GENERIC_READ | GENERIC_WRITE,           //指定对象的访问类型(通过读和写联合访问可以实现文件的读写)  
		FILE_SHARE_READ,                        //与其他进程共享文件的方式  
		NULL,                                   //指向安全属性的指针(NULL表示默认安全对象)  
		OPEN_ALWAYS,                            //指定如何创建文件(OPEN_ALWAYS为如果文件存在则打开文件，否则创建新的文件)  
		FILE_ATTRIBUTE_NORMAL,                  //指定文件的属性和标志(FILE_ATTRIBUTE_NORMAL设置为默认属性)  
		NULL);                                  //用于复制文件句柄  

	HANDLE htarget = CreateFile(ftarget,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	LONG size = lpfindfiledata.nFileSizeLow - lpfindfiledata.nFileSizeHigh;   //源文件的大小  

	DWORD wordbit;            //一字一字存入缓冲区


	int *BUFFER = new int[size];         	//建立缓冲区


	ReadFile(hsource,         //指定要读的文件句柄  
		BUFFER,               //指向存放从文件读的数据的缓冲区的地址指针  
		size,                 //要从文件读的字节数  
		&wordbit,             //存放实际从文件中读的字节数的变量地址  
		NULL);                //同步方式打开设置为NULL  

	WriteFile(htarget,        //指定要写的文件对象的句柄  
		BUFFER,               //指向要写入文件的数据缓冲区指针  
		size,                 //要写入文件的字节数  
		&wordbit,             //实际写入的字节数  
		NULL);                //同步方式打开设置为NULL  

	FindClose(hfind);
	CloseHandle(hsource);
	CloseHandle(htarget);
}

//遍历所复制文件夹的子文件夹与子文件    ZYH
void mycp(char * fsource, char * ftarget)
{
	WIN32_FIND_DATA lpfindfiledata;
	char source[buf_size];
	char target[buf_size];
	lstrcpy(source, fsource);
	lstrcpy(target, ftarget);
	lstrcat(source, "\\*");                 //所有类型文件
	lstrcat(target, "\\");
	HANDLE hfind = FindFirstFile(source, &lpfindfiledata);    //查找第一个源文件
	if (hfind != INVALID_HANDLE_VALUE)
	{
		while (FindNextFile(hfind, &lpfindfiledata) != 0)    //遍历当前目录下的所有文件
		{
			if ((lpfindfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)     //如果找到了一个目录 
			{
				if ((strcmp(lpfindfiledata.cFileName, ".") != 0) && (strcmp(lpfindfiledata.cFileName, "..") != 0))  //它不是当前目录或父目录
				{
					memset(source, '0', sizeof(source));
					lstrcpy(source, fsource);
					lstrcat(source, "\\");
					lstrcat(source, lpfindfiledata.cFileName);    //将source改写为当前循环目录\\查找到的子目录
					lstrcat(target, lpfindfiledata.cFileName);    //将target改写为当前循环目录\\查找到的子目录
					CreateDirectory(target, NULL);                //在目标目录下创建子目录  
					mycp(source, target);                         //进入递归，开始复制子目录        
					lstrcpy(source, fsource);                     //在复制子文件夹结束后，回到当前循环目录，继续复制下一个文件或文件夹
					lstrcat(source, "\\");
					lstrcpy(target, ftarget);
					lstrcat(target, "\\");
				}
			}
			else                            //如果找到了一个文件
			{
				memset(source, '0', sizeof(source));
				lstrcpy(source, fsource);
				lstrcat(source, "\\");
				lstrcat(source, lpfindfiledata.cFileName);
				lstrcat(target, lpfindfiledata.cFileName);
				CopyFile(source, target);                  //调用文件复制函数  
				lstrcpy(source, fsource);
				lstrcat(source, "\\");
				lstrcpy(target, ftarget);
				lstrcat(target, "\\");
			}
		}
	}
	else
	{
		printf("查找指定文件路径的文件失败!\n");
	}
}

//移动文件并重命名   ZYH
void myRename(char * oldname, char * newname)
{
	if (!MoveFile(oldname/*旧名*/, newname/*新名*/))
	{
		printf("移动文件发生错误\n", GetLastError());
	}
	else
	{
		printf("移动文件成功\n");
	}
}

//删除文件    ZYH
void myDelete(char * fileName)
{
	if (!DeleteFile(fileName))
	{
		printf("删除文件发生错误\n");
	}
	else
	{
		printf("删除文件成功\n");
	}
}

//主函数设置所需参数，利用switch调用不同的功能函数    ZYH
int main(void)
{
	fstream fileout;
	WIN32_FIND_DATA lpfindfiledata;
	char source[100];
	char target[100];

	//程序持续运行，键入0时退出
	while (1)
	{
		//交互界面
		cout << endl;
		cout << "==========================================================================" << endl << endl;
		cout << "\t\t\t请输入数字执行相应功能：" << endl;
		cout << "\t1：查找文件" << endl;
		cout << "\t2：复制文件" << endl;
		cout << "\t3：移动文件" << endl;
		cout << "\t4：删除文件" << endl;
		cout << "\t5：查询所有进程" << endl;
		cout << "\t6：查询指定进程" << endl;
		cout << "\t7：终止指定进程" << endl;
		cout << "\t8：获取内存信息" << endl;
		cout << "\t9：获取系统信息" << endl;
		cout << "\t10：获取存储器信息" << endl;
		//待补充
		cout << "\t11：浏览历史记录" << endl;
		cout << "\t0：退出" << endl << endl;
		cout << "===========================All Rights Reserved=============================";
		cout << endl;
		int x;
		cin >> x;

		//键入0时退出程序
		if (x == 0)
		{
			writeHistory(x);
			createBackup(path_history, path_backup);
			break;
		}
		else
			switch (x)
			{
			//执行查找文件的功能
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

			//执行复制文件的功能
			case 2:
				writeHistory(x);
				cout << "请输入希望复制的源文件或源文件夹：" << endl;
				cin >> source;
				cout << "请输入希望复制到的目标路径：" << endl;
				cin >> target;
				if (FindFirstFile(source, &lpfindfiledata) == INVALID_HANDLE_VALUE)
				{
					printf("查找源文件路径失败!\n");
				}
				if (FindFirstFile(target, &lpfindfiledata) == INVALID_HANDLE_VALUE)
				{
					CreateDirectory(target, NULL);//为目标文件创建目录  
				}
				mycp(source, target);
				cout << "复制成功！" << endl;

				WaitForSingleObject(history_mutex, INFINITE);
				fileout.open(path_history, ios::out | ios::ate | ios::app);
				getTime();
				fileout << timeBuffer << "  复制的源文件或源文件夹为：" << source << endl;
				fileout << timeBuffer << "复制到的目标路径为：" << target << endl;
				fileout.close();
				ReleaseSemaphore(history_mutex, 1, NULL);
				break;

			//执行移动文件并重命名的功能
			case 3:
				writeHistory(x);
				cout << "请输入希望移动的文件：" << endl;
				cin >> source;
				cout << "请输入移动目标路径与文件的新名字：" << endl;
				cin >> target;
				myRename(source, target);

				WaitForSingleObject(history_mutex, INFINITE);
				fileout.open(path_history, ios::out | ios::ate | ios::app);
				getTime();
				fileout << timeBuffer << "  移动前的文件为：" << source << endl;
				fileout << timeBuffer << "移动后的文件为：" << target << endl;
				fileout.close();
				ReleaseSemaphore(history_mutex, 1, NULL);
				break;

			//执行删除文件的功能
			case 4:
				writeHistory(x);
				cout << "请输入希望删除的文件名：" << endl;
				cin >> source;
				myDelete(source);

				WaitForSingleObject(history_mutex, INFINITE);
				fileout.open(path_history, ios::out | ios::ate | ios::app);
				getTime();
				fileout << timeBuffer << "  删除的文件为：" << source << endl;
				fileout.close();
				ReleaseSemaphore(history_mutex, 1, NULL);
				break;

			//执行查询所有进程信息的功能
			case 5:
				writeHistory(x);
				ShowAllProcess(-1);
				break;

			//执行查询单个进程信息的功能
			case 6:
				writeHistory(x);
				QuerySingleProcess();
				break;

			//执行终止进程的功能
			case 7:
				writeHistory(x);
				KillProcess();
				break;

			//执行查看内存信息的功能
			case 8:
				writeHistory(x);
				ShowMemory();
				break;

			//执行查看系统信息的功能
			case 9:
				writeHistory(x);
				ShowSystem();
				break;

			//执行查看存储器信息的功能
			case 10:
				writeHistory(x);
				ShowMemoryUnit();
				break;

			//执行浏览历史记录的功能
			case 11:
				writeHistory(x);
				readHistory();
				break;

			//输入非法命令报错
			default:
				cout << "您输入的命令无法识别！" << endl;
				break;
			}
	}
}
