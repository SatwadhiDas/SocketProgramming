#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define WINDOWSIZE 6
#define PACKETSIZE 100
#define TIMER 2
#define PDR 10
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

void die(char *s){
	perror(s);
	exit(1);
}

void main()
{
	int sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(sock==-1)
		die("Socket");
		
	int mSock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(mSock==-1)
		die("Socket");
		
	struct sockaddr_in si_me,input,server;
	int sizeInput=sizeof(input);
	int temp,temp1;
	si_me.sin_family=AF_INET;
	si_me.sin_port=htons(12346);
	si_me.sin_addr.s_addr=htonl(INADDR_ANY);
	
	
	int sizeServer=sizeof(server);
	server.sin_family=AF_INET;
	server.sin_port=htons(8888);
	server.sin_addr.s_addr=inet_addr("127.0.0.1");

	temp=bind(sock,(struct sockaddr*)&si_me,sizeof(si_me));
	if(temp==-1)
		die("Binding");
		
	while(1)
	{
	struct message rcv;
	//printf("Listening \n");
	temp=recvfrom(sock,&rcv,sizeof(rcv),0,(struct sockaddr*)&input,&sizeInput);
		if(rcv.seq_no==-1)
		break;
		int rnd1=rand()%101;
	if(rnd1<PDR)
	{
		//printf("Dropping");
		continue;
	}
	printf("RELAY1	R	DATA	  %d	CLIENT		RELAY1\n",rcv.seq_no);

	//pid_t retVal;
	//if((retVal=fork())==0)
	//{
		//struct message rcv1=rcv;
		int rnd=rand()%300;
		usleep(rnd);
		temp=sendto(mSock,&rcv,sizeof(rcv),0,(struct sockaddr*)&server,sizeof(server));
		printf("RELAY1	S	DATA	  %d	RELAY1	SERVER\n",rcv.seq_no);
		struct ack rcvAck;
		temp=recvfrom(mSock,&rcvAck,sizeof(rcvAck),0,(struct sockaddr*)&server, &sizeServer);
		printf("RELAY1	R	ACK	  %d	server		RELAY1\n",rcvAck.seq_no);
		if(temp<0)
			die("Receiving");
		temp=sendto(sock,&rcvAck,sizeof(rcvAck),0,(struct sockaddr*)&input,sizeInput);
		printf("RELAY1	S	ACK	  %d	RELAY1	CLIENT\n",rcvAck.seq_no);
	//	exit(0);
	//}
	
	}
	close(sock);
	close(mSock);
	



}
