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

int born_ip(char *srt,int *ip);
void add_ip(int *ip);
int cmp_ip(int *s, int *e);
void usage();
void int_handler_send(int sig);
int loop_send = 0;

int main(int argc,char *argv[])	
{
	int opt=0;
	int options_index=0;
	int ip_start[4];
	int ip_send[4];
	int ip_end[4];
	int ip_br[4];
	int net_width;
	int send_times;
	int i;
	int set[10];
	pid = 999;
	net_width = 1;
	send_times = 1;
	for (i=0;i<10;i++)
	{
		set[i] = 0;
	}

	act_int.sa_handler=int_handler_send;
	if(sigaction(SIGINT,&act_int,NULL)==-1)
		bail("SIGALRM handler setting fails.");
	
	while((opt=getopt_long(argc,argv,"s:e:r:w:p:n:b:lh",NULL,&options_index))!=EOF )
	{
		switch(opt)
		{
			case 's': 
					  //printf("%s\n",optarg);
					  if (born_ip(optarg,ip_start) < 0) 
					  {
						  usage();
						  return 0;
					  }
					  else
					  {
						  ip_end[0]=ip_start[0];
						  ip_end[1]=ip_start[1];
						  ip_end[2]=ip_start[2];
						  ip_end[3]=255;
						  set[0] = 1;
						  break;
						  for (i=0;i<4;i++)
						  {
							  printf("%d ",ip_start[i]);
							  printf("\n");
						  }
					  }
					  break; 
			case 'e': 
					  //printf("%s\n",optarg);
					  if (born_ip(optarg,ip_end) < 0) 
					  {
						  usage();
						  return 0;
					  }
					  else
					  {
						  set[1] = 1;
						  break;
						  for (i=0;i<4;i++)
						  {
							  printf("%d ",ip_end[i]);
							  printf("\n");
						  }
					  }
					  break; 

			case 'r': 
					  if (born_ip(optarg,ip_br) < 0) 
					  {
						  usage();
						  return 0;
					  }
					  else
					  {
						  sprintf(ip_saddr,"%s",optarg);
						  set[2] = 1;
					  }
					  break; 
			case 'w': 
					  net_width = atoi(optarg);
					  break; 
			case 'p': 
					  pid = atoi(optarg);
					  break; 
			case 'n': 
					  send_times = atoi(optarg);
					  break; 
			case 'b':
					  if (born_ip(optarg,ip_br) < 0) 
					  {
						  usage();
						  return 0;
					  }
					  else
					  {
						  sprintf(ip_beat,"%s",optarg);
						  set[3] = 1;
					  }
					  break;
			case 'l': 
					  loop_send = 1;
					  break; 
			case 'h': 
			case ':': 
			default:
					  usage();
					  return 0;
					  break;
		}

	}	

    if (set[0]*set[2]*set[3] == 0)
	{
		usage();
		return 0;
	}

    if (net_width<1||pid<1||send_times<1 )
	{
		usage();
		return 0;
	}

	if (optind < argc)
	{
		usage();
		return 0;
	}

	for (i=0;i<4;i++)
	{
		ip_send[i] = ip_start[i];
	}
    int on =1;
    datalen = DEFAULT_LEN;	/*ICMP消息携带的数据长度*/
	if((sockfd = socket(PF_INET,SOCK_RAW,IPPROTO_ICMP))<0){ 
		perror("raw socket created error");
		exit(1);
	}
	setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on));  

	memset(&dest,0,sizeof dest);	
	dest.sin_family=PF_INET;	
	dest.sin_port=ntohs(0);	
	
    i=1;
	int j = 0;
	int sleep_time;
	sleep_time = 1000/net_width;
    while(1)
	{
		sprintf(ip_daddr,"%d.%d.%d.%d",ip_send[0],ip_send[1],ip_send[2],ip_send[3]);

		if (ip_send[3] == 128)
		{
			pid=pid+1;		
			dest.sin_addr.s_addr=inet_addr(ip_beat);
			send_ping();
			pid = pid-1;
			usleep(sleep_time);
			printf("                  \r");
		}
		dest.sin_addr.s_addr=inet_addr(ip_daddr);
		j = send_times;
		while(j>0)
		{
			send_ping();
			usleep(sleep_time);
			j--;
		}
		printf("%s\r",ip_daddr);
		add_ip(ip_send);
		if(cmp_ip(ip_send,ip_end) >=0)
		{
			if (loop_send == 1)
			{
				for (i=0;i<4;i++)
				{
					ip_send[i] = ip_start[i];
				}
				pid=pid+1;		
				dest.sin_addr.s_addr=inet_addr(ip_beat);
				send_ping();
				pid = pid-1;
				usleep(sleep_time);
				printf("                  \r");

			}
			else
			{
				printf("%s\n",ip_daddr);
				break;
			}
		}
	}
    
	close(sockfd);	
	return 0;
}

