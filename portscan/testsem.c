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
#define FILE_MODE (S_IRUSR|S_IWUSR|S_IXGRP|S_IXOTH)
sem_t *nempty;
void my_err(const char * err_string, int line)
{
	fprintf(stderr, "line:%d  ", line);
	perror(err_string);
	exit(1);
}
void usage();
int do_scan(struct sockaddr_in serv_addr);
char addr_scan[10000][20];

int main(int argc, char **argv)
{
	int opt = 0;
	int options_index = 0;
	int port = 0;
	int pid_num = 0;
    char addr[20];
	char addr_file[20];
	int set[3];
	int addr_num = 0;

	set[0] = 0;
	set[1] = 0;
	set[2] = 0;
	set[3] = 0;

	addr[0]      = '\0';
	addr_file[0] = '\0';

	setbuf(stdout,NULL);

	while((opt=getopt_long(argc,argv,"a:f:n:p:h",NULL,&options_index))!=EOF )
	{
		switch(opt)
		{
			case 'a':
				      sprintf(addr,"%s",optarg);
					  set[0] = 1;
					  break;
			case 'f':
					  sprintf(addr_file,"%s",optarg);
					  set[1] = 1;
					  break;
			case 'p':
					  port = atoi(optarg);
					  break; 
			case 'n':
					  pid_num = atoi(optarg);
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
	
	int i = 0;

	struct sockaddr_in	serv_addr;
	
	memset(&serv_addr, 0, sizeof (struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(addr);
	serv_addr.sin_port = htons(port);

	if(set[0]==1)
	{
		do_scan(serv_addr);
	    return 1;
	}

    nempty = sem_open("portscan",O_CREAT, FILE_MODE, pid_num); 	

	if(nempty == SEM_FAILED)
	{
		printf("an error");
	}

	FILE *fp;
	fp = fopen(addr_file,"a+");

	if (fp == NULL)
	{
		printf("open %s error\n",addr_file);
		return 0;
	}

	sem_unlink("portscan");
	signal(SIGCHLD,SIG_IGN);
	while(feof(fp)==0)
	{
		if (fscanf(fp,"%s\n",addr_scan[addr_num]) <0)
		{
			printf("read addr error\n");
			continue;
		}
		addr_num++;
	}
	fclose(fp);

	pid_t pid = 0;
	
	for(i=0;i<addr_num;i++)
	{
		sem_wait(nempty);

	    pid = fork();
		if (pid ==(pid_t)0)
		{
	        serv_addr.sin_addr.s_addr = inet_addr(addr_scan[i]);
			do_scan(serv_addr);
			exit(1);
		}
		

	}

		
	int chpid = 0;

	while(1)
	{
		sem_getvalue(nempty,&chpid);
		if (chpid == pid_num)
		{
			break;
		}
		fprintf(stderr,"%d\n",chpid);
		usleep(1000000);
	}
	return 0;
}
void usage()
{
	fprintf(stderr,
	       "pingrecv [options] \n"
		   "-a addr\n"
           "-f addr file\n"
		   "-n pid num\n"
		   "-p port\n"
		   "-h help\n"
		   "please make sure ip is right!\n"
		  );

}
int do_scan(struct sockaddr_in serv_addr)
{
	int		conn_fd;
	int		ret;
	
	// 创建一个TCP套接字
	conn_fd = socket(AF_INET, SOCK_STREAM,0);
	if (conn_fd < 0) 
	{
		my_err("do_sacn", __LINE__);
	}
	// 向服务器端发送连接请求
	if ( (ret = connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof (struct sockaddr))) < 0 ) 
	{
		if (errno == ECONNREFUSED) 
		{	
			// 目标端口未打开
			printf("port %d close in %s\n", ntohs(serv_addr.sin_port), inet_ntoa(serv_addr.sin_addr));
			close(conn_fd);
			sem_post(nempty);
			return 0;
		} 
		else 
		{	// 其他错误
			printf("port %d close in %s\n", ntohs(serv_addr.sin_port), inet_ntoa(serv_addr.sin_addr));
			close(conn_fd);
			close(conn_fd);
			sem_post(nempty);
			return -1;
		}
	} 
	else if (ret == 0)
	{
		printf("port %d open in %s\n", ntohs(serv_addr.sin_port), inet_ntoa(serv_addr.sin_addr));
		close(conn_fd);
	    sem_post(nempty);
		return 1;
	}
	
	return -1;	// 实际执行不到这里，只是为了消除编译程序时产生的警告
}
