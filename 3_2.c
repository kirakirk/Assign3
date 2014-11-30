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

void writeThatFile(FILE *diskImage, FILE *copyFile, int *SBI, char *name)
{
	
	time_t createTime, modTime;
	struct tm *info;
	double unusedSpace = 0xFF;
	unsigned int fileSize, numBlocks, temp, numBlocks2, cYear, modYear; 
	unsigned int blockNumber = 0;
	unsigned int FATblocks = SBI[3];	//number of blocks in the FAT
	unsigned int EOFBlock = 0xFFFFFFFF;
	int FATblocksLeft = FATblocks*128;
	int blockSize = SBI[0];
	int i = 0;
	int k = SBI[5]; //k = number of blocks in the root directory
	int j = 0;	//to loop through readsequence array
	//unsigned char toReadWrite[blockSize];
	unsigned char statusRead = 0;
	unsigned char status = 3;	//we won't be working with directories, so every status will be 11000000
	char cMonth, cDay, cHour, cMin, cSec, modMonth, modDay, modHour, modMin, modSec;
	size_t m;	//n

	fseek(copyFile, 0, SEEK_END);	//seek to the end of the file
	fileSize = ftell(copyFile);	 //figure out the size of the file in bytes

	printf("fileSize = %d\n", fileSize);
	
	numBlocks = fileSize/512;	//file size in blocks
	numBlocks2 = numBlocks;	//to count down the number of blocks in the file
	
	unsigned int readSequence [numBlocks];	//an array to hold the sequence in the FAT
	
	if(numBlocks > SBI[6])	//check if the number of blocks in the file is bigger than the number of free blocks in the file system
	{
		printf("Error: There Are Not Enough Free Blocks In The File System To Write That File\n");
	}

//add the blocks of the file to the FAT
	//move file pointer to the start of the FAT to find first available block
	fseek(diskImage, (SBI[2]*blockSize), SEEK_SET);

//printf("SBI[2] = %d, blockSize = %d\n", SBI[2], blockSize);
	
	
	while (numBlocks2 > 0 && FATblocksLeft > 0)//we still have blocks in the file left to write and there are still blocks to read
	{			
		//read the FAT entries to fill in the read sequence
		//fread moves the file pointer with it, to read the entries one by one
		fread(&temp, 4, 1, diskImage);
		blockNumber++;
    	temp = ntohl(temp);
		
		if (temp == 0x00000000)	//the block is available
		{
			//printf("temp = %x, %d, blockNumber = %d\n", temp, temp, blockNumber);
			readSequence[j] = blockNumber;	//add the block to the array
			j++;
			numBlocks2--;	//one less block from the file to read
		}
		
		FATblocksLeft--;	//one less block in the FAT to read
		//printf("FATblocksLeft = %d, numBlocks2 = %d\n", FATblocksLeft, numBlocks2);
	}

	//fill the FAT with the information from the read sequence array
	//reset counters
	j = 0;	
	numBlocks2 = numBlocks;
	//seek to the first available block
	while(numBlocks2 > 1)
	{
		fseek(diskImage, readSequence[j], SEEK_SET);
		//write to that block where to find the next block
		m = fwrite(&readSequence[j+1], 4, 1, diskImage);
		j++;
		numBlocks2--;
	}
	//write the EOF block
	fseek(diskImage, readSequence[j], SEEK_SET);
	m = fwrite(&EOFBlock, 4, 1, diskImage);

//add the file info to the directory
	
	//find the first available entry in root directory 
    do
    {
    	fseek(diskImage, SBI[4]*blockSize+(64*i), SEEK_SET);
    	fread(&statusRead, 1, 1, diskImage);
    	//printf("in while, statusRead = %d\n", statusRead);
    	i++;
    	k--;
    }while((statusRead & 1) && (k>0));	//bit 0 is set to 1, this directory entry is in use or we made it to the end of the root directory
    
   // printf("k = %d, SBI[4] = %d, i = %d\n", k, SBI[4], i);

    
    fseek(diskImage, -1, SEEK_CUR);
    //change the status
    m = fwrite(&status, 1, 1, diskImage);
    
    //add the starting block		    
	readSequence[0] = htonl(readSequence[0]);
    fwrite(&readSequence[0], 4, 1, diskImage);
    
    //add the number of blocks
    numBlocks = htonl(numBlocks);
    fwrite(&numBlocks, 4, 1, diskImage);
    
    //add the file size (in bytes)
    fileSize = htonl(fileSize);
    fwrite(&fileSize, 4, 1, diskImage);

    //add a create time
	time(&createTime);
	info = localtime(&createTime);
	cYear = info -> tm_year;
	cYear = cYear + 1900;
	cYear = htonl(cYear);
	fwrite(&cYear, 2, 1, diskImage);
	
	cMonth = info -> tm_mon;
	cMonth = cMonth+1;
	fwrite(&cMonth, 1, 1, diskImage);

	cDay = info -> tm_mday;
	fwrite(&cMonth, 1, 1, diskImage);

	cHour = info -> tm_hour;
	fwrite(&cHour, 1, 1, diskImage);

	cMin = info -> tm_min;
	fwrite(&cMin, 1, 1, diskImage);

	cSec = info -> tm_sec;
	fwrite(&cSec, 1, 1, diskImage);

	//add the modify time
	time(&modTime);
	info = localtime(&modTime);
	modYear = info -> tm_year;
	modYear = modYear + 1900;
	modYear = htonl(modYear);
	fwrite(&modYear, 2, 1, diskImage);
	
	modMonth = info -> tm_mon;
	modMonth = modMonth+1;
	fwrite(&modMonth, 1, 1, diskImage);

	modDay = info -> tm_mday;
	fwrite(&modDay, 1, 1, diskImage);

	modHour = info -> tm_hour;
	fwrite(&modHour, 1, 1, diskImage);

	modMin = info -> tm_min;
	fwrite(&modMin, 1, 1, diskImage);

	modSec = info -> tm_sec;
	fwrite(&modSec, 1, 1, diskImage);

	//add the filename
	fwrite(name, 1, 31, diskImage);

	//add the unused space
	fwrite(&unusedSpace, 1, 6, diskImage);
 
	//add the file to the file system following sequence
	//go to the start of the file to copy in
	rewind(copyFile);
	
	/*for (i = 0; i<numBlocks; i++)
	{
		fseek(diskImage, readSequence[i]*blockSize, SEEK_SET);	//move to the next block in the file to be read
		n = fread(toReadWrite, 1, blockSize, copyFile);	//read the next block of the file

		if(n != 0)
		{
			m = fwrite(toReadWrite, 1, n, diskImage);	//write the next block to the specified file
		}
		else
		{
			m = 0;
		}
	}*/

}