int born_ip(char *str,int *ip)
{
	int  len = strlen(str);
	int  i = 0;
	int  p[3];
	int  p_n = 0;
	char ip_tmp[5];
    for (i=0;i<len;i++)
	{
	    if (str[i]== '.')
		{
			p[p_n] = i;
			p_n++;
		}
		else
		{
			if (str[i] < '0' || str[i] > '9')
			{
				//printf("c\n");
				return -1;
			}
		}
	}
    if (p_n != 3)
	{
		//printf(".\n");
		return -1;
	}
    
	//get ip0
    if(p[0] == 0 || p[0] > 3 )
	{
		return -1;
	}
	
	for (i=0;i<p[0];i++)
	{
		ip_tmp[i] = str[i];
	}
	ip_tmp[i]='\0';
	ip[0] = atoi(ip_tmp);
	if (ip[0] > 255 )
	{
		return -1;
	}
    
    //get ip1
    if(p[1]-1 == p[0] || p[1] - p[0] > 4 )
	{
		return -1;
	}
	
	for (i=p[0]+1;i<p[1];i++)
	{
		ip_tmp[i-p[0]-1] = str[i];
	}
	ip_tmp[i-p[0]-1]='\0';
	ip[1] = atoi(ip_tmp);
	if (ip[1] > 255 )
	{
		return -1;
	}

    //get ip2 
    if(p[2]-1 == p[1] || p[2] - p[1] > 4 )
	{
		return -1;
	}
	
	for (i=p[1]+1;i<p[2];i++)
	{
		ip_tmp[i-p[1]-1] = str[i];
	}
	ip_tmp[i-p[1]-1]='\0';
	ip[2] = atoi(ip_tmp);
	if (ip[2] > 255 )
	{
		return -1;
	}
    
    //get ip3 
    if(p[2] == len-1 || len - p[2] > 4 )
	{
		return -1;
	}
	
	for (i=p[2]+1;i<len;i++)
	{
		ip_tmp[i-p[2]-1] = str[i];
	}
	ip_tmp[i-p[2]-1]='\0';
	ip[3] = atoi(ip_tmp);
	if (ip[3] > 255 )
	{
		return -1;
	}
	if (ip[3] == 0)
	{
		ip[3] = 1;
	}
    
	return 1;
}
void add_ip(int *ip)
{
	ip[3] = ip[3] + 1;
    int i = 0;
	for (i=3;i>0;i--)
	{
		if (ip[i] > 255)
		{
			ip[i] = 0;
			ip[i-1] = ip[i-1] + 1;
		}
	}
	if (ip[3] == 0)
	{
		ip[3] = 1;
	}
}
int cmp_ip(int *s, int *e)
{
	int i = 0;
	for (i=0;i<4;i++)
	{
		if (s[i]-e[i]==0)
		{
			continue;
		}

		if (s[i]-e[i] > 0)
		{
			return 1;
		}
		if (s[i]-e[i] < 0)
		{
			return -1;
		}

	}

	if (i==4)
	{
		return 0;
	}

}
void usage()
{
	fprintf(stderr,
	       "pingseg -s start_ip \n"
		   "-s ip_start --need\n"
		   "-e ip_end [=the end of the start ip seg x.x.x.255 ]\n"
		   "-r ip_recv  --need\n"
		   "-w net width [1=]M\n"
		   "-p pid num[=999]\n"
		   "-n ping send times [=1]\n"
		   "-b ip_beat  --need\n"
		   "-l loop send\n"
		   "-h help\n"
		   "please make sure ip is right!\n"
		  );
}

void int_handler_send(int sig)
{

	printf("                  \r");
	printf("%s\n",ip_daddr);
	close(sockfd);	
	exit(1);
}
