#pragma once
#include<stdio.h>
#include"File.h"

#define FDMAX 1023

//�ļ����������±�Ϊ�ļ���������ֵ
struct File_Descriptor {
    int flag;//flagΪ0������ļ�����������
    FCB* tagetFCB;//�ļ���������ָ���Ŀ���ļ����ƿ�
}FDE[FDMAX];


int findfd();//���ļ�����������Ѱ����С��δ��ʹ�õ��ļ�������
int createfd(char fileName[]);//���ļ����������ļ�������ļ�������
int write(int fd, char* buf);//�� buf �� n �ֽ�д�뵽�ļ��������У����� n
int read(int fd, char* buf, int n);//���ļ��������ж�ȡ n �ֽڵ� buf �����ض�ȡ�ֽ������ļ�����Ϊ 0
int close(int fd);//�ͷ�һ���ļ�������
int dup(int fd);//����һ�����ļ����������������� fd ��ͬ���ļ�
int pipe(int p[]);//�����ܵ�������д�ļ������������� p[0] �� p[1]
int chdir(char* dir);//�ı䵱ǰĿ¼
int mkdir(char* dir);//������Ŀ¼
int link(char* file1, char* file2);//Ϊ�ļ�file1 ����һ���µ�����(file2)
int unlink(char* file);//�Ƴ�һ���ļ�
void listfile();//ls����
char* printwd();//pwd����
int touch(char fileName[], int fileSize);//touch����