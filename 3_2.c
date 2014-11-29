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

FILE *copyFile;

/*void readThatFile(FILE *diskImage, unsigned char dStartBlock, unsigned char fileSize, unsigned char numBlocks)
{
	//fread the file from the .img
	//fwrite the info to the file in the root directory
	char toReadWrite[blockSize];
	unsigned int nextBlock = dStartBlock;	//next block from FAT, each FAT entry is 4 bytes, initally set so the while loop must go through once
	unsigned char fNextBlock = dStartBlock;	//the next block in the file to read, intitally set to the first block to read
	
	while (nextBlock != 0xFFFFFFFF)
	{
		if (nextBlock == 0x00000000)	//this block is available
		{
			printf("Error Reading FAT Block in Part 3: Block should be part allocated to a file but instead is available\n");
		}
		else if(nextBlock == 0x00000001)	//this block is reserved
		{
			printf("Error Reading FAT Block in Part 3: Block should be part allocated to a file but instead is reserved\n");
		}
		else if ((nextBlock >= 0x00000002 && nextBlock <= 0xFFFFFF00) || nextBlock == -1)	//this block is allocated as part of a file
		{
			fNextBlock = nextBlock;
		}
		else if (nextBlock == 0xFFFFFFFF)	//this is the last block in a file and therefore allocated to the file
		{
			fNextBlock = nextBlock;
		}
		else
		{
			printf("Error Reading FAT in Part 3: FAT entry is invalid\n");
		}
		
		//move file pointer to the next block in the file to read
		fseek(diskImage, (fNextBlock*blockSize), SEEK_SET);
		//read the block 
	//might have to ntohl the read and write
		fread(&toReadWrite, 1, blockSize, diskImage);
		//write the block to the file
		fwrite(&toReadWrite, 1, blockSize, copyFile);
		//check FAT at current block of file
		fseek(diskImage, (FATstart*blockSize) + fNextBlock, SEEK_SET);
		//look at that block in the FAT to find the next block in the file
		fread(&nextBlock, 4, 1, diskImage);
		nextBlock = ntohl(nextBlock);
	}

}*/

//function to read the information in the superblock as per part 1
//returns the pointer to an array containing the data read
int *readSuperBlockInfo(FILE *diskImage)
{
	unsigned int blockSize, fileSystemSize, FATstart, FATblocks, rootStart, rootBlocks, temp;
	int freeBlocks = 0;
	int resBlocks = 0;
	int alloBlocks = 0;
	int d = 0;
	int i;
	static int superBlockInfo[9];	//contains the info read in this function in the order it's read
	//unsigned ints are 4 bytes
	//short is 2 bytes
	//long is 4 bytes

	fseek(diskImage, 8, SEEK_SET);
    
    fread(&blockSize, 2, 1, diskImage);
    blockSize = ntohs(blockSize);
    superBlockInfo[d] = blockSize;
    d++;
	
	fread(&fileSystemSize, 4, 1, diskImage);
    fileSystemSize = ntohl(fileSystemSize);
    superBlockInfo[d] = fileSystemSize;
    d++;
	
	fread(&FATstart, 4, 1, diskImage);
    FATstart = ntohl(FATstart);
    superBlockInfo[d] = FATstart;
    d++;
	
	fread(&FATblocks, 4, 1, diskImage);
	FATblocks = ntohl(FATblocks);
	superBlockInfo[d] = FATblocks;
	d++;

	fread(&rootStart, 4, 1, diskImage);
	rootStart = ntohl(rootStart);
	superBlockInfo[d] = rootStart;
	d++;

	fread(&rootBlocks, 4, 1, diskImage);
	rootBlocks = ntohl(rootBlocks);
	superBlockInfo[d] = rootBlocks;
	d++;
	

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
    		printf("Error Reading FAT in Part 1: FAT entry is invalid\n");
    	}
	}

	superBlockInfo[d] = freeBlocks;
	d++;
	superBlockInfo[d] = resBlocks;
	d++;
	superBlockInfo[d] = alloBlocks;
	return superBlockInfo;
}

void printSuperBlockInfo(int *SBI)
{
	int i = 0;

	printf("\nSuper block information:\n");
	printf("Block size: %d\n", SBI[i]);	// i = 0
	i++;
	printf("Block count: %d\n", SBI[i]);	// i = 1
	i++;
	printf("FAT starts: %d\n", SBI[i]);	// i = 2
	i++;
	printf("FAT blocks: %d\n", SBI[i]);	// i = 3
	i++;
	printf("Root directory start: %d\n", SBI[i]);	// i = 4
	i++;
	printf("Root directory blocks: %d\n", SBI[i]);	// i = 5
	i++;
	printf("\nFAT information: \n");	
	printf("Free Blocks: %d\n", SBI[i]);	// i = 6
	i++;
	printf("Reserved Blocks: %d\n", SBI[i]);	// i = 7
	i++;
	printf("Allocated Blocks: %d\n\n", SBI[i]);	// i = 8
	i++;
}

