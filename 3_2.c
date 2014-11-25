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

void readSuperBlockInfo(FILE *diskImage)
{
	unsigned int blockSize, fileSystemSize, FATstart, FATblocks, rootStart, rootBlocks, temp;
	int freeBlocks = 0;
	int resBlocks = 0;
	int alloBlocks = 0;
	int i;
	//unsigned ints are 4 bytes

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


int main (int argc, char *argv[])
{
	FILE *diskImage;
	
	diskImage = fopen(argv[1], "rb");	//read the file

	if (diskImage == NULL)
	{
		perror("Error: ");
		return(-1);
	}

	readSuperBlockInfo(diskImage);

	//readFATInfo(diskImage);

	fclose(diskImage);
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