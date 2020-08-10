#include"Gao_socket_Class.h"
#pragma comment(lib,"Gao_socket_Class.lib")
#include<stdio.h>

Server server = { 0 };
int client_coming_callback(SOCKET client_sock, char* ip) {//�ͻ�����
	char buf[100] = { 0 };
	printf("\nclient_coming:\nip:%s\nsocket:%d", ip, client_sock);//��ʾ�ͻ���Ϣ
	sprintf(buf, "IP:%s\n", ip);
	printf("\nsend:%d", (*server.method_Send_msg)(server, client_sock, buf, strlen(buf)));//��������Ϣ
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
	char buf[100] = { 0 };
	printf("\n�������ݣ�");
	scanf("%s", buf);//��������
	printf("\nsend:%d", (*server.method_Send_msg)(server, client_sock, buf, strlen(buf)));//����
}

void error_callback(SOCKET client_sock, int error) {//�����쳣
	printf("\nerror:%d", error);
}

void main() {

	struct timeval tim = { 1,0 };//��������ѯ���ͻ��˳�ʱʱ��


	Get_object_server(&server);//�ö��󷽷�
	{//�ö����Ա����
		server.member_buf_lenth = 1024;
		server.member_timeout = tim;
		server.member_callback_client_coming = client_coming_callback;
		server.member_callback_data_coming = data_coming_callback;
		server.member_callback_error = error_callback;
		server.member_callback_client_leave = client_leave_callback;
		printf("Monitored port��"); scanf("%d", &(server.member_port));//�����������Ķ˿�
	}
	if (Startup_sock_api() == -1)return;//startup_sock_api() �������ȵ���(ͬһ���̵���һ�μ���)

	printf("\nstartup succeed");

	printf("\nreturn:%d", (*server.method_Create_serversock)(&server));
	getchar(); getchar();
}
