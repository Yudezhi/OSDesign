//写入历史记录与完成备份相关的头文件    ZYH

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <Windows.h>  
#include <Windowsx.h>  
#include <time.h> 
#include <cstdio>

using namespace std;

//设置互斥锁
HANDLE history_mutex = CreateSemaphore(NULL, 1, 1, "INNER");

char timeBuffer[32];

//设置历史文件与历史备份的路径
LPCSTR path_history = "E:\\job\\helloWorld\\内存管理\\Debug\\history.txt";
LPCSTR path_backup = "E:\\job\\helloWorld\\内存管理\\Debug\\backup.txt";

//得到当前时间
void getTime(void)
{
	time_t t = time(0);
	strftime(timeBuffer, sizeof(timeBuffer), "%Y/%m/%d %X", localtime(&t));
}

//写入历史记录
bool writeHistory(int a)
{
	fstream fileout;
	//申请互斥锁，防止多进程同时读写
	WaitForSingleObject(history_mutex, INFINITE);
	//将文件按流式打开，追加写入，于文件末尾添加新的历史记录
	fileout.open(path_history, ios::out | ios::ate | ios::app);
	if (fileout.is_open())
	{
		switch (a)
		{
		case 1:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "1号操作：文件查找" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 2:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "2号操作：复制文件" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 3:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "3号操作：移动文件" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 4:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "4号操作：删除文件" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 5:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "5号操作：查询所有进程" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 6:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "6号操作：查询指定进程" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 7:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "7号操作：终止指定进程" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 8:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "8号操作：获取内存信息" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 9:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "9号操作：获取系统信息" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 10:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "10号操作：获取存储器信息" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 11:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "11号操作：浏览历史记录" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 0:
			getTime();
			fileout << timeBuffer << "  执行了";
			fileout << "0号操作：退出" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;
		}
	}
	else
	{
		cout << "写入历史记录失败！" << endl;
		ReleaseSemaphore(history_mutex, 1, NULL);
		return false;
	}
}

bool readHistory(void)
{
	fstream filein;
	WaitForSingleObject(history_mutex, INFINITE);
	filein.open(path_history, ios::in);
	char buffer[1024];
	if (filein.is_open())
	{
		cout << "历史记录如下：" << endl;
		while (!filein.eof())
		{
			filein.getline(buffer, 1023);
			cout << buffer << endl;
		}
		filein.close();
		ReleaseSemaphore(history_mutex, 1, NULL);
		return true;
	}
	else
	{
		cout << "读取历史记录失败！" << endl;
		ReleaseSemaphore(history_mutex, 1, NULL);
		return false;
	}
}

void createBackup(LPCSTR fsource, LPCSTR ftarget)
{
	WIN32_FIND_DATA lpfindfiledata;
	//查找指定文件路径  
	HANDLE hfind = FindFirstFile(fsource,		//文件名  
		&lpfindfiledata);						//数据缓冲区  
	HANDLE hsource = CreateFile(fsource,		//指向打开文件名的指针  
		GENERIC_READ | GENERIC_WRITE,			//指定对象的访问类型(通过读和写联合访问可以实现文件的读写)  
		FILE_SHARE_READ,						//与其他进程共享文件的方式  
		NULL,									//指向安全属性的指针(NULL表示默认安全对象)  
		OPEN_ALWAYS,							//指定如何创建文件(OPEN_ALWAYS为如果文件存在则打开文件，否则创建新的文件)  
		FILE_ATTRIBUTE_NORMAL,					//指定文件的属性和标志(FILE_ATTRIBUTE_NORMAL设置为默认属性)  
		NULL);									//用于复制文件句柄  
	HANDLE htarget = CreateFile(ftarget,		//指向创建文件名的指针  
		GENERIC_READ | GENERIC_WRITE,			//指定对象的访问类型(通过读和写联合访问可以实现文件的读写)  
		FILE_SHARE_READ,						//与其他进程共享文件的方式  
		NULL,									//指向安全属性的指针(NULL表示默认安全对象)  
		CREATE_ALWAYS,							//指定如何创建文件(CREATE_ALWAYS为创建一个已经存在的新文件，如果存在会将现有的文件清空且重写文件)  
		FILE_ATTRIBUTE_NORMAL,					//指定文件的属性和标志(FILE_ATTRIBUTE_NORMAL设置为默认属性)  
		NULL);									//用于复制文件句柄  
												//源文件的大小  
	LONG size = lpfindfiledata.nFileSizeLow - lpfindfiledata.nFileSizeHigh;

	DWORD wordbit;

	//新开缓冲区，保存数据  
	int *BUFFER = new int[size];

	//源文件读数据  
	ReadFile(hsource,		//指定要读的文件句柄  
		BUFFER,				//指向存放从文件读的数据的缓冲区的地址指针  
		size,				//要从文件读的字节数  
		&wordbit,			//存放实际从文件中读的字节数的变量地址  
		NULL);				//同步方式打开设置为NULL  
							//目标文件写数据  
	WriteFile(htarget,		//指定要写的文件对象的句柄  
		BUFFER,				//指向要写入文件的数据缓冲区指针  
		size,				//要写入文件的字节数  
		&wordbit,			//实际写入的字节数  
		NULL);				//同步方式打开设置为NULL  

	FindClose(hfind);
	CloseHandle(hsource);
	CloseHandle(htarget);
}