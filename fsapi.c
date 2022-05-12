#include<stdio.h>
#include <stdlib.h>
#include"fsapi.h"
#include"File.h"
#include"Disk.h"


//���ļ�����������Ѱ����С��δ��ʹ�õ��ļ�������
int findfd() {
    int fdtmp = 3;
    while (fdtmp <= FDMAX && FDE[fdtmp].flag == 1) {
        fdtmp++;
    }
    return fdtmp;
}

//���ļ����������ļ�������ļ�������
int createfd(char fileName[]) {
    FCB* myFCB = my_open(fileName);
    int fdnum = findfd();
    FDE[fdnum].flag = 1;
    FDE[fdnum].tagetFCB = myFCB;
    return fdnum;
}

//�� buf �� n �ֽ�д�뵽�ļ��������У����� n
int write(int fd, char* buf) {
    my_write(FDE[fd].tagetFCB, buf);
    return 1;
}

//���ļ��������ж�ȡ n �ֽڵ� buf �����ض�ȡ�ֽ������ļ�����Ϊ 0
int read(int fd, char* buf, int n) {
    //buf = my_read(FDE[fd].tagetFCB, n);
	my_read(FDE[fd].tagetFCB, n);
    /*if (strlen(buf) < n) {
        return 0;
    }*/
    return n;
}

//�ͷ�һ���ļ�������
int close(int fd) {
    FDE[fd].flag = 0;
    FDE[fd].tagetFCB = NULL;
    return 0;
}

//����һ�����ļ����������������� fd ��ͬ���ļ�
int dup(int fd) {
    int newfd = findfd();
    FDE[newfd].flag = 1;
    FDE[newfd].tagetFCB = FDE[fd].tagetFCB;
    return newfd;
}

//�����ܵ�������д�ļ������������� p[0] �� p[1]
int pipe(int p[]) {
    p[0] = findfd();
    p[1] = findfd();
    return 0;
}

//�ı䵱ǰĿ¼
int chdir(char* dir) {
    return changeDir(dir);
}

//������Ŀ¼
int mkdir(char* dir) {
    return creatDir(dir);
}

//Ϊ�ļ�file1 ����һ���µ�����(file2)
int link(char* file1, char* file2) {
    return linkfile(file1, file2);

}

//�Ƴ�һ���ļ�
int unlink(char* file) {
    return deletelink(file);
}

//ls����
void listfile() {
    showDir();
}

//pwd����
char* printwd() {
    return getPath();
}

//touch����
int touch(char fileName[], int fileSize) {
    return creatFile(fileName, fileSize);
}