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

#include"ping_ip.h"
#define IP_HSIZE sizeof(struct iphdr)   
#define IPVERSION  4   

void usage();

int main(int argc,char *argv[])	
{
	int opt=0;
	int options_index=0;
	int set [10];
	char out[1024];
	pid = 999;

	fout = NULL;
	int i = 0;

	for (i=0;i<10;i++)
	{
		set[i] = 0;
	}

	while((opt=getopt_long(argc,argv,"p:f:h",NULL,&options_index))!=EOF )
	{
		switch(opt)
		{
			case 'p':
					  pid = atoi(optarg);
					  break; 
			case 'f':
					  sprintf(out,"%s",optarg);
					  set[1] = 1;
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
	
	if(set[1]==1)
	{
	    fout = fopen(out,"w");
		if(!fout)
		{
			usage();
			return 0;
		}
	}
	
	
	int on =1;
	if((sockfd = socket(PF_INET,SOCK_RAW,IPPROTO_ICMP))<0){ 
		perror("raw socket created error");
		exit(1);
	}
	setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on));  

	val_alarm.it_interval.tv_sec = 60;	
	val_alarm.it_interval.tv_usec=0;
	val_alarm.it_value.tv_sec=0;
	val_alarm.it_value.tv_usec=1;
 
	set_sighandler();
	time_out = 0;
	if((setitimer(ITIMER_REAL,&val_alarm,NULL))==-1)	
		bail("setitimer fails.");
	recv_reply();
    
	fclose(fout);
	return 0;
}
void usage()
{
	fprintf(stderr,
	       "pingrecv [options] \n"
		   "-p pid [=999]\n"
           "-f result save file\n"
		   "-h help\n"
		   "please make sure ip is right!\n"
		  );
}
