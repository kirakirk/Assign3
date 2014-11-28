/*
Assignment #3
Course: CSC 360 - Operating Systems
Date: December 1, 2014
Description: A Simple File System - Part I
Submitted by: Kira Kirk - V00705087
*/

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

unsigned int rootStart, rootBlocks, blockSize;

void readSuperBlockInfo(FILE *diskImage)
{
	unsigned int fileSystemSize, FATstart, FATblocks, temp;
	int freeBlocks = 0;
	int resBlocks = 0;
	int alloBlocks = 0;
	int i;
	//unsigned ints are 4 bytes
	//short is 2 bytes
	//long is 4 bytes

	fseek(diskImage, 8, SEEK_SET);

	printf("\nSuper block information:\n");
    
    fread(&blockSize, 2, 1, diskImage);
    blockSize = ntohs(blockSize);
	printf("Block size: %d\n", blockSize);

	fread(&fileSystemSize, 4, 1, diskImage);
    fileSystemSize = ntohl(fileSystemSize);
	printf("Block count: %d\n", fileSystemSize);

	fread(&FATstart, 1, 4, diskImage);	//might need to be 4, 1 not 1, 4
    FATstart = ntohl(FATstart);
	printf("FAT starts: %d\n", FATstart);

	fread(&FATblocks, 4, 1, diskImage);
	FATblocks = ntohl(FATblocks);
	printf("FAT blocks: %d\n", FATblocks);

	fread(&rootStart, 4, 1, diskImage);
	rootStart = ntohl(rootStart);
	printf("Root directory start: %d\n", rootStart);

	fread(&rootBlocks, 4, 1, diskImage);
	rootBlocks = ntohl(rootBlocks);
	printf("Root directory blocks: %d\n\n", rootBlocks);

	//move file pointer to the start of the FAT
	fseek(diskImage, (FATstart*blockSize), SEEK_SET);

	//read through the FAT to get the information
	//there might be a better way to do this
	for (i = 0; i < fileSystemSize; i++)
	{
		fread(&temp, 4, 1, diskImage);
    	temp = ntohl(temp);
    	if (temp == 0x00000000)	//this block is available
    	{
    		freeBlocks++;
    	}
    	else if(temp == 0x00000001)	//this block is reserved
    	{
    		resBlocks++;
    	}
    	else if (temp >= 0x00000002 && temp <= 0xFFFFFF00)	//this block is allocated as part of a file
    	{
    		alloBlocks++;
    	}
    	else if (temp == 0xFFFFFFFF)	//this is the last block in a file and therefore allocated to the file
    	{
    		alloBlocks++;
    	}
    	else
    	{
    		printf("Error Reading FAT: FAT entry is invalid\n");
    	}
	}
	
	printf("FAT information: \n");	
	printf("Free Blocks: %d\n", freeBlocks);
	printf("Reserved Blocks: %d\n", resBlocks);
	printf("Allocated Blocks: %d\n", alloBlocks);
}

