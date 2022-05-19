#include "File.h"
#include "Disk.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NUMREADER 5
#define UNUSED -1

dirTable* rootDirTable; //��Ŀ¼
dirTable* currentDirTable;  //��ǰλ��
char path[200]; //���浱ǰ����·��

//TODO�����ļ�ϵͳ���������ڴ�page�н��У�����ϵͳ��������ҳ��дҳ�����Խ��ϵ�����


//��ʼ����Ŀ¼
void initRootDir()
{
	//����һ���̿�ռ��rootDirTable
	int startBlock = getBlock(1);
	if (startBlock == -1)
		return;
	char* page = (char*)malloc(block_szie);
	readBlock(startBlock, page);
	rootDirTable = (dirTable*)page;
	//printf("rootDir block:%d\n", startBlock);
	//printf("%p\n", rootDirTable);
	rootDirTable->dirUnitAmount = 0;
	rootDirTable->startBlock = startBlock;
	//��������Ϊ����Ŀ¼
	//addDirUnit(rootDirTable, "..", 0, startBlock);

	currentDirTable = rootDirTable;
	//��ʼ����ʼ����·��
	path[0] = '/';
	path[1] = '\0';
}


//��þ���·��
char* getPath()
{
	return path;
}


//չʾ��ǰĿ¼ ls
void showDir()
{
	int unitAmount = currentDirTable->dirUnitAmount;
	printf("total:%d\n", unitAmount);
	printf("name\ttype\tsize\tFCB\tdataStartBlock\n");
	//�������б���
	for (int i = 0; i < unitAmount; i++)
	{
		//��ȡĿ¼��
		dirUnit unitTemp = currentDirTable->dirs[i];

		char fileType[5] = { 0 };

		if (unitTemp.type == FILE) {
			strcpy(fileType, "File");
		}
		else if (unitTemp.type == LINKFILE) {
			strcpy(fileType, "Link");
		}
		else {
			strcpy(fileType, "Dir");
		}

		printf("%s\t%s\t", unitTemp.fileName, fileType);

		
		
		//�ñ������ļ������������С����ʼ�̿��
		if (unitTemp.type == FILE || unitTemp.type == LINKFILE)
		{
			int FCBBlock = unitTemp.startBlock;
			char* page = (char*)malloc(block_szie);
			readBlock(FCBBlock, page);
			FCB* fileFCB = (FCB*)page;	

			printf("%d\t%d\t%d\n", fileFCB->fileSize, FCBBlock, fileFCB->blockNum);
		}
		else {
			int dirBlock = unitTemp.startBlock;
			char* page = (char*)malloc(block_szie);
			readBlock(dirBlock, page);
			dirTable* myTable = (dirTable*)page;


			printf("%d\t%d\n", myTable->dirUnitAmount, unitTemp.startBlock);
		}
	}
}


//�л�Ŀ¼ cd
int changeDir(char dirName[])
{
	//Ŀ¼����Ŀ¼λ��
	int unitIndex = findUnitInTable(currentDirTable, dirName);
	//������
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	if (currentDirTable->dirs[unitIndex].type == 1)
	{
		printf("not a dir\n");
		return -1;
	}
	//�޸ĵ�ǰĿ¼
	int dirBlock = currentDirTable->dirs[unitIndex].startBlock;
	char* page = (char*)malloc(block_szie);
	readBlock(dirBlock, page);
	currentDirTable = (dirTable*)page;
	//�޸�ȫ�־���·��
	if (strcmp(dirName, "..") == 0)
	{
		//���˾���·��
		int len = strlen(path);
		for (int i = len - 2; i >= 0; i--)
			if (path[i] == '/')
			{
				path[i + 1] = '\0';
				break;
			}
	}
	else {
		strcat(path, dirName);
		strcat(path, "/");
	}

	return 0;
}



