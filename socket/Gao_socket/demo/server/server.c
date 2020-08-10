#include"Gao_socket.h"
#pragma comment(lib,"Gao_socket.lib")
#include<stdio.h>


int client_coming_callback(SOCKET client_sock, char* ip) {//�ͻ�����
	char buf[100] = {0};
	printf("\nclient_coming:\nip:%s\nsocket:%d", ip,client_sock);//��ʾ�ͻ���Ϣ
	sprintf(buf,"IP:%s\n",ip);
	printf("\nsend:%d", send_msg(client_sock, buf, strlen(buf)));//��������Ϣ
	return ACCEPT_CLIENT;//���ܿͻ�
}

void client_leave_callback(SOCKET client_sock, char* ip, int state) {//�ͻ��뿪
	if (state == CLIENT_NOR_CLOSE)
	{
		printf("\nclient_normal_leave:\nip:%s\nsocket:%d", ip, client_sock);
	}
	else
	{
		printf("\nclient_unnormal_leave:\nip:%s\nsocket:%d", ip, client_sock);
	}

}

void data_coming_callback(SOCKET client_sock, char* ip, char* data) {//���ݵ���
	printf("\ndata_coming:\nip:%s\ndata:%s\nsocket:%d", ip, data, client_sock);//�����Ϣ
}

void error_callback(SOCKET client_sock, int error) {//�����쳣
	printf("\nerror:%d", error);
}

void main() {

	struct timeval timv = { 1,0 };//��������ѯ���ͻ��˳�ʱʱ��
	unsigned int port;
	printf("Monitored port��"); scanf("%d",&port);//�����������Ķ˿�
	printf("\nstart:%d", startup_sock_api());//startup_sock_api() �������ȵ���
	printf("\nreturn:%d", create_serversock(port, timv, client_coming_callback, client_leave_callback, data_coming_callback, error_callback));
	getchar(); getchar();
}
