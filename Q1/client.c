#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXPENDING 10
#define BUFFERSIZE 32

int TIMER=2;
int end=0;
struct message{
	int size;
	int seq_no;
	int final;
	int dataOrAck;
	int id;
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


void make_packet(struct message *packet, int channel, FILE *fp, int *offset)
{
	
	int size = fread(packet->data,1,100,fp);
	packet->data[100]='\0';
	if( size > 0)
	{
	packet->size=size;
	packet->seq_no=(*offset);
	*offset += size;
	packet->dataOrAck=0;
	packet->id=channel;
	packet->final=0;
	}
	
	if(size!=100)
	{	
	end=1;
	packet->final=1;
	}
	//printf("%d, %s \n",channel,packet->data);
	return;
}

struct timeval min_time(struct timeval t1, struct timeval t2)
{
	if(t1.tv_sec > t2.tv_sec)
		return t2;
	else if(t1.tv_sec < t2.tv_sec)
		return t1;
		
	if(t1.tv_usec > t2.tv_usec)
		return t2;
	else
		return t1;
}

struct timeval sub_time(struct timeval t1, struct timeval t2)
{
	struct timeval ret;
	int usec=t1.tv_usec-t2.tv_usec;
	int carry=0;
	if(usec<0)
	{
	carry=1;
	usec=1000000 + usec;
	}
	int sec= t1.tv_sec - t2.tv_sec - carry;
	
	ret.tv_sec=sec;
	ret.tv_usec=usec;
	
	return ret;
}

int min_time_ind(struct timeval t1, struct timeval t2)
{
	if(t1.tv_sec > t2.tv_sec)
		return 1;
	else if(t1.tv_sec < t2.tv_sec)
		return 0;
		
	if(t1.tv_usec > t2.tv_usec)
		return 1;
	else
		return 0;
}
int main()
{
	int *offset=(int*)malloc(sizeof(int));
	*offset=0;
	
	
	int sock[2];
	sock[0] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sock[1] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	//printf("%d, %d\n",sock[0],sock[1]);
	int max_sock=sock[0];
	if(sock[0]<sock[1])
		max_sock=sock[1];
	
	if(sock[0] < 0 || sock[1]<0) 
	{ 
		printf ("Error in opening a socket"); 
		exit (0);
	}
	printf ("Client Socket Created\n");

	struct sockaddr_in serverAddr,serverAddr1;
	memset (&serverAddr,0,sizeof(serverAddr));
	memset (&serverAddr1,0,sizeof(serverAddr1));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(12345); //You can change port number here
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Specify server's
	
	serverAddr1.sin_family = AF_INET;
	serverAddr1.sin_port = htons(12345); //You can change port number here
	serverAddr1.sin_addr.s_addr = inet_addr("127.0.0.1"); //Specify server's
	
	printf ("Address assigned\n");
	int c = connect (sock[0], (struct sockaddr*) &serverAddr, sizeof(serverAddr));
	//printf ("%d\n",c);
	int c1=connect (sock[1], (struct sockaddr*) &serverAddr, sizeof(serverAddr));
	if (c < 0 || c1 < 0)
	{ 
		printf ("Error while establishing connection");
		exit (0);
	}
	
	printf ("Connection Established\n");
	
	FILE *fp;
	fp=fopen("input.txt","rb");
	if(NULL == fp)
	{
		printf("file opening error\n");
		return 1;
	}
	
	struct timeval t0,t1,timer;
	t1.tv_sec=TIMER;
	t1.tv_usec=0;
	
	t0.tv_sec=TIMER;
	t0.tv_usec=0;
	
	timer.tv_sec=TIMER;
	timer.tv_usec=0;
	
	struct message packet1, packet0;
	int temp, temp1;
	int pending=0;
	int state=0;
	
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock[0],&fds);
	FD_SET(sock[1],&fds);
	
