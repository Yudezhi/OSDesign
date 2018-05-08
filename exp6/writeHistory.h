//д����ʷ��¼����ɱ�����ص�ͷ�ļ�    ZYH

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

//���û�����
HANDLE history_mutex = CreateSemaphore(NULL, 1, 1, "INNER");

char timeBuffer[32];

//������ʷ�ļ�����ʷ���ݵ�·��
LPCSTR path_history = "E:\\job\\helloWorld\\�ڴ����\\Debug\\history.txt";
LPCSTR path_backup = "E:\\job\\helloWorld\\�ڴ����\\Debug\\backup.txt";

//�õ���ǰʱ��
void getTime(void)
{
	time_t t = time(0);
	strftime(timeBuffer, sizeof(timeBuffer), "%Y/%m/%d %X", localtime(&t));
}

//д����ʷ��¼
bool writeHistory(int a)
{
	fstream fileout;
	//���뻥��������ֹ�����ͬʱ��д
	WaitForSingleObject(history_mutex, INFINITE);
	//���ļ�����ʽ�򿪣�׷��д�룬���ļ�ĩβ����µ���ʷ��¼
	fileout.open(path_history, ios::out | ios::ate | ios::app);
	if (fileout.is_open())
	{
		switch (a)
		{
		case 1:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "1�Ų������ļ�����" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 2:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "2�Ų����������ļ�" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 3:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "3�Ų������ƶ��ļ�" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 4:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "4�Ų�����ɾ���ļ�" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 5:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "5�Ų�������ѯ���н���" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 6:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "6�Ų�������ѯָ������" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 7:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "7�Ų�������ָֹ������" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 8:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "8�Ų�������ȡ�ڴ���Ϣ" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 9:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "9�Ų�������ȡϵͳ��Ϣ" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 10:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "10�Ų�������ȡ�洢����Ϣ" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 11:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "11�Ų����������ʷ��¼" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;

		case 0:
			getTime();
			fileout << timeBuffer << "  ִ����";
			fileout << "0�Ų������˳�" << endl;
			fileout.close();
			ReleaseSemaphore(history_mutex, 1, NULL);
			return true;
		}
	}
	else
	{
		cout << "д����ʷ��¼ʧ�ܣ�" << endl;
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
		cout << "��ʷ��¼���£�" << endl;
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
		cout << "��ȡ��ʷ��¼ʧ�ܣ�" << endl;
		ReleaseSemaphore(history_mutex, 1, NULL);
		return false;
	}
}

void createBackup(LPCSTR fsource, LPCSTR ftarget)
{
	WIN32_FIND_DATA lpfindfiledata;
	//����ָ���ļ�·��  
	HANDLE hfind = FindFirstFile(fsource,		//�ļ���  
		&lpfindfiledata);						//���ݻ�����  
	HANDLE hsource = CreateFile(fsource,		//ָ����ļ�����ָ��  
		GENERIC_READ | GENERIC_WRITE,			//ָ������ķ�������(ͨ������д���Ϸ��ʿ���ʵ���ļ��Ķ�д)  
		FILE_SHARE_READ,						//���������̹����ļ��ķ�ʽ  
		NULL,									//ָ��ȫ���Ե�ָ��(NULL��ʾĬ�ϰ�ȫ����)  
		OPEN_ALWAYS,							//ָ����δ����ļ�(OPEN_ALWAYSΪ����ļ���������ļ������򴴽��µ��ļ�)  
		FILE_ATTRIBUTE_NORMAL,					//ָ���ļ������Ժͱ�־(FILE_ATTRIBUTE_NORMAL����ΪĬ������)  
		NULL);									//���ڸ����ļ����  
	HANDLE htarget = CreateFile(ftarget,		//ָ�򴴽��ļ�����ָ��  
		GENERIC_READ | GENERIC_WRITE,			//ָ������ķ�������(ͨ������д���Ϸ��ʿ���ʵ���ļ��Ķ�д)  
		FILE_SHARE_READ,						//���������̹����ļ��ķ�ʽ  
		NULL,									//ָ��ȫ���Ե�ָ��(NULL��ʾĬ�ϰ�ȫ����)  
		CREATE_ALWAYS,							//ָ����δ����ļ�(CREATE_ALWAYSΪ����һ���Ѿ����ڵ����ļ���������ڻὫ���е��ļ��������д�ļ�)  
		FILE_ATTRIBUTE_NORMAL,					//ָ���ļ������Ժͱ�־(FILE_ATTRIBUTE_NORMAL����ΪĬ������)  
		NULL);									//���ڸ����ļ����  
												//Դ�ļ��Ĵ�С  
	LONG size = lpfindfiledata.nFileSizeLow - lpfindfiledata.nFileSizeHigh;

	DWORD wordbit;

	//�¿�����������������  
	int *BUFFER = new int[size];

	//Դ�ļ�������  
	ReadFile(hsource,		//ָ��Ҫ�����ļ����  
		BUFFER,				//ָ���Ŵ��ļ��������ݵĻ������ĵ�ַָ��  
		size,				//Ҫ���ļ������ֽ���  
		&wordbit,			//���ʵ�ʴ��ļ��ж����ֽ����ı�����ַ  
		NULL);				//ͬ����ʽ������ΪNULL  
							//Ŀ���ļ�д����  
	WriteFile(htarget,		//ָ��Ҫд���ļ�����ľ��  
		BUFFER,				//ָ��Ҫд���ļ������ݻ�����ָ��  
		size,				//Ҫд���ļ����ֽ���  
		&wordbit,			//ʵ��д����ֽ���  
		NULL);				//ͬ����ʽ������ΪNULL  

	FindClose(hfind);
	CloseHandle(hsource);
	CloseHandle(htarget);
}