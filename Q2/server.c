#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define WINDOWSIZE 6
#define PACKETSIZE 100
#define TIMER 2

int start_seq=1;
int end=0;

struct message{
	int size;
	int seq_no;
	int final;
	char data[101];
	};
	
void die(char *s){
	perror(s);
	exit(1);
}
	
struct ack
{
int seq_no;
};

void shift_one(struct message *arr, int *status_arr,FILE *fp)
{
	while(status_arr[0]==1)
	{
	if(arr[0].final==1)
		end=1;
	int m=fwrite(arr[0].data,1,arr[0].size,fp);
	//printf("\n \n char count %d\n",m);
//	printf("%d --- %s \n",arr[0].seq_no,arr[0].data);
	start_seq++;
	struct message temp;
	for(int i=0;WINDOWSIZE-1>i;i++)
	{	arr[i]=arr[i+1];
		status_arr[i]=status_arr[i+1];
	}
	status_arr[5]=0;
	}
	return;
}


void main()
{
	int sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(sock==-1)
		die("Socket");
		
	FILE *fp;
	fp=fopen("destination.txt","wb");
	if(fp==NULL)
	{
		die("Error");
	}
	struct sockaddr_in server,relay;
	int sizeRelay=sizeof(relay);
	server.sin_family=AF_INET;
	server.sin_port=htons(8888);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	
	int temp=bind(sock,(struct sockaddr*)&server,sizeof(server));
	
	if(temp==-1)
		die("Binding");
	
//	printf("Listening\n");
	struct message arr[WINDOWSIZE];
	int status_arr[WINDOWSIZE]={0};
	
//	for(int r=0;6>r;r++)
//		printf("%d ",status_arr[r]);
//	printf("Hello \n");
	while(1)
	{
		if(end==1)
			break;
		struct message rcv;
		temp=recvfrom(sock,&rcv,sizeof(rcv),0,(struct sockaddr*)&relay,&sizeRelay);
				printf("SERVER	R	DATA	  %d	RELAY	SERVER\n",rcv.seq_no);
		
		if(temp<0)
			die("Receiving");
		
		struct ack rcvAck;
		
		rcvAck.seq_no=rcv.seq_no;
		status_arr[rcvAck.seq_no-start_seq]=1;
		arr[rcvAck.seq_no-start_seq]=rcv;
		temp=sendto(sock,&rcvAck,sizeof(rcvAck),0,(struct sockaddr*)&relay,sizeRelay);
		printf("SERVER	S	ACK	  %d	SERVER	RELAY\n",rcvAck.seq_no);
		shift_one(arr,status_arr,fp);
		
	
	}
	fclose(fp);
	close(sock);
	return;
	





}