void readThatFile(FILE *diskImage, FILE *copyFile, int **allDirEntry, int *SBI)
{
	//fread the file from the .img into toReadWrite
	//fwrite the info from toREadWrite to the file in the root directory specified by copyFile
	size_t n, m;
	int i = 0;
	
	
	int blockSize = SBI[0];
	
	while(allDirEntry[i][3] != 5 && allDirEntry[i] != NULL){
		i++;
	}
	
	int o = 0;
	if (allDirEntry[i][3] == 5)	//this is the file to print out
	{
		int numBlocks = allDirEntry[i][1];	//the number of blocks in the file
		int fileSize = allDirEntry[i][2];	//size of the file in bytes
		unsigned char toReadWrite[blockSize];	//temp variable to hold bytes as they're read, before they're written
		int bytesLeft = fileSize;	//to keep track of the bytes left to read in the file
		unsigned int readSequence [numBlocks];	//to store the sequence of blocks in the file system that the file is stored in, this is read from the FAT
		unsigned int dStartBlock = allDirEntry[i][0];
		readSequence[o] = dStartBlock;
		
		//o constraint may break this, needs to be tested
		//read the block sequence from the FAT into an array
		while(readSequence[o] != 0xFFFFFFFF && bytesLeft > 0 && o < numBlocks-1)
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
	unsigned int numBlocks, fileSize, modYear, dStartBlock;
	double createTime, unusedSpace;
	char fileName[31];
	int g;
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

	    //printf("in readDirectory, statusRead = %d\n", statusRead);

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
	   // printf("startblock = %x, %d\n", dStartBlock, dStartBlock);

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

		fread(fileName, sizeof(fileName), 1, diskImage);
		
		fread(&unusedSpace, 6, 1, diskImage);		
		allDirEntry[k][0] = dStartBlock;	
		allDirEntry[k][1] = numBlocks;
		allDirEntry[k][2] = fileSize;
		allDirEntry[k][3] = 0;
		k++;
	    
	    //printf("%d, %d, %f, %f\n", dStartBlock, numBlocks, createTime, unusedSpace);
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
	FILE *copyFile;

	int *SBI = calloc(9, sizeof(int));	//pointer to the start of the array storing the superblockinfo
	
	char *name = argv[2];

	diskImage = fopen(argv[1], "r+b");	//open the file for reading and writing in binary

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
	copyFile = NULL;
	printf ("\nPart 1: diskinfo\n");
	printSuperBlockInfo(SBI);

#elif defined(PART2)
	flag = 1;
	copyFile = NULL;
	printf ("Part 2: disklist\n");
	
	//reads and prints directory information if flag is set
	allDirEntry = readDirectory(diskImage, SBI, allDirEntry, name);
	
#elif defined(PART3)
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

	printf ("Part 4: diskput\n");

	copyFile = fopen(argv[2], "r+b");

	if (copyFile == NULL)
	{
		printf("File not found\n");
		return(-1);
	}

	writeThatFile(diskImage, copyFile, SBI, name);
	fclose(copyFile);
#else
#	error "PART[1234] must be defined"
#endif
	fclose(diskImage);
	
	//free(SBI);	gives an error...
	free(allDirEntry);
	
	return 0;
}