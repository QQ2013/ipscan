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
    /*��ȡ��ֵ*/
	if((msgkey_s = ftok(PATH_SERVER, PROJ_ID)) == -1)
	{
		perror("ftok error!\n");
		return -1;
	}

	/*��ȡ��Ϣ���б�ʶ��*/
	if((qid_s = msgget(msgkey_s, IPC_CREAT|0660)) == -1)
	{
		perror("msgget error!\n");
		return -1;
	}

	/*ɾ����Ϣ����*/
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

	/*��ȡ��Ϣ���б�ʶ��*/
	if((qid_s = msgget(msgkey_s, IPC_CREAT|0660)) == -1)
	{
		perror("msgget error!\n");
		return -1;
	}

	/*client*/
    /*��ȡ��ֵ*/
	if((msgkey_c = ftok(PATH_CLIENT, PROJ_ID)) == -2)
	{
		perror("ftok error!\n");
		return -2;
	}

	/*��ȡ��Ϣ���б�ʶ��*/
	if((qid_c = msgget(msgkey_c, IPC_CREAT|0660)) == -2)
	{
		perror("msgget error!\n");
		return -2;
	}

	/*ɾ����Ϣ����*/
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

	/*��ȡ��Ϣ���б�ʶ��*/
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

	/*�����ն�I/O�źţ�STOP�ź�*/
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	
	pid = fork();
	if(pid > 0) {
		exit(0); /*���������̣�ʹ���ӽ��̳�Ϊ��̨����*/
	}
	else if(pid < 0) { 
		return -1;
	}

	/*����һ���µĽ�����,������µĽ�������,�ӽ��̳�Ϊ�����������׽���,��ʹ�ý������������ն�*/
	setsid();

	/*�ٴ��½�һ���ӽ��̣��˳������̣���֤�ý��̲��ǽ����鳤��ͬʱ�øý����޷��ٴ�һ���µ��ն�*/
	pid=fork();
	if( pid > 0) {
		exit(0);
	}
	else if( pid< 0) {
		return -1;
	}

	/*�ر����дӸ����̼̳еĲ�����Ҫ���ļ�������*/
	for(i=0;i< NOFILE;close(i++));

	/*�ı乤��Ŀ¼��ʹ�ý��̲����κ��ļ�ϵͳ��ϵ*/
	//chdir("/");

	/*���ļ���ʱ��������������Ϊ0*/
	umask(0);

	/*����SIGCHLD�ź�*/
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
		if (msgrcv(qid_s, &msgbuffer, msglen, 3, 0) == -1)  /*��ȡ����*/
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