//�޸��ļ�������Ŀ¼�� mv
int changeName(char oldName[], char newName[])
{
	int unitIndex = findUnitInTable(currentDirTable, oldName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	strcpy(currentDirTable->dirs[unitIndex].fileName, newName);

	//uncomment if mem swap ready
	writeBlock(currentDirTable->startBlock, currentDirTable);

	return 0;
}


//******************������ɾ���ļ�********************
//�����ļ� touch
int creatFile(char fileName[], int fileSize)
{
	//����ļ����ֳ���
	if (strlen(fileName) >= NUM)
	{
		printf("file name too long\n");
		return -1;
	}

	//�Ƿ����ͬ���ļ�
	if (findUnitInTable(currentDirTable, fileName) != -1)
	{
		printf("file already exist\n");
		return -1;
	}

	//���FCB�Ŀռ�
	int FCBBlock = getBlock(1);
	if (FCBBlock == -1)
		return -1;
	//��ȡ�ļ����ݿռ�
	int FileBlock = getBlock(fileSize);
	if (FileBlock == -1)
		return -1;
	//����FCB
	if (creatFCB(FCBBlock, FileBlock, fileSize) == -1)
		return -1;
	//��ӵ�Ŀ¼��
	if (addDirUnit(currentDirTable, fileName, 1, FCBBlock) == -1)
		return -1;

	return 0;
}

//Ϊ�ļ�file1���һ����Ϊfile2�����ӣ�����ָ��ͬһ��FCB
int linkfile(char* file1, char* file2) {
	//�����ļ���Ŀ¼��λ��
	int unitIndex = findUnitInTable(currentDirTable, file1);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//�ж�����
	if (myUnit.type == FILE)
	{
		FCB* myFCB = my_open(file1);
		if (strlen(file2) >= NUM)
		{
			printf("file name too long\n");
			return -1;
		}
		myFCB->link++;

		if (addDirUnit(currentDirTable, file2, LINKFILE, myFCB->startBlock) == -1)
			return -1;

		//uncomment if mem swap ready
		writeBlock(myFCB->startBlock, myFCB);
	}
	else if (myUnit.type == DIRECTORY) {
		printf("to link a directory is currently not supported \n");
		return -1;
	}
	else {
		printf("to link a link file is currently not supported \n");
		return -1;
	}
	
	
	return 0;
}




//����FCB
int creatFCB(int fcbBlockNum, int fileBlockNum, int fileSize)
{
	//�ҵ�fcb�Ĵ洢λ��
	char* page = (char*)malloc(block_szie);
	readBlock(fcbBlockNum, page);
	FCB* currentFCB = (FCB*)page;
	currentFCB->startBlock = fcbBlockNum;
	currentFCB->blockNum = fileBlockNum;//�ļ�������ʼλ��
	currentFCB->fileSize = fileSize;//�ļ���С
	currentFCB->link = 1;//�ļ�������
	currentFCB->dataSize = 0;//�ļ���д�����ݳ���
	currentFCB->readptr = 0;//�ļ���ָ��


	currentFCB->count_sem = sem_open("count_sem", O_CREAT, 0644, NUMREADER);
	if (currentFCB->count_sem == SEM_FAILED)
	{
		perror("sem_open error");
		exit(1);
	}
	currentFCB->write_sem = sem_open("write_sem", O_CREAT, 0644, 1);
	if (currentFCB->write_sem == SEM_FAILED)
	{
		perror("sem_open error");
		exit(1);
	}

	//Comment if mem swap ready
	writeBlock(fcbBlockNum, currentFCB);

	return 0;
}

//����Ŀ¼ mkdir
int creatDir(char dirName[])
{
	//����ļ����Ƿ����
	if (strlen(dirName) >= NUM)
	{
		printf("file name too long\n");
		return -1;
	}
	//�Ƿ����ͬ���ļ�
	if (findUnitInTable(currentDirTable, dirName) != -1)
	{
		printf("file already exist\n");
		return -1;
	}
	//ΪĿ¼�����ռ�
	int dirBlock = getBlock(1);
	//printf("dirBlock in createDir: %d\n", dirBlock);
	if (dirBlock == -1)
		return -1;
	//��Ŀ¼��ΪĿ¼����ӵ���ǰĿ¼
	if (addDirUnit(currentDirTable, dirName, 0, dirBlock) == -1)
		return -1;
	//Ϊ�½���Ŀ¼���һ������Ŀ¼��Ŀ¼��
	char* page = (char*)malloc(block_szie);
	readBlock(dirBlock, page);
	dirTable* newTable = (dirTable*)page;
	newTable->startBlock = dirBlock;
	newTable->dirUnitAmount = 0;
	char parent[] = "..";
	if (addDirUnit(newTable, parent, DIRECTORY, currentDirTable->startBlock) == -1) {
		printf("addDirUnit Failed");
		return -1;
	}
	return 0;
}

//���Ŀ¼��
int addDirUnit(dirTable* myDirTable, char fileName[], int type, int FCBBlockNum)
{
	//printf("DirTable in addDirUnit: %p\n", myDirTable);
	//���Ŀ¼��
	int dirUnitAmount = myDirTable->dirUnitAmount;
	//���Ŀ¼��ʾ�Ƿ�����
	if (dirUnitAmount == DIRTABLE_MAX_SIZE)
	{
		printf("dirTables is full, try to delete some file\n");
		return -1;
	}

	//�Ƿ����ͬ���ļ�
	if (findUnitInTable(myDirTable, fileName) != -1)
	{
		printf("file already exist\n");
		return -1;
	}
	//������Ŀ¼��

	dirUnit* newDirUnit = &myDirTable->dirs[dirUnitAmount];
	myDirTable->dirUnitAmount++;//��ǰĿ¼���Ŀ¼������+1
	//������Ŀ¼������

	strcpy(newDirUnit->fileName, fileName);

	newDirUnit->type = type;
	newDirUnit->startBlock = FCBBlockNum;
	//printf("fileName, type:%s %c\n", newDirUnit->fileName, newDirUnit->type);

	//Comment if mem swap ready
	writeBlock(myDirTable->startBlock, myDirTable);
	
	return 0;
}


//ɾ���ļ� rm
int deleteFile(char fileName[])
{
	//����ϵͳ���Զ������ĸ�Ŀ¼
	if (strcmp(fileName, "..") == 0)
	{
		printf("can't delete ..\n");
		return -1;
	}
	//�����ļ���Ŀ¼��λ��
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//�ж�����
	if (myUnit.type != FILE)
	{
		printf("not a file\n");
		return -1;
	}
	int FCBBlock = myUnit.startBlock;
	//�ͷ��ڴ�
	releaseFile(FCBBlock);
	//��Ŀ¼�����޳�
	deleteDirUnit(currentDirTable, unitIndex);

	//comment if mem swap ready
	writeBlock(currentDirTable->startBlock, currentDirTable);

	return 0;
}

//ɾ������
int deletelink(char* fileName) { //�Ƴ�һ��link
	//����ϵͳ���Զ������ĸ�Ŀ¼
	if (strcmp(fileName, "..") == 0)
	{
		printf("can't delete ..\n");
		return -1;
	}
	//�����ļ���Ŀ¼��λ��
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("link file not found\n");
		return -1;
	}
	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//�ж�����
	if (myUnit.type != LINKFILE)
	{
		printf("not a link file\n");
		return -1;
	}

	FCB* myFCB = my_open(fileName);

	if (myFCB->link < 2) {
		printf("link num below 2\n");
		return -1;
	}
	myFCB->link--;
	
	/*int unitIndex = findUnitInTable(currentDirTable, fileName);
	printf("unitIndex:%d", unitIndex);*/
	deleteDirUnit(currentDirTable, unitIndex);

	//comment if mem swap ready
	writeBlock(currentDirTable->startBlock, currentDirTable);

	return 0;
}

