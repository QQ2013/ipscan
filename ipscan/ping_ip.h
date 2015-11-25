#define ICMP_ECHOREPLY 0 /* EchoӦ��*/
#ifndef __PING_IP_H
#define __PING_IP_H
#define ICMP_ECHO	/*Echo����*/

#define BUFSIZE 1500	/*���ͻ������ֵ*/
#define DEFAULT_LEN 56  /**ping��Ϣ����Ĭ�ϴ�С/

/*�������ͱ���*/
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

/*ICMP��Ϣͷ��*/
struct icmphdr {
    u8 type;     /*������Ϣ����*/
    u8 code;	/*������Ϣ����*/
    u16 checksum;	/*����У��*/
    union{
    	struct{
	    u16 id;
	    u16 sequence;
	}echo;
	u32 gateway;
	struct{
 	    u16 unsed;
	    u16 mtu;
	}frag; /*pmtuʵ��*/
    }un;
  /*ICMP����ռλ��*/
    u8 data[0];
#define icmp_id un.echo.id
#define icmp_seq un.echo.sequence
};
#define ICMP_HSIZE sizeof(struct icmphdr)
/*����һ��IP��Ϣͷ���ṹ��*/
struct iphdr {
    u8 hlen:4, ver:4;	/*����4λ�ײ����ȣ���IP�汾��ΪIPV4*/
    u8 tos;		/*8λ��������TOS*/
    u16 tot_len;	/*16λ�ܳ���*/
    u16 id;			/*16λ��־λ*/
    u16 frag_off;	/*3λ��־λ*/
    u8 ttl;			/*8λ��������*/
    u8 protocol;	/*8λЭ��*/
    u16 check;		/*16λIP�ײ�У���*/
    u32 saddr;		/*32λԴIP��ַ*/
    u32 daddr;		/*32λĿ��IP��ַ*/
};

char *hostname;			/*��ping��������*/
int datalen ;	/*ICMP��ϢЯ�������ݳ���*/
char sendbuf[BUFSIZE];		/*�����ַ�������*/	
char recvbuf[BUFSIZE];		/*�����ַ�������*/
int nsent;			/*���͵�ICMP��Ϣ���*/
int nrecv;			/*���յ�ICMP��Ϣ���*/
int pid;			/*ping����Ľ���PID*/
struct timeval recvtime;	/*�յ�ICMPӦ���ʱ���*/
int sockfd;			/*���ͺͽ���ԭʼ�׽���*/
struct sockaddr_in dest;	/*��ping������IP*/
struct sockaddr_in from;	/*����pingӦ����Ϣ������IP*/
struct sigaction act_alarm;
struct sigaction act_int;

/*����ԭ��*/
void alarm_handler(int);	/*SIGALRM�������*/
void int_handler(int);		/*SIGINT�������*/
void set_sighandler();		/*�����źŴ������*/
void send_ping();		/*����ping��Ϣ*/
void recv_reply();		/*����pingӦ��*/
u16 checksum(u8 *buf, int len); /*����У���*/
int handle_pkt();		/*ICMPӦ����Ϣ����*/
void get_statistics(int, int);	/*ͳ��ping����ļ����*/
void bail(const char *);	/*���󱨸�*/
char ip_daddr[20];         /*Ŀ��ip*/
char ip_saddr[20];         /*Դip-����ip*/
char ip_beat[20];         /*����ip*/
struct itimerval val_alarm;
FILE *fout;
int time_out;
#endif

