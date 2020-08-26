#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <sys/wait.h>
#include <fcntl.h>
void child(int sd)
{
	char message[255], filename[50];
	char buffer[255];
	int n1; 
	while(1)
	{
		fprintf(stderr, "Enter a message to send to the client\n");
		fgets(message, 254, stdin);
		write(sd, message, strlen(message)+1);

		if(!read(sd, message, 255))
		{
			close(sd);
			fprintf(stderr,"Bye, client dead, wait for a new client\n");
			exit(0);
		}
		fprintf(stderr, "Client's message or request: %s\n", message);
		int m,k=0, commandLength = 3;
		char command[3];
		for(m=0; m < commandLength ; m++)
		{
			command[m] = message[k]; k++;		
		}		
		int length = strlen(message);
		int i,j=0;
		for(i=commandLength +1; i < length -1; i++)
		{
			filename[j] = message[i]; j++;		
		}

		if(strncmp(command,"get", strlen("get"))==0)
		{
			//open the file if exist
			int fd1, fd2;
			if(( fd1 = open(filename, O_RDONLY)) == -1)
			{
				perror("file problem");
				exit(1);
			}
			//read the file and put content in message
			//write(sd, message, strlen(message) + 1);
			while((n1=read(fd1, buffer, 100)) > 0)
			{
				if(write(sd, buffer, n1) != n1)
				{
					perror("writing problem ");
					exit(3);
				}
			}
			printf("File downloaded");	
			//send ascii code 4 i.e Ctrl + D to the client
		}
		else if(strncmp(command,"put", strlen("put"))==0)
		{
			fprintf(stderr, "Client! you can send file content now %s\n", message);
			write(sd, message, strlen(message)+1);
			// file already there on client side. create or open the file that is named
			// and write contents which is there on socket.
			if(!read(sd, message, 255))
			{
				close(sd);
				fprintf(stderr,"Bye, client dead, wait for a new client\n");
				exit(0);
			}
			fprintf(stderr, "Message for input file content is %s\n", message);
			fprintf(stderr, "input file NAME is %s\n", filename);
			int fd3;
			// create or open the specified file and put message in it.
			if(((fd3 = open(filename, O_CREAT|O_WRONLY, 0700)) == -1))
			{
					perror("file problem ");
					exit(1);
			}
			// write message to the file
			if(write(fd3, message, strlen(message)) != strlen(message))
			{
				perror("writing problem ");
				exit(3);
			}
			close(fd3);
			printf("File uploaded");
		}
	}
}
int main(int argc, char *argv[])
{
	int sd, client, portNumber;
	socklen_t len;
	struct sockaddr_in servAdd;
	if(argc != 2)
	{
		printf("Call model: %s <Port #>\n", argv[0]);
		exit(0);
	}
	if ((sd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		fprintf(stderr, "Cannot create socket\n");
		exit(1);
	}
	servAdd.sin_family = AF_INET;
	servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
	sscanf(argv[1], "%d", &portNumber);
	servAdd.sin_port = htons((uint16_t)portNumber);
	bind(sd,(struct sockaddr*)&servAdd,sizeof(servAdd));
	listen(sd, 5);

	while(1)
	{
		client=accept(sd,(struct sockaddr*)NULL,NULL);
		printf("\nNew client connected\n");
		if(!fork())
			child(client);
		close(client);
	}
}