//�ͷ��ļ��ڴ�
int releaseFile(int FCBBlock)
{
	char* page = (char*)malloc(block_szie);
	readBlock(FCBBlock, page);
	FCB* myFCB = (FCB*)page;
	myFCB->link--;  //��������һ
	//�����ӣ�ɾ���ļ�
	if (myFCB->link == 0)
	{
		//�ͷ��ļ������ݿռ�
		releaseBlock(myFCB->blockNum, myFCB->fileSize);
	}
	//�ͷ�FCB�Ŀռ�
	sem_close(myFCB->count_sem);
	sem_close(myFCB->write_sem);
	sem_unlink("count_sem");
	sem_unlink("write_sem");
	releaseBlock(FCBBlock, 1);
	return 0;
}


//ɾ��Ŀ¼��
int deleteDirUnit(dirTable* myDirTable, int unitIndex)
{
	//Ǩ�Ƹ���
	int dirUnitAmount = myDirTable->dirUnitAmount;
	for (int i = unitIndex; i < dirUnitAmount - 1; i++)
	{
		myDirTable->dirs[i] = myDirTable->dirs[i + 1];
	}
	myDirTable->dirUnitAmount--;
	return 0;
}


//ɾ��Ŀ¼ rmdir
int deleteDir(char dirName[])
{
	//����ϵͳ���Զ������ĸ�Ŀ¼
	if (strcmp(dirName, "..") == 0)
	{
		printf("can't delete ..\n");
		return -1;
	}
	//�����ļ�
	int unitIndex = findUnitInTable(currentDirTable, dirName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//�ж�����
	if (myUnit.type == DIRECTORY)//Ŀ¼
	{
		deleteFileInTable(currentDirTable, unitIndex);
	}
	else {
		printf("not a directory\n");
		return -1;
	}
	//��Ŀ¼�����޳�
	deleteDirUnit(currentDirTable, unitIndex);

	//comment if mem swap ready
	writeBlock(currentDirTable->startBlock, currentDirTable);

	return 0;
}


//ɾ���ļ�/Ŀ¼��
int deleteFileInTable(dirTable* myDirTable, int unitIndex)
{
	//�����ļ�
	dirUnit myUnit = myDirTable->dirs[unitIndex];
	//�ж�����
	if (myUnit.type == 0)//Ŀ¼
	{
		//�ҵ�Ŀ¼λ��
		int FCBBlock = myUnit.startBlock;
		char* page = (char*)malloc(block_szie);
		readBlock(FCBBlock, page);
		dirTable* table = (dirTable*)page;
		//�ݹ�ɾ��Ŀ¼�µ������ļ�
		printf("recursively delete files in dir %s\n", myUnit.fileName);
		int unitCount = table->dirUnitAmount;
		for (int i = 1; i < unitCount; i++)//���ԡ�..��
		{
			printf("delete %s\n", table->dirs[i].fileName);
			deleteFileInTable(table, i);
		}
		//�ͷ�Ŀ¼��ռ�
		releaseBlock(FCBBlock, 1);

	}
	else {//�ļ�
	   //�ͷ��ļ��ڴ�
		int FCBBlock = myUnit.startBlock;
		releaseFile(FCBBlock);
	}

	//comment if mem swap ready
	writeBlock(myDirTable->startBlock, myDirTable);

	return 0;
}



//**********************��д����*******************
FCB* my_open(char fileName[])
{
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("File no found\n");
		return NULL;
	}

	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//�ж�����
	if (myUnit.type != FILE)
	{
		printf("Cannot write: Not a file.\n");
		return NULL;
	}

	//���ƿ�
	int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	//printf("startBlock%d", FCBBlock);
	char* page = (char*)malloc(block_szie);
	readBlock(FCBBlock, page);
	FCB* myFCB = (FCB*)page;
	return myFCB;
}


