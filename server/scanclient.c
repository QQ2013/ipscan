#include <stdio.h>
#include <stdlib.h>
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
int queues_init_no_del()
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

}
int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("error:scan_client cmdline \n");
	}
	queues_init_no_del();
	msgbuffer. msgtype = 3;
	sprintf(msgbuffer.ctrlstring,"%s",argv[1]);
	msglen = sizeof(msgbuffer) - 4;

	if(msgsnd (qid_s, &msgbuffer, msglen, 0) == -1)
	{
		printf("scan_client send error \n");
		exit (1);
	}
	exit(0);
}
