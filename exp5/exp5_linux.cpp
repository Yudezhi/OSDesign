#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>  //open()
#include <iostream>
#include <string>
#include <sys/types.h>
#include <utime.h>

#define FILELENGTH   256

# define FILE_MODE  0755  

using namespace std;
//copy file here
/*readme:
1.current mode will link to each other if it has a hard link or symbol link

2. if hadlink is not  necessary,just delete the link and copy 

3. if symbol link is necessary,use " cp -a / cp -ar  /cp -r etc".


*/
// struct utimbuf  
// {  
//     time_t actime;  
//     time_t modtime;  
// };
string getfilename(char * path)
{
			string name;
			name = path;
			int pos = name.find_last_of("/\\");
			if(pos>=0)
			name = name.substr(pos+1);//name is the file name
			else
			cout<<"check your input path ,make sure to seprate by '/'"<<endl;

			cout<<"pos is :"<<pos<<endl;
			cout<<"the file name Get is:"<<name<<endl;
			return name;
}
void OrdiaryCopy(char *fsource, char *ftarget)
{
	struct utimbuf timebuf;
	int fd = open(fsource, 0);
			int fdr;
			struct stat statbuf;
			char BUFFER[1024];
			int wordbit;
			stat(fsource, &statbuf);
	//
			fdr = creat(ftarget, statbuf.st_mode);

	//read and write
			while ((wordbit = read(fd, BUFFER, 1024)) > 0)
			{
			write(fdr, BUFFER, wordbit);
			}
			timebuf.actime=statbuf.st_atime;
			timebuf.modtime=statbuf.st_mtime;
			utime(ftarget, &timebuf );

	//close 
			close(fd);
			close(fdr);
}
void SymbolCopy(char *fsource, char *ftarget)
{			
			struct utimbuf timebuf;
			struct stat sb;
			lstat(fsource,&sb);
			printf("=======symbol link file ==========\t");
			getchar();
			struct dirent * entry;
			char  symname[1000];
			printf("%s\n",fsource );
///////////////////////////////////////////////////////
			readlink(fsource,symname,sb.st_size);	//3			//  1symbolfilename   2targetpath  3 point to
			
			symname[sb.st_size]='\0'; 				 //important
			//int len_b = b.find_last_of(NULL);
			//printf("len_b=%d\n",len_b );
//////////////////////////////////////////////////////////
			printf("%s\n",symname);
			string symbolname = getfilename(symname);
			const char * symbolname2 =symbolname.c_str();
			string name = getfilename(fsource); //1
			string targetpath= ftarget;
			int a= targetpath.find_last_of("/\\");
			printf("a=%d\n",a );
			targetpath=targetpath.substr(0,a);//without name
			const char * targetpath2 = targetpath.c_str();
			printf("--%s--\n", targetpath2);
			//name="consumer.c";
			const char * name2 = name.c_str();
			
			DIR * dir;
			if((dir  = opendir(targetpath2))==NULL)
			{
				printf("error2!\n");
			}
			
        	int fd;
        	fd = dirfd(dir);

        	printf("=============\n");
        	printf("%s\n", symbolname2);
        	printf("name2=%s\n",name2 );
        	printf("=============\n");
       		symlinkat(symbolname2,fd,name2);//oldone newone
       		timebuf.actime = sb.st_atime;
       		timebuf.modtime=sb.st_mtime;
       		utime(ftarget,&timebuf);

       		
}

void CopyFile(char *fsource, char *ftarget)
{
	int flag=0;
	int l=0;
//////////////////////////////////////////////////////////
    struct stat buf = {0};
    lstat(fsource, &buf);
    //printf("number of hard links for %s  %d\n", fsource, buf.st_nlink);

    //if(buf.st_nlink>1)//=================================================hardlink
   	//link(fsource, ftarget);

    if (S_ISLNK(buf.st_mode))//========================================symbol link
        {
        	int symsize = buf.st_size;
        	printf("=======symbol link file ==========\t");
        	getchar();
        	SymbolCopy(fsource,ftarget);
        }
    else

      {
      		OrdiaryCopy(fsource,ftarget);
      }

/////////////////////////////////////////////////////////////

	// string source=fsource;
	// //printf("%s\n", source);
	// for(int i=0;i<source.size();i++)
	// {
	// //	printf("%d:%d\n",i,source.size() );
	// 	if(source.at(i)==' ')
	// 		{
	// 			source.replace(i,1,"\\ ");
	// 			i++;
	// 			flag=1;
	// 		}
	// }
	// string target=ftarget;
	// if(flag)
	// {

	// 	for(int i=0;i<target.size();i++)
	// 	{
	// 	//	printf("%d:%d\n",i,source.size() );
	// 		if(target.at(i)==' ')
	// 			{
	// 				target.replace(i,1,"\\ ");
	// 				i++;
	// 			}
	// 	}
	// 	flag=0;
	// }
///////////////////////////////////////////////////////////////////
	// here the source and target is formed

	// string str_cp="cp -a ";
	// str_cp+=source;
	// str_cp+=' ';
	// str_cp+=target;
	// const char * commond = str_cp.c_str();
	// //printf("%s\n", commond);
	// //getchar();
	// if(buf.st_nlink<2&& l==0 )
	// system(commond);
	// l=0;
}

