#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>

#define SIZE 32

int main()
{
	int fd,ret;
	char buffR[SIZE]; // read buffer
	
	fd = open("/dev/sample0",O_RDWR); // sample0 c 42 0 // fd represents open file 
      	if(fd < 0)
	{
		perror("Error Opening \n");
		return fd;
	}	
	
	ret = read(fd,buffR,SIZE);

	printf("Got : %s\n No.of Bytes Read : %d\n",buffR,ret); 
	return 0; // here close function will be called implicitly by the Os 

}
