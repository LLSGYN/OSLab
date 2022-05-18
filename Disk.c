#include "Disk.h"

#include<stdio.h>
#include<stdlib.h>



  //系统起始地址
int baseAddr = 0;


//初始化系统
void InitDisk()
{
	/* 分配10M空间 */
	char* systemStartAddr;
	systemStartAddr = (char*)malloc(system_size * sizeof(char));
	memset(systemStartAddr, 0, system_size * sizeof(char));
	//ftruncate(vdisk, 1000);
	/*for (int i = 0; i < system_size; i++)
	{
		fprintf(vdisk, "%c", 0);
	}*/
	//初始化盘块的位示图
	for (int i = 0; i < block_count; i++)
		systemStartAddr[i] = '0';
	//用于存放位示图的空间已被占用
	int bitMapSize = block_count * sizeof(char) / block_szie;//位示图占用盘块数:10
	for (int i = 0; i < bitMapSize; i++)//从零开始分配
		systemStartAddr[i] = '1';   //盘块已被使用
	FILE *vdisk;
	vdisk = fopen("VDISK.osp", "w+");
	fwrite(systemStartAddr, block_szie, block_count, vdisk);
	fclose(vdisk);
	free(systemStartAddr);
}

//void LoadDisk() {
//	vdisk = fopen("VDISK.osp", "rb+");
//}

//退出系统
//void exitSystem()
//{
//	free(systemStartAddr);
//}

//磁盘分配
int getBlock(int blockSize)
{
	int startBlock = 0;
	int sum = 0;
	FILE *vdisk;
	vdisk = fopen("VDISK.osp", "r+");
	for (int i = 0; i < block_count; i++)
	{
		//printf("pos:%ld", ftell(vdisk));
		char c = fgetc(vdisk);
		if (c == '0')//可用盘块
		{
			//printf("start block %d", i);
			if (sum == 0)//刚开始，设置开始盘块号
				startBlock = i;
			sum++;
			if (sum == blockSize)//连续盘块是否满足需求
			{
				//当访问到的字符为0时，文件指针已经后移一位，即为11111100的第二个0，此时将其写入1会变成11111101，无法获取正确磁盘块。因此需要将文件指针固定在当前第一个0的位置。
				fseek(vdisk, startBlock, SEEK_SET);
				//满足分配，置1
				for (int j = startBlock; j < startBlock + blockSize; j++) {
					fputc('1', vdisk);
				}
					
				//fflush(vdisk);
				fclose(vdisk);
				return startBlock;
			}

		}
		else//已被使用,连续已经被打断
			sum = 0;
	}
	fclose(vdisk);
	printf("not found such series memory Or memory is full\n");
	return -1;
}
//获得盘块的物理地址
//char* getBlockAddr(int blockNum)
//{
//	FILE* fp;
//	fp = fopen("VDISK.osp", "rb+");
//	fseek(fp, blockNum * block_szie, SEEK_SET);
//	fread(page, block_szie, 1, fp);
//	return page; 
//	return systemStartAddr + blockNum * block_szie; //偏移量单位为字节
//}

int getBlockAddr(int blockNum) {
	return blockNum * block_szie;
}

int readBlock(int blockNum, char* page) {
	//page size should be 1024B 
	//page def: char* page = (char*)malloc(system_size * sizeof(char));
	int blockAddr = getBlockAddr(blockNum);
	FILE* vdisk;
	vdisk = fopen("VDISK.osp", "rb+");
	fseek(vdisk, blockAddr, SEEK_SET);
	fread(page, block_szie, 1, vdisk);
	return page;
}

int writeBlock(int blockNum, char* page) {
	//page size should be 1024B
	int blockAddr = getBlockAddr(blockNum);
	FILE* vdisk;
	vdisk = fopen("VDISK.osp", "rb+");
	fseek(vdisk, blockAddr, SEEK_SET);
	//char* page = (char*)malloc(system_size * sizeof(char));
	fwrite(page, block_szie, 1, vdisk);
	return 0;
}


//获得物理地址的盘块号
int getAddrBlock(int addr)
{
	return (addr - baseAddr) / block_szie;
}

//释放盘块、
int releaseBlock(int blockNum, int blockCount)
{
	int endBlock = blockNum + blockCount;
	//修改位示图盘块的位置为0
	FILE* fp;
	fp = fopen("VDISK.osp", "rb+");
	fseek(fp, blockNum, SEEK_SET);
	for (int i = blockNum; i < endBlock; i++)
		 fputc('0',fp);
	fclose(fp);
	return 0;
}
