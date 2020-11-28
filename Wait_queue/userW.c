#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>

#define SIZE 32

int main()
{
	int fd,ret;
	char buffW[] = "fromuser";//write buffer 
	
	fd = open("/dev/sample0",O_RDWR); // sample0 c 42 0 // fd represents open file 
      	if(fd < 0)
	{
		perror("Error Opening \n");
		return fd;
	}	
	
	ret = write(fd,buffW,sizeof(buffW));
	
	printf("Wrote %d Bytes of information.",ret);
	return 0; // here close function will be called implicitly by the Os 

}
