#include"Gao_socket.h"
#pragma comment(lib,"Gao_socket.lib")
#include<stdio.h>
void server_leave_callback(int error) {//����˶Ͽ�����
	printf("\nerver_leave:%d", error);
}
void data_coming_callback(SOCKET client_sock, char* data) {//���ݵ���
	char buf[100] = { 0 };
	printf("\ndata_coming:%s", data);
	scanf("%s", buf);//��������
	printf("\nsend:%d", send_msg(get_client(), buf, strlen(buf)));//����
}
void connect_succeed_callback(SOCKET client_sock) {//connect �ɹ� �ص�����
	printf("\nconnect_succeed");
}
void error_callback(SOCKET client_sock, int error) {//�����쳣
	printf("\nerror:%d", error);

}
void main() {
	struct timeval tim = { 1,0 };
	char ip[20];
	int port;
	printf("Ip��"); gets(&ip);//���� server ip��ַ
	printf("Port��"); scanf("%d", &port);//���� server �����˿�
	printf("\nstart:%d", startup_sock_api());//startup_sock_api() �������ȵ���
	printf("\nreturn:%d", create_clientsock(port, &ip, tim, server_leave_callback, data_coming_callback, connect_succeed_callback, error_callback));
	getchar(); getchar();
}
