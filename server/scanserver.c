#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <time.h>
#include <syslog.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#define PATH_SERVER  "./server"
#define PATH_CLIENT  "./client"
#define PROJ_ID      32
#define BUF_SIZE     256
key_t msgkey_s;
key_t msgkey_c;
int qid_s;
int qid_c;
struct mymsgbuf {
	long msgtype;
	char ctrlstring[BUF_SIZE];
} msgbuffer;

int msglen;
int queues_init()
{
	/*server*/
    /*获取键值*/
	if((msgkey_s = ftok(PATH_SERVER, PROJ_ID)) == -1)
	{
		perror("ftok error!\n");
		return -1;
	}

	/*获取消息队列标识符*/
	if((qid_s = msgget(msgkey_s, IPC_CREAT|0660)) == -1)
	{
		perror("msgget error!\n");
		return -1;
	}

	/*删除消息队列*/
	if(msgctl(qid_s, IPC_RMID, NULL) == -1)
	{
		perror("delete msg error!\n");
		return -1;
	}
	if((msgkey_s = ftok(PATH_SERVER, PROJ_ID)) == -1)
	{
		perror("ftok error!\n");
		return -1;
	}

	/*获取消息队列标识符*/
	if((qid_s = msgget(msgkey_s, IPC_CREAT|0660)) == -1)
	{
		perror("msgget error!\n");
		return -1;
	}

	/*client*/
    /*获取键值*/
	if((msgkey_c = ftok(PATH_CLIENT, PROJ_ID)) == -2)
	{
		perror("ftok error!\n");
		return -2;
	}

	/*获取消息队列标识符*/
	if((qid_c = msgget(msgkey_c, IPC_CREAT|0660)) == -2)
	{
		perror("msgget error!\n");
		return -2;
	}

	/*删除消息队列*/
	if(msgctl(qid_c, IPC_RMID, NULL) == -2)
	{
		perror("delete msg error!\n");
		return -2;
	}
	sleep(1);
	if((msgkey_c = ftok(PATH_CLIENT, PROJ_ID)) == -2)
	{
		perror("ftok error!\n");
		return -2;
	}

	/*获取消息队列标识符*/
	if((qid_c = msgget(msgkey_c, IPC_CREAT|0660)) == -2)
	{
		perror("msgget error!\n");
		return -2;
	}

}
int init_daemon(void) 
{ 
	int pid; 
	int i; 

	/*忽略终端I/O信号，STOP信号*/
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	
	pid = fork();
	if(pid > 0) {
		exit(0); /*结束父进程，使得子进程成为后台进程*/
	}
	else if(pid < 0) { 
		return -1;
	}

	/*建立一个新的进程组,在这个新的进程组中,子进程成为这个进程组的首进程,以使该进程脱离所有终端*/
	setsid();

	/*再次新建一个子进程，退出父进程，保证该进程不是进程组长，同时让该进程无法再打开一个新的终端*/
	pid=fork();
	if( pid > 0) {
		exit(0);
	}
	else if( pid< 0) {
		return -1;
	}

	/*关闭所有从父进程继承的不再需要的文件描述符*/
	for(i=0;i< NOFILE;close(i++));

	/*改变工作目录，使得进程不与任何文件系统联系*/
	//chdir("/");

	/*将文件当时创建屏蔽字设置为0*/
	umask(0);

	/*忽略SIGCHLD信号*/
	signal(SIGCHLD,SIG_IGN); 
	
	return 0;
}

int main() 
{ 
	init_daemon();
	int q_init = queues_init();

	if (q_init == -1)
	{
		syslog(LOG_USER|LOG_INFO,"server queues_init false\n");
		exit(1);
	}

	if (q_init == -2)
	{
		syslog(LOG_USER|LOG_INFO,"client queues_init false\n");
		exit(1);
	}

	syslog(LOG_USER|LOG_INFO,"queues_init OK qid_s:%d qid_c:%d \n",qid_s,qid_c);

	/*
	msgbuffer. msgtype = 3;
	strcpy (msgbuffer.ctrlstring , "Hello,message queue");
	msglen = sizeof(msgbuffer) - 4;
	if(msgsnd (qid_c, &msgbuffer, msglen, 0) == -1)
	{
		syslog(LOG_USER|LOG_INFO,"server send error \n");
		exit (1);
	}
	if(msgsnd (qid_s, &msgbuffer, msglen, 0) == -1)
	{
		syslog(LOG_USER|LOG_INFO,"client send error \n");
		exit (1);
	}
	while(1)
	{
		sleep(1);
	}
	*/
	char cmdline[1024];
	pid_t pid;
	msglen = sizeof(struct mymsgbuf) - 4;
	while(1)
	{
		if (msgrcv(qid_s, &msgbuffer, msglen, 3, 0) == -1)  /*读取数据*/
		{
			syslog(LOG_USER|LOG_INFO,"recv error\n");
			exit (1);
		}
		else
		{
			syslog(LOG_USER|LOG_INFO,"recv:%s\n",msgbuffer.ctrlstring);
			
			sprintf(cmdline,". ./%s",msgbuffer.ctrlstring);
			//system(cmdline);
			pid = fork();

			if(pid == (pid_t)0)
			{
				execl("/bin/sh","sh","-c",cmdline,(char*)0);
				exit(1);
			}
			syslog(LOG_USER|LOG_INFO,"shell:%s\n",cmdline);
		}
	}

}
