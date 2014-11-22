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
	unsigned int blockSize, fileSystemSize, FATstart, FATblocks, rootStart, rootBlocks;
	int i;
	//unsigned ints are 4 bytes
 
	//uint32_t

	fseek(diskImage, 8, SEEK_SET);

	printf("Super block information:\n");
    
    fread(&blockSize, 2, 1, diskImage);
    blockSize = ntohs(blockSize);
	printf("Block size: %X\n", blockSize);

	fread(&fileSystemSize, 4, 1, diskImage);
    fileSystemSize = ntohl(fileSystemSize);
	printf("Block count: %X\n", fileSystemSize);

	fread(&FATstart, 1, 4, diskImage);	//might need to be 4, 1 not 1, 4
    FATstart = ntohl(FATstart);
	printf("FAT starts: %X\n", FATstart);

	fread(&FATblocks, 4, 1, diskImage);
	FATblocks = ntohl(FATblocks);
	printf("FAT blocks: %X\n", FATblocks);

	fread(&rootStart, 4, 1, diskImage);
	rootStart = ntohl(rootStart);
	printf("Root directory start: %X\n", rootStart);

	fread(&rootBlocks, 4, 1, diskImage);
	rootBlocks = ntohl(rootBlocks);
	printf("Root directory blocks: %X\n", rootBlocks);

	i = FATstart*512;

	fseek(diskImage, 512, SEEK_SET);

	printf("FAT information: i = %d\n", i);

	//another test comment

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

	fclose(diskImage);
}


/*
Super block information:
Block size: 512
Block count: 5120
FAT starts: 1
FAT blocks: 40
Root directory start: 41
Root directory blocks: 8

FAT information:
Free Blocks: 5071
Reserved Blocks: 41
Allocated Blocks: 8

Description 						Size 	 Default Value
File system identifier  			8 bytes  CSC360FS 
Block size 			    			2 bytes  0x200
File system size (in blocks) 		4 bytes  0x00001400
Block where FAT starts 				4 bytes  0x00000001
Number of blocks in FAT 			4 bytes  0x00000028
Block where root directory starts   4 bytes  0x00000029
Number of blocks in root directory  4 bytes  0x00000008

*/