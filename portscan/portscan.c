// �˿�ɨ�����ֻ֧��ɨ��TCP�˿�
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// ����һ���˿�������Ϣ
typedef struct _port_segment {
	struct in_addr		dest_ip;	// Ŀ��IP
	unsigned short int	min_port;	// ��ʼ�˿�
	unsigned short int	max_port;	// ���˿�
} port_segment;

/*�Զ���Ĵ�������*/
void my_err(const char * err_string, int line)
{
	fprintf(stderr, "line:%d  ", line);
	perror(err_string);
	exit(1);
}

/*
 * ��  ����ɨ��ĳһIP��ַ�ϵ�ĳһ���˿ڵĺ���
 * ����ֵ�� -1  	����
 *	    0  	Ŀ��˿�δ��
 *	    1	Ŀ��˿��Ѵ�
 */
int do_scan(struct sockaddr_in serv_addr)
{
	int		conn_fd;
	int		ret;
	
	// ����һ��TCP�׽���
	conn_fd = socket(AF_INET, SOCK_STREAM,0);
	if (conn_fd < 0) {
		my_err("socket", __LINE__);
	}
	
	// ��������˷�����������
	if ( (ret = connect(conn_fd, (struct sockaddr *)&serv_addr, 
				sizeof (struct sockaddr))) < 0 ) {
		if (errno == ECONNREFUSED) {	// Ŀ��˿�δ��
			printf("port %d close in %s\n", ntohs(serv_addr.sin_port), 
					inet_ntoa(serv_addr.sin_addr));
			close(conn_fd);
			return 0;
		} else {	// ��������
			close(conn_fd);
			return -1;
		}
	} else if (ret == 0){
		printf("port %d found in %s\n", ntohs(serv_addr.sin_port), 
				inet_ntoa(serv_addr.sin_addr));
		close(conn_fd);
		return 1;
	}
	
	return -1;	// ʵ��ִ�в������ֻ��Ϊ�������������ʱ�����ľ���
}

// ִ��ɨ����̣߳�ɨ��ĳһ����Ķ˿�
void * scaner(void *arg)
{
	unsigned short int	i;
	struct sockaddr_in	serv_addr;
	port_segment		portinfo;	  // �˿���Ϣ
	
	// ��ȡ�˿�������Ϣ
	memcpy(&portinfo, arg, sizeof(struct _port_segment));
	
	// ��ʼ���������˵�ַ�ṹ
	memset(&serv_addr, 0, sizeof (struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = portinfo.dest_ip.s_addr;
	
	for (i=portinfo.min_port; i<=portinfo.max_port; i++) {
		serv_addr.sin_port = htons(i);
		if (do_scan(serv_addr) < 0) {
			continue;	// �������˳�����
		}
	}
	return NULL;
}

/*
 * �����в�����-m ���˿ڣ� -a Ŀ��������IP��ַ,  -n ����߳���
 */
int main(int argc, char **argv)
{
	pthread_t*	thread;			// ָ�����е��߳�ID
	int		max_port;		// ���˿ں�
	int		thread_num;		// ����߳���
	int		seg_len;		// �˿����䳤��
	struct in_addr	dest_ip;		// Ŀ������IP
	int		i;

	// ����������
	if (argc != 7) {
		printf("Usage: [-m] [max_port] [-a] [serv_address] [-n] [thread_number]\n");
		exit(1);
	}

	// ���������в���
	for (i=1; i<argc; i++) {
		if (strcmp("-m", argv[i]) == 0) {
			max_port = atoi(argv[i+1]);   // ���ַ���ת��Ϊ��Ӧ������
			if (max_port < 0 || max_port > 65535) {
				printf("Usage:invalid max dest port\n");
				exit(1);
			}
			continue;
		}

		if (strcmp("-a", argv[i]) == 0) {
			if (inet_aton(argv[i+1], &dest_ip) == 0) {
				printf("Usage:invalid dest ip address\n");
				exit(1);
			}
			continue;
		}

		if (strcmp("-n", argv[i]) == 0) {
			thread_num = atoi(argv[i+1]);
			if (thread_num <= 0) {
				printf("Usage:invalid thread_number\n");
				exit(1);
			}
			continue;
		}
	}
	// �����������˿ں�С���߳��������߳�����Ϊ���˿ں�
	if (max_port < thread_num) {
		thread_num = max_port;
	}

	seg_len = max_port / thread_num;
	if ( (max_port%thread_num) != 0 ) {
		thread_num += 1;
	}
	// ����洢�����߳�ID���ڴ�ռ�
	thread = (pthread_t*)malloc(thread_num*sizeof(pthread_t));

	// �����̣߳��������˿ںź��߳�������ÿ���߳�ɨ��Ķ˿�����
	for (i=0; i<thread_num; i++) {
		port_segment	portinfo;	
		portinfo.dest_ip = dest_ip;
		portinfo.min_port = i*seg_len + 1;
		if (i == thread_num-1) {
			portinfo.max_port = max_port;
		} else {
			portinfo.max_port = portinfo.min_port + seg_len - 1;
		}
		// �����߳�
		if (pthread_create(&thread[i], NULL, scaner, (void *)&portinfo) != 0) {
			my_err("pthread_create", __LINE__);
		}
		// ���̵߳ȴ����߳̽���
		pthread_join(thread[i],NULL);
	}
	
	return 0;
}
