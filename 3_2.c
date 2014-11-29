/*
Assignment #3
Course: CSC 360 - Operating Systems
Date: December 1, 2014
Description: A Simple File System
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

int flag;	//flag = 1 if part2 defined, flag = 2 if filename is valid in part3

void readThatFile(FILE *diskImage, FILE *copyFile, int **allDirEntry, int *SBI)
{
	//fread the file from the .img into toReadWrite
	//fwrite the info from toREadWrite to the file in the root directory specified by copyFile
	size_t n, m;
	int i = 0;
	int o = 0;
	
	int blockSize = SBI[0];
	
	while(allDirEntry[i][3] != 5 && allDirEntry[i] != NULL){
		i++;
	}
	
	if (allDirEntry[i][3] == 5)	//this is the file to print out
	{
		int numBlocks = allDirEntry[i][1];	//the number of blocks in the file
		int fileSize = allDirEntry[i][2];
		unsigned char toReadWrite[fileSize];
		int bytesLeft = fileSize;
		unsigned int readSequence [numBlocks];
		unsigned int dStartBlock = allDirEntry[i][0];
		readSequence[o] = dStartBlock;
		
		while(readSequence[o] != 0xFFFFFFFF && bytesLeft > 0)
		{
			fseek(diskImage, SBI[2]*blockSize + readSequence[o]*4, SEEK_SET);
			o++;
			fread(&readSequence[o], 4, 1, diskImage);
			readSequence[o] = ntohl(readSequence[o]);
			bytesLeft--;
		}

		for (i = 0; i<numBlocks; i++)
		{
			fseek(diskImage, readSequence[i]*blockSize, SEEK_SET);	//move to the next block in the file to be read
			n = fread(toReadWrite, 1, blockSize, diskImage);	//read the next block of the file
			
			if(n != 0)
			{
				m = fwrite(toReadWrite, 1, n, copyFile);	//write the next block to the specified file
			}
			else
			{
				m = 0;
			}
		}
	}
	
} //end function

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

int **readDirectory(FILE *diskImage, int *SBI, int **allDirEntry, char *name)
{
	
	unsigned int numBlocks, fileSize, tempFN, modYear, dStartBlock;
	double createTime, unusedSpace;
	char fileName[31];
	int i, g;
	int l = 9;
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
		allDirEntry[k][3] = 0;
		k++;
	    
	    if (flag == 1)	//function called from part 2 so print information
	    {
			printf("%c %10d %30s %04d/%02d/%02d %02d:%02d:%02d\n", status, fileSize, fileName, modYear, modMonth, modDay, modHour, modMinute, modSecond);
	    }

	    if(name != NULL)
	    {
	    	l = memcmp(fileName, name, strlen(fileName));
	    }

	    if (l == 0)
	    {
	    	flag = 2;
	    	allDirEntry[k-1][3] = 5;	//set a flag to show this is the file to use in diskget
	    }
	    
	}
	allDirEntry[k] = NULL;

	return allDirEntry;
}

int main (int argc, char *argv[])
{
	FILE *diskImage;
	
	int *SBI = calloc(9, sizeof(int));	//pointer to the start of the array storing the superblockinfo
	
	char *name = argv[2];

	diskImage = fopen(argv[1], "rb");	//read the file

	if (diskImage == NULL)
	{
		perror("Error: ");
		return(-1);
	}

	SBI = readSuperBlockInfo(diskImage);
	int temp = SBI[5]*8;	//number of blocks in the root directory * number of entries per block
	int ii;
	//allocate memory for array holding file info arrays
	int **allDirEntry = calloc(temp, sizeof(int *)); 
	for(ii = 0; ii < temp; ii++) 
	{ 
   		allDirEntry[ii] = calloc(4, sizeof(int));
	}

#if defined(PART1)
	name = NULL;
	printf ("\nPart 1: diskinfo\n");
	printSuperBlockInfo(SBI);

#elif defined(PART2)
	flag = 1;
	printf ("Part 2: disklist\n");
	
	//reads and prints directory information if flag is set
	allDirEntry = readDirectory(diskImage, SBI, allDirEntry, name);
	
#elif defined(PART3)
	FILE *copyFile;
	flag = 0;
	
	printf ("Part 3: diskget\n");
	
	allDirEntry = readDirectory(diskImage, SBI, allDirEntry, name);

	if (flag != 2)
	{
		printf("File not found\n");
		return(-1);
	}
	
	copyFile = fopen(argv[2], "wb");

	if (copyFile == NULL)
	{
		perror("Error: ");
		return(-1);
	}

	readThatFile(diskImage, copyFile, allDirEntry, SBI);
	fclose(copyFile);

#elif defined(PART4)

	name = NULL;
	printf ("Part 4: diskput\n");
#else
#	error "PART[1234] must be defined"
#endif
	fclose(diskImage);
	
	//free(SBI);	gives an error...
	free(allDirEntry);
	
	return 0;
}