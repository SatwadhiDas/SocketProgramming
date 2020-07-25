#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define WINDOWSIZE 6
#define PACKETSIZE 100
#define TIMER 2
int seq_no=1;
int start_seq=1;
int end;
struct message{
	int size;
	int seq_no;
	int final;
	char data[101];
	};
	

struct ack
{
int seq_no;
};
	
void die(char* s){
	perror(s);
	exit(1);
}

void make_packet(struct message *packet, FILE *fp)
{
	
	int size = fread(packet->data,1,100,fp);
	packet->data[100]='\0';
	if( size > 0)
	{
	packet->size=size;
	packet->seq_no=seq_no;
	seq_no++;
	packet->final=0;
	}
	
	if(size!=100)
	{	
	end=1;
	packet->final=1;
	}
	return;
}

void make_packet_arr(struct message *arr,FILE *fp)
{
for(int i=0;WINDOWSIZE>i;i++)
	make_packet(&arr[i],fp);
return;
}

void shift_one(struct message *arr, int *status_arr,FILE *fp)
{
	start_seq++;
	
	struct message temp;
	for(int i=0;WINDOWSIZE-1>i;i++)
	{	arr[i]=arr[i+1];
		status_arr[i]=status_arr[i+1];
	}
	make_packet(&arr[5],fp);
	status_arr[5]=0;
	return;
}


	

	
	
int main()
{	
	int sock[2];
	sock[0] = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sock[1] = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int mSock=sock[0];
	if(sock[0]<sock[1])
		mSock=sock[1];
		
	if(sock[0] < 0 || sock[1]<0) 
	{ 
		printf ("Error in opening a socket"); 
		exit (0);
	}
	printf ("Client Socket Created\n");
	
	struct sockaddr_in relayEven,relayOdd,ran;
	int sizEven=sizeof(relayEven);
	int sizOdd=sizeof(relayOdd);
	memset (&relayEven,0,sizeof(relayEven));
	memset (&relayOdd,0,sizeof(relayOdd));
	relayEven.sin_family = AF_INET;
	relayEven.sin_port = htons(12345); //You can change port number here
	relayEven.sin_addr.s_addr = inet_addr("127.0.0.1"); //Specify server's
	
	relayOdd.sin_family = AF_INET;
	relayOdd.sin_port = htons(12346); //You can change port number here
	relayOdd.sin_addr.s_addr = inet_addr("127.0.0.1"); //Specify server's
	
	FILE *fp;
	fp=fopen("input.txt","rb");
	if(NULL == fp)
	{
		printf("file opening error\n");
		return 1;
	}
	struct message arr[WINDOWSIZE];
	make_packet_arr(arr,fp);
	int status_arr[WINDOWSIZE]={0};
	int cnt=0;
	
	fd_set fds;

	while(1)
	{
	if(end==1 && cnt==WINDOWSIZE)
		break;
	if(start_seq%2==1)
		for(int i=0;WINDOWSIZE/2>i;i++)
		{
			if(status_arr[2*i]==0)
			{
			sendto(sock[1],&(arr[2*i]),sizeof(arr[2*i]),0,(struct sockaddr*)&relayOdd,sizeof(relayOdd));
			printf("CLIENT	S	DATA	%d	CLIENT	RELAY2\n",arr[2*i].seq_no);
			}
			if(status_arr[2*i+1]==0)
			{
			printf("CLIENT	S	DATA	%d	CLIENT	RELAY1\n",arr[2*i].seq_no);
			sendto(sock[0],&(arr[2*i+1]),sizeof(arr[2*i+1]),0,(struct sockaddr*)&relayEven,sizeof(relayEven));
			}
		}
	

	else
		for(int i=0;WINDOWSIZE/2>i;i++)
			{
				if(status_arr[2*i]==0)
				{
				sendto(sock[0],&(arr[2*i]),sizeof(arr[2*i]),0,(struct sockaddr*)&relayEven,sizeof(relayEven));
				printf("CLIENT	S	DATA	%d	CLIENT	RELAY1\n",arr[2*i].seq_no);
				}
				if(status_arr[2*i+1]==0)
				{sendto(sock[1],&(arr[2*i+1]),sizeof(arr[2*i+1]),0,(struct sockaddr*)&relayOdd,sizeof(relayOdd));
				printf("CLIENT	S	DATA	%d	CLIENT	RELAY2\n",arr[2*i].seq_no);
				}
			}	
	
	struct timeval timeout;
	timeout.tv_sec=TIMER;
	timeout.tv_usec=0;
	int temp=1;
	
	while(cnt !=WINDOWSIZE)
	{
	//printf("waiting for ACK\n");
		FD_ZERO(&fds);
	FD_SET(sock[0],&fds);
	FD_SET(sock[1],&fds);
	temp=select(mSock+1,&fds,NULL,NULL,&timeout);
	
	if(temp==-1)
		die("Timer");
	
	else if(temp==0)
	{
	//	printf("Timeout\n");
		break;
	}	
	for(int y=0;2>y;y++)
	
		if(FD_ISSET(sock[y],&fds))
		{
		struct ack rcv;
		int temp1;
		if(y==0)
		{
		temp1=recvfrom(sock[0],&rcv,sizeof(rcv),0,(struct sockaddr*)&relayEven,&sizEven);
		printf("CLIENT	R	ACK	  %d	RELAY1	CLIENT\n",rcv.seq_no);
		}
		else
		{
		temp1=recvfrom(sock[1],&rcv,sizeof(rcv),0,(struct sockaddr*)&relayOdd,&sizOdd);	
		printf("CLIENT	R	ACK	  %d	RELAY2	CLIENT\n",rcv.seq_no);
		}			
		if(temp1==-1)
			die("Receiving");
	//	printf("Seq_no %d",rcv.seq_no);
		cnt++;
		//int p=rcv.seq_no-seq_no;
		status_arr[rcv.seq_no-start_seq]=1;
		break;
		}
	
	//	for(int r=0;6>r;r++)
	//	printf("%d ",status_arr[r]);
//	printf("Hello \n");
	}
	while(status_arr[0]==1 && end!=1)
	{
	shift_one(arr,status_arr,fp);
	cnt--;
	}
	
	}	
	
	struct message pkt;
	
	pkt.seq_no=-1;
	
	sendto(sock[0],&pkt,sizeof(pkt),0,(struct sockaddr*)&relayEven,sizeof(relayEven));
				
	sendto(sock[1],&pkt,sizeof(pkt),0,(struct sockaddr*)&relayOdd,sizeof(relayOdd));
	
	
	
	
	
	
}	