//���ļ�
int my_read(FCB* myFCB, int length)
{
	//int unitIndex = findUnitInTable(currentDirTable, fileName);
	//if (unitIndex == -1)
	//{
	//	printf("file no found\n");
	//	return -1;
	//}
	////���ƿ�
	//int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	//FCB* myFCB = (FCB*)readBlock(FCBBlock);
	myFCB->readptr = 0; //�ļ�ָ������
	//������
	char* page = (char*)malloc(block_szie);
	readBlock(myFCB->blockNum, page);
	char* data = (char*)page;
	
	myFCB->count_sem = sem_open("count_sem", O_CREAT, UNUSED, 0);
	/* ��ȡ��¼������������ */
	int val;
	if (sem_wait(myFCB->count_sem) == -1)
		perror("sem_wait error");
	/* ����ӵ�����Ľ����������ж��Ƿ��ǵ�һ������ */
	/* ����ǵ�һ�����߾͸�������д���� */
	sem_getvalue(myFCB->count_sem, &val);
	//printf("count_sem val:%d\n", val);
	if (val == NUMREADER - 1)
	{
		myFCB->write_sem = sem_open("write_sem", O_CREAT, UNUSED, 0);
		int val;
		sem_getvalue(myFCB->write_sem, &val);
		//printf("write_sem val:%d\n", val);// 1����ǰ����д������

		if (sem_wait(myFCB->write_sem) == -1)
			perror("sem_wait error");
	}

	int dataSize = myFCB->dataSize;
	/* printf("myFCB->dataSize = %d\n", myFCB->dataSize); */
	//�ڲ��������ݳ����£���ȡָ�����ȵ�����
	if (length == -1) {
		length = dataSize;
	}
	for (int i = 0; i < length && myFCB->readptr < dataSize; i++, myFCB->readptr++)
	{
		printf("%c", *(data + myFCB->readptr));
	}
	if (myFCB->readptr == dataSize)//�����ļ�ĩβ��#��ʾ
		printf("#");
	/* ��������ֻ��Ϊ��ģ��༭���Ĺر�֮ǰ������� */
	/* �������ܿ��ƽ��̲��������ͷ��� */
	printf("\ninput a character to end up reading....\n");
	getchar();
	
	sem_post(myFCB->count_sem);

	/* ��������һ�����߾͸����ͷ�д���� */
	// Need debugging here
	sem_getvalue(myFCB->count_sem, &val);
	/*printf("count_sem val:%d\n", val);*/
	if (val == NUMREADER)
	{
		sem_post(myFCB->write_sem);
	}
	printf("\n");

	return 0;
}


