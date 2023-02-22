#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ioctl_example.h"

int main() {
	int fp;
	char dataToWrite[8], dataToRead[8], c, i;
	char answer;
	
	dataToWrite[0] = 'H';
	dataToWrite[1] = 'E';
	dataToWrite[2] = 'L';
	dataToWrite[3] = 'L';
	dataToWrite[4] = 'O';
	dataToWrite[5] = '-';
	dataToWrite[6] = '1';
	dataToWrite[7] = '2';
	
	fp = open("/dev/ioctl-example", O_RDWR);
	if(fp == -1) {
		printf("Opening was not possible!\n");
		return -1;
	}
	printf("Opening was successfull!\n");
	
	write(fp, dataToWrite, 8);
	
	printf("Writing was successful\n");
	
	read(fp, dataToRead, 8);
	
	printf("\nReading was successful\n%s\n", dataToRead);
	
	ioctl(fp, RD_VALUE, &answer);
	printf("The answer is %c\n", answer);
	
	answer = 98;
	ioctl(fp, WR_VALUE, &answer);
	ioctl(fp, RD_VALUE, &answer);
	printf("The answer is now %c\n", answer);
	
	printf("Closing the file\n");
	
	close(fp);
	return 0;
}

