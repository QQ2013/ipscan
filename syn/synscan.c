#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>	
#include <string.h>
#include <sys/socket.h>	
#include <netdb.h>	
#include <errno.h>	
#include <arpa/inet.h>	
#include <signal.h>	
#include <netinet/in.h>	
#include <linux/tcp.h>
#include <netinet/ip.h>
#include <time.h>
#include <syslog.h>
#define TCP_SIZE sizeof(struct tcphdr)
void send_data(int sockfd,struct sockaddr_in *addr, int soureport,char ip[30]);
unsigned short check_sum(unsigned short *addr, int len);
void *recv_packet();
void usage();

int ip_num;
int startport;
int endport;
int sockfd;
int sourceport = 33333;

char ip[30];
int times = 1;
int main(int argc,char *argv)
{
    struct sockaddr_in addr;
	struct hostent *host;
	char sourceip[30];
	int i;
	int on = 1;
	pthread_t tid;
	clock_t start;
	int sleep_t=0;
	
	//////////////////////////////////////////////////
	int opt = 0;
	int options_index = 0;
	int port = 8080;
	int net_width = 1;
	int set[3];
	char ip_file[1024];
	

	set[0] = 0;
	set[1] = 0;
	set[2] = 0;
	set[3] = 0;

	ip[0]      = '\0';
	ip_file[0] = '\0';

	setbuf(stdout,NULL);

	while((opt=getopt_long(argc,argv,"s:f:w:p:n:t:h",NULL,&options_index))!=EOF )
	{
		switch(opt)
		{
			case 's':
				      sprintf(sourceip,"%s",optarg);
					  set[0] = 1;
					  break;
			case 'f':
					  sprintf(ip_file,"%s",optarg);
					  set[1] = 1;
					  break;
			case 'p':
					  port = atoi(optarg);
					  break; 
			case 'w':
					  net_width = atoi(optarg);
					  break; 
			case 'n':
					  times = atoi(optarg);
					  break; 
			case 't':
					  sleep_t = atoi(optarg);
					  break; 
			default:
					  usage();
					  return 0;
		}
	}

	if (optind < argc)
	{
		usage();
		return 0;
	}

	FILE *fp;
	fp = fopen(ip_file,"a+");

	if (fp == NULL)
	{
		printf("open %s error\n",ip_file);
		return 0;
	}

	while(feof(fp)==0)
	{
		if (fscanf(fp,"%s\n",ip)<0)
		{
			printf("read addr error\n");
			continue;
		}
		ip_num++;
	}
	fclose(fp);

	int sleep_num = 10000/net_width;
	
	syslog(LOG_USER|LOG_INFO,"total:%d ip\n",ip_num);
	//////////////////////////////////////////////////
	start = clock();
	
	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

	if((sockfd = socket(AF_INET,SOCK_RAW,IPPROTO_TCP)) <= 0)
	{
		fprintf(stderr,"Socket Error:%s\n", strerror(h_errno));
		exit(EXIT_FAILURE);
	}

	setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
	if((errno = pthread_create(&tid,NULL,recv_packet,start))<0)
	{
		perror("pthread\n");
		return;
	}

	fp = fopen(ip_file,"a+");

	if (fp == NULL)
	{
		printf("open %s error\n",ip_file);
		return 0;
	}


	i = 0;
	int send = 0;
	while(feof(fp)==0)
	{
		if (fscanf(fp,"%s\n",ip)<0)
		{
			printf("read addr error\n");
			continue;
		}
	    addr.sin_addr.s_addr = inet_addr(ip);
		send = times;
		while(send)
		{
		    send_data(sockfd, &addr, sourceport, sourceip);
			send --;
			usleep(sleep_num);
		}
		i++;

		if( i%10000 == 0)
		{
			syslog(LOG_USER|LOG_INFO,"send %d/%d %s  \n",i,ip_num,ip);	
		}
	}
	fclose(fp);
	fprintf(stderr,"\n"); 
	int cost = 0;
	while(1)
	{
		syslog(LOG_USER|LOG_INFO,"send done:%d\n",cost);
		usleep(1000000);
		cost++;
	}
	pthread_join(tid, NULL);
}