	while(1)
	{
	//printf("case %d, pending %d\n",state,pending);
	if(end==1 && pending==0)
		break;
		switch(state)
		{
			case 0:
			make_packet(&packet0,0,fp,offset);
			make_packet(&packet1,1,fp,offset);
			temp=send(sock[0],&packet0,sizeof(packet0),0);
			printf("SENT PKT:  Seq. No. %d of size %d Bytes from channel %d \n",packet0.seq_no,packet0.size,packet0.id);
			temp1=send(sock[1],&packet1,sizeof(packet1),0);
						printf("SENT PKT:  Seq. No. %d of size %d Bytes from channel %d \n",packet1.seq_no,packet1.size,packet1.id);
			pending+=2;
			
			if(temp==-1 || temp1==-1)
			{
				die("Sending");
			}
			state=1;
	//		printf("here");
			break;
			
			case 1:
		//	printf("Waiting for ACK\n\n");
			temp=select(max_sock+1,&fds,NULL,NULL,&timer);
			//printf("temp val %d\n",temp);
			
			if(temp==-1)
				die("Timer");
			else if(temp==0)
			{
			//	printf("Packet 0 Lost. Retransmitting.\n");
				
				FD_ZERO(&fds);
				FD_SET(sock[0],&fds);
				FD_SET(sock[1],&fds);
				
				int cng=min_time_ind(t0,t1);
				//printf("%ld, %ld\n",timer.tv_sec,timer.tv_usec);
				state=4+cng;
				
				break;
			}
			pending--;
			//printf("%ld, %ld\n",timer.tv_sec,timer.tv_usec);
			struct ack rec;
			for(int i=0;2>i;i++)
			if(FD_ISSET(sock[i],&fds))
			{
				state=2+i;
				
				temp1=recv(sock[i],&rec,sizeof(rec),0);
				printf("RCVD ACK: for PKT with Seq. No. %d from channel %d \n",rec.seq_no,i);
				
				//printf("seq no %d\n",rec.seq_no);
				if(temp1 == -1)
				{
					
					die("Receiving");
				}
				break;	
			}
			break;
			
			case 2:
			;
			struct timeval temp_time, temp_time1;
			temp_time=min_time(t0,t1);
			temp_time1=sub_time(temp_time,timer);
			t1=sub_time(t1,temp_time1);
			t0.tv_sec=TIMER;
			t0.tv_usec=0;
			timer=t1;

			if(end!=1)
			{
			make_packet(&packet0,0,fp,offset);
			temp=send(sock[0],&packet0,sizeof(packet0),0);
			printf("SENT PKT:  Seq. No. %d of size %d Bytes from channel %d \n",packet0.seq_no,packet0.size,packet0.id);
			pending++;
			if(temp==-1)
				die("Sending");
			
			FD_ZERO(&fds);
			FD_SET(sock[0],&fds);
			FD_SET(sock[1],&fds);
			}	
			state=1;
			break;	
			
			case 3:
			;
			
			//timeval temp_time;
			temp_time=min_time(t0,t1);
			temp_time1=sub_time(temp_time,timer);
			t0=sub_time(t0,temp_time1);
			t1.tv_sec=TIMER;
			t1.tv_usec=0;
			timer=t0;
			if(end!=1)
			{
			make_packet(&packet1,1,fp,offset);
			temp=send(sock[1],&packet1,sizeof(packet1),0);
			printf("SENT PKT:  Seq. No. %d of size %d Bytes from channel %d \n",packet1.seq_no,packet1.size,packet1.id);
			pending++;
			if(temp==-1)
				die("Sending");
			
			FD_ZERO(&fds);
			FD_SET(sock[0],&fds);
			FD_SET(sock[1],&fds);
			}	
			state=1;
			break;
			
			case 4:
		
			t1=sub_time(t1,t0);
			timer=t1;
			t0.tv_sec=TIMER;
			t0.tv_usec=0;
			temp=send(sock[0],&packet0,sizeof(packet0),0);
			state=1;
			break;
			
			case 5:
			t0=sub_time(t0,t1);
			timer=t0;
			t1.tv_sec=TIMER;
			t1.tv_usec=0;
			temp=send(sock[1],&packet1,sizeof(packet1),0);
			state=1;
			break;
	
		}
		
	}
	
		
	
	close(sock[0]);
	close(sock[1]);
	fclose(fp);
}