void readDirectory(FILE *diskImage)
{
	unsigned int dStartBlock, numBlocks, fileSize, tempFN, modYear;
	double createTime, unusedSpace;
	//typedef uint8_t BYTE;
	//BYTE fileName [32];
	char fileName[31];
	int i;
	unsigned char statusRead, modMonth, modDay, modHour, modMinute, modSecond;
	char status = 'x';

	//move file pointer to the start of the root directory
	fseek(diskImage, (rootStart*blockSize), SEEK_SET);

	printf("\nRoot directory information:\n");
    
    fread(&statusRead, 1, 1, diskImage);
    //statusRead = ntohs(statusRead);
    //printf("statusRead: %d\n", statusRead);

	if(!statusRead & 1)	//bit 0 is set to 0, this directory entry is available
    {
    	status = 'a';
    }
    else if (statusRead & 1)	//bit 0 is set to 1, this directory entry is in use
	{
		status = 'u';

		//check the use
		if (statusRead & 2)	//bit 1 is set to 1, this entry is a normal file
		{
			status = 'F';
		}
		else if (statusRead & 4)	//bit 2 is set to 1, this entry is a directory
		{
			status = 'D';
		}
		else
		{
			printf("Error in Status bit of directory block\n");
		}
	}
	else
	{
		printf("Error in Status bit of directory block\n");
	}
	
	printf("Status: %c\n", status);
	
	fread(&dStartBlock, 4, 1, diskImage);
    dStartBlock = ntohl(dStartBlock);
	printf("Starting Block: %d\n", dStartBlock);

	fread(&numBlocks, 4, 1, diskImage);
    numBlocks = ntohl(numBlocks);
	printf("Number of Blocks: %d\n", numBlocks);

	fread(&fileSize, 4, 1, diskImage);
    fileSize = ntohl(fileSize);
	printf("File Size: %d\n", fileSize);

	fread(&createTime, 7, 1, diskImage);
    createTime = ntohl(createTime);
	printf("Create Time: %f\n", createTime);

	fread(&modYear, 2, 1, diskImage);
    modYear = ntohs(modYear);
    
    //check if this is less than 10, if so pad with %0d if not just use %d to print
    fread(&modMonth, 1, 1, diskImage);
   /* modMonth = ntohs(modMonth);
    if (modMonth<10)
    {
    	printf("Month less: %d\n", modMonth);
    }
    else
    {
    	printf("Month: %d\n", modMonth);
    }*/

    fread(&modDay, 1, 1, diskImage);
    //modDay = ntohs(modDay);
    
    fread(&modHour, 1, 1, diskImage);
    //modHour = ntohs(modHour);
    
    fread(&modMinute, 1, 1, diskImage);
    //modMinute = ntohs(modMinute);
    
    fread(&modSecond, 1, 1, diskImage);
    //modSecond = ntohs(modSecond);
	
	

	//printf("File Name: \n");
	
	for (i = 0; i < 31; i++)
	{
		fread(&tempFN, 1, 1, diskImage);
		//tempFN = ntohs(tempFN);
		fileName[i] = tempFN;
		//printf("%c", fileName[i]);
		
	}
	//printf(" %04d/%02d/%02d %02d:%02d:%02d\n", modYear, modMonth, modDay, modHour, modMinute, modSecond);
	//fileNamePrintable[i] = '\0';
	//printf("\nfileNamePrintable: %s", fileNamePrintable);
    //printf("\n");
	fread(&unusedSpace, 6, 1, diskImage);
    printf("%c %10d %30s %04d/%02d/%02d %02d:%02d:%02d\n", status, fileSize, fileName, modYear, modMonth, modDay, modHour, modMinute, modSecond);
    //char array[31] = "Hello World";
    //printf("array: %s\n", array);
    //unusedSpace = ntohl(unusedSpace);
	//printf("Unused: %f\n", unusedSpace);
}


int main (int argc, char *argv[])
{
	
	FILE *diskImage;
	
	diskImage = fopen(argv[1], "rb");	//read the file

	if (diskImage == NULL)
	{
		perror("Error: ");
		return(-1);
	}

//#if defined(PART1)
	printf ("Part 1: diskinfo\n");
	readSuperBlockInfo(diskImage);
//#elif defined(PART2)
	printf ("Part 2: disklist\n");
	readDirectory(diskImage);
/*#elif defined(PART3)
	printf ("Part 3: diskget\n");
#elif defined(PART4)
	printf ("Part 4: diskput\n");
#else
#	error "PART[1234] must be defined"
#endif*/
	fclose(diskImage);
	return 0;
}


/*
block 0 = superblock
block 2 = FAT for 32 blocks, maybe not consecutive
block 35 = root directory for 8 blocks, maybe not consecutive


Block size: 200
file system size in blocks = Block count: 1900
FAT starts: 2
FAT blocks: 32
Root directory start: 35
Root directory blocks: 8

512 byte blocks

Description 						Size 	 Default Value
File system identifier  			8 bytes  CSC360FS 
Block size 			    			2 bytes  0x200
File system size (in blocks) 		4 bytes  0x00001400
Block where FAT starts 				4 bytes  0x00000001
Number of blocks in FAT 			4 bytes  0x00000028
Block where root directory starts   4 bytes  0x00000029
Number of blocks in root directory  4 bytes  0x00000008

*/