//copy a folder
void CopyFolder(char *fsource, char *ftarget)
{
	char source[FILELENGTH];
	char target[FILELENGTH];
	struct stat statbuf;
	struct stat statbuf2;
	DIR *dir;
	struct dirent * entry;
	struct  utimbuf timebuf;
	/*
	struct dirent
{
    long d_ino;                 // inode number 索引节点号 
    off_t d_off;                // offset to this dirent 在目录文件中的偏移 
    unsigned short d_reclen;    // length of this d_name 文件名长 
    unsigned char d_type;        //the type of d_name 文件类型 
    char d_name [256];   // file name (null-terminated) 文件名，最长255字符 
}
*/
	dir = opendir(fsource);
	//
	while ((entry = readdir(dir)) != NULL)
	{
		strcpy(source, fsource);
		strcpy(target, ftarget);
		//
		//printf("%s\n",entry->d_name);
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)   //current folder or father folder
			continue;
		//
		if (entry->d_type == DT_DIR)   //folder  deep until a file appear
		{
			//
			strcat(source, "/");
			strcat(source, entry->d_name);

			strcat(target, "/");
			strcat(target, entry->d_name);
			stat(source, &statbuf);

			//create the sub folder and file
			mkdir(target, FILE_MODE);//statbuf.st_mode
			
			//stat(target,&statbuf2);
			//statbuf2.st_ctime=statbuf.st_ctime; //folder
			//copy file only
			
			CopyFolder(source, target);
			timebuf.actime=statbuf.st_atime;
			timebuf.modtime=statbuf.st_mtime;
			utime(target, &timebuf );

		}
		//
		else  //  copy file
		{
			
			strcat(source, "/");
			strcat(source, entry->d_name);
			strcat(target, "/");
			strcat(target, entry->d_name);
			//copy
			CopyFile(source, target);
		}
	}
}

int main(int argc, char *argv[])

{
	int len = strlen(argv[1]);
	int ans, t = 0, i, j;
	int tt = 0;
	int len2;
	char hu[100];
	char *ww;

	struct stat statbuf;
/*
	struct stat    
{    
    dev_t       st_dev;     // ID of device containing file   
    ino_t       st_ino;     // inode number -inode 
    mode_t      st_mode;    // 文件的类型和存取的权限   
    nlink_t     st_nlink;   // number of hard links   
    uid_t       st_uid;     // user ID of owner -user id   
    gid_t       st_gid;     // group ID of owner - group id   
    dev_t       st_rdev;    // device ID (if special file)     
    off_t       st_size;    // total size, in bytes -文件大小，字节为单位    
    blksize_t   st_blksize; // blocksize for filesystem I/O -系统块的大小    
    blkcnt_t    st_blocks;  // number of blocks allocated -文件所占块数    
    time_t      st_atime;   // time of last access -最近存取时间
    time_t      st_mtime;   // time of last modification -最近修改时间    
    time_t      st_ctime;   // time of last status change - 
};    
*/
	DIR * dir;

	// input
	if (argc != 3)
	{
		printf("Usage:./processname.exe  CopyFileFromPath CopyFileToPath");
		exit(-1);
	}
	else
	{
		printf("Usage:\n./exp5.exe ./test ./test2\n");
		printf("./exp5.exe ./test/q ./\n");
		stat(argv[1], &statbuf);

		/*
		const char *path: file path include the file name
		struct _stat *buffer: store the info
		return 0 for success,-1 for failure
		*/
		
		if ((statbuf.st_mode)&S_IFREG)  //S_IFREG     0100000     an  ordiary file ,copy derectly
		{
			if ((dir = opendir(argv[2])) == NULL)  //not exsit
			{
				mkdir(argv[2], FILE_MODE); // statbuf.st_mode 
			}
			printf("Input is an file,the desti folder has beeb set!\n");
			string name;
			name = argv[1];
			int pos = name.find_last_of("/\\");
			if(pos>=0)
			name = name.substr(pos+1);//name is the file name
			else
			cout<<"check your input path ,make sure to seprate by '/'"<<endl;

			cout<<"pos is :"<<pos<<endl;
			cout<<"the file name Get is:"<<name<<endl;

			//len2 = strlen(argv[2]);
			string name2=argv[2];
			//if(argv[2][name2.size()-1]=='/')
			//{
			name2+= name; // name2 is the des filepath include the name 
			argv[2] = (char *)name2.c_str();
			argv[2][name2.size()] = '\0';
			//}
			//else
			//{
				// argv[2] is argv[2]  ,it means change the file name 
			//}
			getchar();
			printf("the desti file path is: %s\n",argv[2] );
			CopyFile(argv[1], argv[2]);
			printf("success!\n");

			return 0;
		} //end if an ordiary file

		if ((opendir(argv[1])) == NULL)  //open failed
		{
			printf("The source file you input is Invalid!\n");
			return 0;
		}
		//  create the desti folder if not exist
		if ((dir = opendir(argv[2])) == NULL)
		{
			mkdir(argv[2],FILE_MODE); //statbuf.st_mode
		}
		//
		CopyFolder(argv[1], argv[2]);
		printf("Copy Finished!\n");
		return 0;
	}
}