int **readDirectory(FILE *diskImage, int *SBI, int **allDirEntry, int flag)
{
	
	unsigned int numBlocks, fileSize, tempFN, modYear, dStartBlock;
	double createTime, unusedSpace;
	char fileName[31];
	int i, g;
	int k = 0;
	unsigned char statusRead, modMonth, modDay, modHour, modMinute, modSecond;
	char status = 'x';
	int rootStart = SBI[4];
	int blockSize = SBI[0];
	int rootBlocks = SBI[5];

	for (g = 0; g < rootBlocks*8; g++)	//rootBlocks*8, 8 directory entries/root block
	{
		//move file pointer to the start of the next root directory to read
		fseek(diskImage, (rootStart*blockSize) + g*64, SEEK_SET);
	   
	    fread(&statusRead, 1, 1, diskImage);

		if(!statusRead & 1)	//bit 0 is set to 0, this directory entry is available
	    {
			status = 'a';
			continue;
	    }
	    else if (statusRead & 1)	//bit 0 is set to 1, this directory entry is in use
		{
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

		fread(&dStartBlock, 4, 1, diskImage);		    
	    dStartBlock = ntohl(dStartBlock);

		fread(&numBlocks, 4, 1, diskImage);
	    numBlocks = ntohl(numBlocks);

		fread(&fileSize, 4, 1, diskImage);
	    fileSize = ntohl(fileSize);

		fread(&createTime, 7, 1, diskImage);
	    createTime = ntohl(createTime);

		fread(&modYear, 2, 1, diskImage);
	    modYear = ntohs(modYear);
	    
	    //check if this is less than 10, if so pad with %0d if not just use %d to print
	    fread(&modMonth, 1, 1, diskImage);

	    fread(&modDay, 1, 1, diskImage);
	    
	    fread(&modHour, 1, 1, diskImage);
	    
	    fread(&modMinute, 1, 1, diskImage);
	    
	    fread(&modSecond, 1, 1, diskImage);

		for (i = 0; i < 31; i++)
		{
			fread(&tempFN, 1, 1, diskImage);
			fileName[i] = tempFN;
		}
		
		fread(&unusedSpace, 6, 1, diskImage);		
		allDirEntry[k][0] = dStartBlock;	
		allDirEntry[k][1] = numBlocks;
		allDirEntry[k][2] = fileSize;
		k++;
	    
	    if (flag == 1)	//function called from part 2 so print information
	    {
			printf("%c %10d %30s %04d/%02d/%02d %02d:%02d:%02d\n", status, fileSize, fileName, modYear, modMonth, modDay, modHour, modMinute, modSecond);
	    }
	    
	}

	return allDirEntry;
}

int main (int argc, char *argv[])
{
	FILE *diskImage;
	int *SBI = calloc(9, sizeof(int));	//pointer to the start of the array storing the superblockinfo
	int flag = 0;	//flag = 1 if part2 defined
	diskImage = fopen(argv[1], "rb");	//read the file

	if (diskImage == NULL)
	{
		perror("Error: ");
		return(-1);
	}

#if defined(PART1)
	printf ("\nPart 1: diskinfo\n");
	SBI = readSuperBlockInfo(diskImage);
	printSuperBlockInfo(SBI);

#elif defined(PART2)
	flag = 1;
	SBI = readSuperBlockInfo(diskImage);
	int temp = SBI[5]*8;

	//allocate memory for array holding file info arrays
	int ii;
	int **allDirEntry = calloc(temp, sizeof(int *)); //number of blocks in the root directory * number of entries per block
	for(ii = 0; ii < temp; ii++) 
	{ 
   		allDirEntry[ii] = calloc(3, sizeof(int));
	}

	printf ("Part 2: disklist\n");

	//reads and prints directory information if flag is set
	allDirEntry = readDirectory(diskImage, SBI, allDirEntry, flag);
	
#elif defined(PART3)
	printf ("Part 3: diskget\n");
	
	copyFile = fopen(argv[2], "r+b");

	if (copyFile == NULL)
	{
		printf("File not found\n");
		return(-1);
	}
#elif defined(PART4)
	printf ("Part 4: diskput\n");
#else
#	error "PART[1234] must be defined"
#endif
	fclose(diskImage);
	return 0;
}


/*
In part III, you will write a program that copies a file from the file system to the current directory in Linux. If the specified file is not found in the root directory of the file system, you should output the message File not found. and exit.
Your program for part III will be invoked as follows:
./diskget disk1.img foo.txt

*/