//д�ļ�����ĩβд�� write
int my_write(FCB* myFCB, char content[])
{
	//int unitIndex = findUnitInTable(currentDirTable, fileName);
	//if (unitIndex == -1)
	//{
	//	printf("file no found\n");
	//	return -1;
	//}
	////���ƿ�
	//int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	//FCB* myFCB = (FCB*)readBlock(FCBBlock);

	//uncomment if write from begin
	//myFCB->dataSize = 0;
	//myFCB->readptr = 0;

	int contentLen = strlen(content);
	int fileSize = myFCB->fileSize * block_szie;
	char* page = (char*)malloc(block_szie);
	readBlock(myFCB->blockNum, page);
	char* data = (char*)page;
	myFCB->write_sem = sem_open("write_sem", O_CREAT, UNUSED, 0);
	/* ���д���� */
	if (sem_wait(myFCB->write_sem) == -1)
		perror("sem_wait error");
	//�ڲ������ļ��Ĵ�С�ķ�Χ��д��
	for (int i = 0; i < contentLen && myFCB->dataSize < fileSize; i++, myFCB->dataSize++)
	{
		*(data + myFCB->dataSize) = content[i];
	}
	/* ģ��༭��,����д�߲������˳� */
	printf("input a character to end up waiting....\n");
	getchar();
	/* �ͷ�д���� */
	sem_post(myFCB->write_sem);
	if (myFCB->dataSize == fileSize)
		printf("file is full, can't write in\n");
	
	//comment if mem swap ready, rethinking multiple block write?
	writeBlock(myFCB->blockNum, data);

	//comment if mem swap ready, writing changes the value of datasize in FCB, so we should write it to disk also.
	writeBlock(myFCB->startBlock, myFCB);
	
	return 0;
}



//��Ŀ¼�в���Ŀ¼��Ŀ
int findUnitInTable(dirTable* myDirTable, char unitName[])
{
	//���Ŀ¼��
	int dirUnitAmount = myDirTable->dirUnitAmount;
	int unitIndex = -1;
	for (int i = 0; i < dirUnitAmount; i++)//����Ŀ¼��λ��
		if (strcmp(unitName, myDirTable->dirs[i].fileName) == 0)
			unitIndex = i;
	return unitIndex;
}