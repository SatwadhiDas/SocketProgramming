#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXPENDING 5
#define BUFFERSIZE 32
#define PDR 10
struct message{
	int size;
	int seq_no;
	int final;
	int dataOrAck;
	int id;
	char data[101];
};

struct ack{
int seq_no;
};


void die(char *s){
	perror(s);
	exit(1);
}
	
int main()
{
	int serverSocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
	char buffer[1000];
	int offset=0;
	int temp_offset=0;
	if (serverSocket < 0) 
	{
		printf ("Error while server socketcreation"); 
		exit (0); 
	}
	printf ("Server Socket Created\n");
	
	struct sockaddr_in serverAddress, clientAddress[2];
	memset (&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(12345);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	
	printf ("Server address assigned\n");

		int temp = bind(serverSocket, (struct sockaddr*) &serverAddress,sizeof(serverAddress));
		
		if(temp < 0)
	{ 
		printf ("Error while binding\n");
		exit (0);
	}

	printf ("Binding successful\n");
	
	int temp1 = listen(serverSocket, MAXPENDING);
	if(temp1 < 0)
	{ 
		printf ("Error in listen");
		exit (0);
	}

	printf ("Now Listening\n");

	

	int clientLength[2];
	clientLength[0] = sizeof(clientAddress[0]);
	clientLength[1] = sizeof(clientAddress[1]);
	int clientSocket[2];
	clientSocket[0] = accept (serverSocket, (struct sockaddr*)&clientAddress[0], &clientLength[0]);
	clientSocket[1] = accept (serverSocket, (struct sockaddr*)&clientAddress[1], &clientLength[1]);
	
	int max_cs=clientSocket[0];
	if(max_cs<clientSocket[1])
		max_cs=clientSocket[1];

	if (clientLength[0] < 0 || clientLength[1] < 0 ) 
	{
		printf ("Error in client socket"); 
		exit(0);
	}

	FILE *fp=fopen("destination.txt","wb");
	
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(clientSocket[0],&fds);
	FD_SET(clientSocket[1],&fds);
	
	struct ack rec;
	rec.seq_no=0;
	int end=0;
	while(1)
	{
	
	if(end==1 && temp_offset==0)
		break;

	 temp = select( max_cs+1,&fds,NULL,NULL,NULL);
	if(temp==-1)
		die("Receiving");
	struct message packet;
	for(int i=0;2>i;i++)
	{
		if(FD_ISSET(clientSocket[i],&fds))
		{
			temp=recv(clientSocket[i],&packet,sizeof(packet),0);
			printf("RCVD PKT:  Seq. No. %d of size %d Bytes from channel %d \n",packet.seq_no,packet.size,packet.id);	
			//printf("Received \n");
			if(temp<0)
				die("Receiving");
			
			int rnd=rand()%101;
			if(rnd<PDR)
			{
			
			FD_ZERO(&fds);
			FD_SET(clientSocket[0],&fds);
			FD_SET(clientSocket[1],&fds);
			break;
			}
			
			if(packet.seq_no==offset && temp_offset==0)
			{
			fwrite(packet.data,1,packet.size,fp);
			offset+=packet.size;
			}
			
			else if(packet.seq_no==offset && temp_offset!=0)
			{
			fwrite(packet.data,1,packet.size,fp);
			offset+=packet.size;
			
			fwrite(buffer,1,temp_offset,fp);
			offset+=temp_offset;
			temp_offset=0;
			memset (buffer,0,1000);
			}
			else
			{
			//printf("here");
			strcat(buffer,packet.data);
			temp_offset+=packet.size;
			}
			
			if(packet.final==1)
				end=1;
				
			rec.seq_no=packet.seq_no;
			
			temp=send(clientSocket[i], &rec,sizeof(rec),0);	
			printf("SENT ACK: for PKT with Seq. No. %d from channel %d \n",rec.seq_no,i);
			if(temp<0)
				die("Sending");
			FD_ZERO(&fds);
			FD_SET(clientSocket[0],&fds);
			FD_SET(clientSocket[1],&fds);
			break;
		}
	
	}
	}

	close(serverSocket);
	fclose(fp);
		
	
	exit(0);
	
	
}
