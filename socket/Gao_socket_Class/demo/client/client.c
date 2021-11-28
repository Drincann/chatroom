#include"Gao_socket_Class.h"
#pragma comment(lib,"Gao_socket_Class.lib")
#include<stdio.h>
Client client = { 0 };
void server_leave_callback(int error) {//����˶Ͽ�����
	printf("\nerver_leave:%d", error);
}
void data_coming_callback(SOCKET client_sock, char* data) {//���ݵ���
	char buf[100] = { 0 };
	printf("\ndata_coming:%s", data);
	printf("\n�������ݣ�");
	scanf("%s", buf);//��������
	printf("\nsend:%d", (*client.method_Send_msg)(client, buf, strlen(buf)));//����
}
void connect_succeed_callback(SOCKET client_sock) {//connect �ɹ� �ص�����
	printf("\nconnect_succeed");
}
void error_callback(SOCKET client_sock, int error) {//�����쳣
	printf("\nerror:%d", error);

}
void main() {
	struct timeval tim = { 1,0 };


	Get_object_client(&client);//�ö��󷽷�

	{//�ö����Ա����
		client.member_buf_lenth = 1024;
		client.member_timeout = tim;
		client.member_callback_connect_succeed = connect_succeed_callback;
		client.member_callback_data_coming = data_coming_callback;
		client.member_callback_error = error_callback;
		client.member_callback_server_leave = server_leave_callback;
		printf("Ip��"); scanf("%s",client.member_ip);//���� server ip��ַ
		printf("Port��"); scanf("%hu", &client.member_port);//���� server �����˿�
	}

	if (Startup_sock_api() == -1)return;//startup_sock_api() �������ȵ���(ͬһ���̵���һ�μ���)

	printf("\nstartup succeed");

	printf("\nreturn:%d", client.method_Create_clientsock(&client));
	printf("\nerror:%d", client.method_Get_error_client(client));

	getchar(); getchar();
}
