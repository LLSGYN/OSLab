#include<stdio.h>
#include <stdlib.h>
#include"fsapi.h"
#include"File.h"
#include"Disk.h"


//在文件描述符表中寻找最小的未被使用的文件描述符
int findfd() {
    int fdtmp = 3;
    while (fdtmp <= FDMAX && FDE[fdtmp].flag == 1) {
        fdtmp++;
    }
    return fdtmp;
}

//打开文件，并根据文件名添加文件描述符
int createfd(char fileName[]) {
    FCB* myFCB = my_open(fileName);
    int fdnum = findfd();
    FDE[fdnum].flag = 1;
    FDE[fdnum].tagetFCB = myFCB;
    return fdnum;
}

//将 buf 中 n 字节写入到文件描述符中；返回 n
int write(int fd, char* buf) {
    my_write(FDE[fd].tagetFCB, buf);
    return 1;
}

//从文件描述符中读取 n 字节到 buf ；返回读取字节数，文件结束为 0
int read(int fd, char* buf, int n) {
    //buf = my_read(FDE[fd].tagetFCB, n);
	my_read(FDE[fd].tagetFCB, n);
    /*if (strlen(buf) < n) {
        return 0;
    }*/
    return n;
}

//释放一个文件描述符
int close(int fd) {
    FDE[fd].flag = 0;
    FDE[fd].tagetFCB = NULL;
    return 0;
}

//返回一个新文件描述符，其引用与 fd 相同的文件
int dup(int fd) {
    int newfd = findfd();
    FDE[newfd].flag = 1;
    FDE[newfd].tagetFCB = FDE[fd].tagetFCB;
    return newfd;
}

//创建管道，将读写文件描述符放置在 p[0] 和 p[1]
int pipe(int p[]) {
    p[0] = findfd();
    p[1] = findfd();
    return 0;
}

//改变当前目录
int chdir(char* dir) {
    return changeDir(dir);
}

//创建新目录
int mkdir(char* dir) {
    return creatDir(dir);
}

//为文件file1 创建一个新的名称(file2)
int link(char* file1, char* file2) {
    return linkfile(file1, file2);

}

//移除一个文件
int unlink(char* file) {
    return deletelink(file);
}

//ls命令
void listfile() {
    showDir();
}

//pwd命令
char* printwd() {
    return getPath();
}

//touch命令
int touch(char fileName[], int fileSize) {
    return creatFile(fileName, fileSize);
}