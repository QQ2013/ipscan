#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>	
#include<unistd.h>	
#include<string.h>
#include<sys/socket.h>	
#include<sys/types.h>
#include<netdb.h>	
#include<errno.h>	
#include<arpa/inet.h>	
#include<signal.h>	
#include<netinet/in.h>	
#include<time.h>
#include"ping_ip.h"
#include<syslog.h>
#define IP_HSIZE sizeof(struct iphdr)   
#define IPVERSION  4   

//struct itimerval val_alarm={.it_interval.tv_sec = 1,	
//                               .it_interval.tv_usec=0,
//                               .it_value.tv_sec=0,
//                               .it_value.tv_usec=1
//                                };
char msg[1024];
void printtime()
{
	struct tm *ptr;
	time_t it;
    char str_time[100];
	it = time(NULL);

	ptr = localtime(&it);
	sprintf(str_time,"%s",asctime(ptr));

    int len = strlen(str_time);
	str_time[len-1] = '\0';
	printf("%s ",str_time);

//	printf(ctime(&it));
}
void send_ping()
{
	struct iphdr *ip_hdr;	
	struct icmphdr *icmp_hdr;	
	int len;
	int len1;

	ip_hdr=(struct iphdr *)sendbuf;	
	ip_hdr->hlen=sizeof(struct iphdr)>>2;	
	ip_hdr->ver=IPVERSION;	
	ip_hdr->tos=0;	
	ip_hdr->tot_len=IP_HSIZE+ICMP_HSIZE+datalen;	
	ip_hdr->id=0;	
	ip_hdr->frag_off=0;	
	ip_hdr->protocol=IPPROTO_ICMP;
	ip_hdr->ttl=255;	
	ip_hdr->daddr=dest.sin_addr.s_addr;	
	ip_hdr->saddr=inet_addr(ip_saddr);
	len1=ip_hdr->hlen<<2;	

	icmp_hdr=(struct icmphdr *)(sendbuf+len1);	
	icmp_hdr->type=8;  	
	icmp_hdr->code=0;	
	icmp_hdr->icmp_id=pid;	
	icmp_hdr->icmp_seq=nsent++;	
	memset(icmp_hdr->data,0xff,datalen);	 

	gettimeofday((struct timeval *)icmp_hdr->data,NULL);	

	len=ip_hdr->tot_len;	
	icmp_hdr->checksum=0;	
	icmp_hdr->checksum=checksum((u8 *)icmp_hdr,len);  

	sendto(sockfd,sendbuf,len,0,(struct sockaddr *)&dest,sizeof (dest)); 
}

void recv_reply()
{
	int n,len;
	int errno;
	n=nrecv=0;
	len=sizeof(from);	

	while(1){
		if((n=recvfrom(sockfd,recvbuf,sizeof recvbuf,0,(struct sockaddr *)&from,&len))<0){	
			if(errno==EINTR)	
				continue;
			bail("recvfrom error");
		}

		gettimeofday(&recvtime,NULL);	

		if(handle_pkt())	
			continue;
		nrecv++;
	}

	get_statistics(nsent,nrecv);	 
}

u16 checksum(u8 *buf,int len)
{
	u32 sum=0;
	u16 *cbuf;

	cbuf=(u16 *)buf;

	while(len>1){
		sum+=*cbuf++;
		len-=2;
	}

	if(len)
		sum+=*(u8 *)cbuf;

	sum=(sum>>16)+(sum & 0xffff);
	sum+=(sum>>16);

	return ~sum;
}

int handle_pkt()
{
	struct iphdr *ip;
	struct icmphdr *icmp;

	int ip_hlen;
	u16 ip_datalen;	
	double rtt;	
	struct timeval *sendtime;

	ip=(struct iphdr *)recvbuf;
        char cmdline[1024];
	time_t now;
	//time(&now);


    //printf("%d\n",ip->tos);
	ip_hlen=ip->hlen << 2;
	ip_datalen=ntohs(ip->tot_len)-ip_hlen;

	icmp=(struct icmphdr *)(recvbuf+ip_hlen);

	if(checksum((u8 *)icmp,ip_datalen))	
		return -1;


	if(icmp->icmp_id == pid +1)
	{
		sendtime=(struct timeval *)icmp->data; 
		rtt=((&recvtime)->tv_sec-sendtime->tv_sec)*1000+((&recvtime)->tv_usec-sendtime->tv_usec)/1000.0;

		//printtime();
		syslog(LOG_USER|LOG_INFO,"heart_beat %s %.3f ms\n",
				inet_ntoa(from.sin_addr),	
				rtt);
		//system(cmdline);
		time_out = 0;
		if((setitimer(ITIMER_REAL,&val_alarm,NULL))==-1)	
			bail("setitimer fails.");
	}

	if(icmp->icmp_id!=pid)
		return -1;

	sendtime=(struct timeval *)icmp->data; 
	rtt=((&recvtime)->tv_sec-sendtime->tv_sec)*1000+((&recvtime)->tv_usec-sendtime->tv_usec)/1000.0;

    if(!fout)
	{
		printf("%d bytes from %s:icmp_seq=%u ttl=%d rtt=%.3f ms\n",
				ip_datalen, 
				inet_ntoa(from.sin_addr),	
				icmp->icmp_seq, 
				ip->ttl,	
				rtt);
	}
	else
	{
		sprintf(msg,"%-16s %-10d %.3f \n",
				inet_ntoa(from.sin_addr),	
				icmp->icmp_seq, 
				rtt);
		fputs(msg,fout);
		fflush(fout);
	}

	return 0;
}

void set_sighandler()
{
	act_alarm.sa_handler=alarm_handler;
	if(sigaction(SIGALRM,&act_alarm,NULL)==-1)	
		bail("SIGALRM handler setting fails.");

	act_int.sa_handler=int_handler;
	if(sigaction(SIGINT,&act_int,NULL)==-1)
		bail("SIGALRM handler setting fails.");
}

void get_statistics(int nsent,int nrecv)
{
	printf("--- %s ping statistics ---\n",inet_ntoa(dest.sin_addr)); 
	printf("%d packets transmitted, %d received, %0.0f%% ""packet loss\n",
			nsent,nrecv,1.0*(nsent-nrecv)/nsent*100);
}

void bail(const char * on_what)
{
	fputs(strerror(errno),stderr);	
	fputs(":",stderr);
	fputs(on_what,stderr);
	fputc('\n',stderr);	
	exit(1);
}


void int_handler(int sig)
{
	close(sockfd);	
	printtime();
	printf(":use exit\n");
	exit(1);
}

void alarm_handler(int signo)
{
    if(time_out==1)
	{
		printtime();
		printf(":nomal exit\n");
		exit(1);
	}
	else
	{
		time_out=1;
	}
}