void send_data(int sockfd, struct sockaddr_in *addr, int sourceport,char sourceip[30])
{
	char buffer[100];
	struct iphdr *ip;
	struct tcphdr *tcp;

	int head_len;
	int n,i;
	u_char *pPseudoHead;
	u_char pseudoHead[12 + sizeof(struct tcphdr)];
	u_short tcpHeadLen;

	tcpHeadLen = htons(sizeof(struct tcphdr));
	head_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
	bzero(buffer,100);
	
	//ip
	ip = (struct iphdr *)buffer;
	ip->version = IPVERSION;
	ip->ihl = sizeof(struct ip) >> 2;
	ip->tos = 0;
	ip->tot_len = htons(head_len);
	ip->id = 0;
	ip->frag_off = 0;
	ip->ttl = MAXTTL;
	ip->protocol = IPPROTO_TCP;
	ip->check = 0;
	ip->daddr = addr->sin_addr.s_addr;
	ip->saddr = inet_addr(sourceip);

	//tcp
	tcp = (struct tcphdr*)(buffer + sizeof(struct ip));
	tcp->source = htons(sourceport);
	tcp->dest = addr->sin_port;
	tcp->seq = htonl(30000);
	tcp->ack_seq = 0;
	tcp->doff = 5;
	tcp->syn = 1;
	tcp->urg_ptr = 0;
	tcp->window = htons(10052);

	//header
	pPseudoHead = pseudoHead;
	memset(pPseudoHead,0,12 +sizeof(struct tcphdr));
	memcpy(pPseudoHead,&ip->saddr,4);
	pPseudoHead+=4;
	memcpy(pPseudoHead,&ip->daddr,4);
	pPseudoHead+=4;
	memset(pPseudoHead,0,1);
	pPseudoHead++;
	memset(pPseudoHead,0x0006,1);
	pPseudoHead++;

	memcpy(pPseudoHead,&tcpHeadLen,2);
	pPseudoHead+=2;

	memcpy(pPseudoHead,tcp,sizeof(struct tcphdr));

	tcp->check = 0;
	tcp->check = check_sum((unsigned short*)pseudoHead,sizeof(struct tcphdr) + 12);
	if(sendto(sockfd, buffer,head_len,0,(struct sockaddr*)addr,(socklen_t)sizeof(struct sockaddr_in)) < 0)
	{
		perror("sendto\n");
	}
}

void *recv_packet(clock_t start)
{
	struct tcphdr *tcp;
	struct iphdr *ip;
	unsigned char *ip4;
	char *srcaddr;
	int loopend;
	int size;
	char readbuff[1600];
	struct sockaddr_in from;
	int from_len;
	int n;
	float costtime;
	clock_t end;
    
    ip = (struct iphdr*)(readbuff);
	tcp = (struct tcphdr*)(readbuff + 20);
	ip4 = (char *) &ip->saddr;
	for(n=0;n<ip_num*times+1;n++)
	{
		size = recv(sockfd, readbuff, 1600, 0);

		if(size <(20+20))
		{
		//	printf("1\n");
			continue;
		}

		if(ntohs(tcp->dest)!= sourceport)
		{
		//	printf("2\n");
			continue;
		}

		if(tcp->rst&&tcp->ack)
		{
		//	printf("3\n");
			continue;
		}

		if(tcp->ack&&tcp->syn)
		{
		//	printf("4\n");
			syslog(LOG_USER|LOG_INFO,"recv %u.%u.%u.%u %5u open\n",ip4[0],ip4[1],ip4[2],ip4[3],(ntohs(tcp->source)));
			continue;
		} 
		/*
		if(tcp->ack)
		{
		//	printf("4\n");
			printf("%u.%u.%u.%u %5u open\n",ip4[0],ip4[1],ip4[2],ip4[3],(ntohs(tcp->source)));
			continue;
		} */

	}
	end = clock();
	costtime = (float)(end-start)/CLOCKS_PER_SEC;
	printf("\nfinished:  %f s\n", costtime);
}

unsigned short check_sum(unsigned short *buffer, int size)
{
	unsigned int cksum = 0;
	while(size>1)
	{
		cksum+=*buffer++;
		size-=sizeof(unsigned short);
	}

	if(size)
	{
		cksum += *(unsigned char*)buffer;
	}
	cksum = (cksum>>16) + (cksum& 0xffff);
	cksum += (cksum>>16);
	return (unsigned short)(~cksum);
}
void usage()
{
	fprintf(stderr,
	       "pingrecv [options] \n"
		   "-s sourceip\n"
		   "-f ip file\n"
		   "-w net width [=1]\n"
		   "-p port [=80]\n"
		   "-n scan times[=1]\n"
		   "-t sleep sec after 10000 scan  [=0]\n"
		   "-h help\n"
		   "please make sure ip is right!\n"
		  );

}
