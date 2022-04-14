#include<stdio.h>
#include"File.h"

#define FDMAX 1023

//文件描述符表，下标为文件描述符的值
struct File_Descriptor{
    int flag;//flag为0代表此文件描述符可用
    FCB* tagetFCB;//文件描述符所指向的目标文件控制块
}FDE[FDMAX];


int findfd();//在文件描述符表中寻找最小的未被使用的文件描述符
int createfd(char fileName[]);//打开文件，并根据文件名添加文件描述符
int write( int fd, char *buf, int n);//将 buf 中 n 字节写入到文件描述符中；返回 n
int read(int fd, char *buf, int n);//从文件描述符中读取 n 字节到 buf ；返回读取字节数，文件结束为 0
int close( int fd);//释放一个文件描述符
int dup(int fd);//返回一个新文件描述符，其引用与 fd 相同的文件
int pipe(int p[]);//创建管道，将读写文件描述符放置在 p[0] 和 p[1]
int chdir(char *dir);//改变当前目录
int mkdir(char *dir);//创建新目录
int link (char *file1, char * file2);//为文件file1 创建一个新的名称(file2)
int unlink( char *file);//移除一个文件
void listfile();//ls命令
char* printwd();//pwd命令
int touch(char fileName[], int fileSize);//touch命令