#pragma once
#ifndef GAO_LIHAI_sock_api
#define GAO_LIHAI_sock_api

#include<winsock2.h>

#include<string.h>

#define NORMAL_CLOSE 1//�������ֶ���������

//server client_leave_callback �ص������ĵ��ĸ���������
#define CLIENT_NOR_CLOSE -4//�ͻ������˳�
#define CLIENT_UNNOR_CLOSE -5//�ͻ��������˳�
//server client_coming_callback �ص������ķ��س���
#define ACCEPT_CLIENT 1//���������ܿͻ�
#define REFUSE_CLIENT -1//���������Կͻ�
//server �����ķ��س���
#define ERROR_SER_MEM_1 -1//��ʼʱ�ڴ����ʧ��
#define ERROR_SER_SOCK -2//��ȡ socket ʧ��
#define ERROR_SER_BIND -3//bind port ʧ��
#define ERROR_SER_LISTEN -4//listen ʧ��
#define ERROR_SER_MEM_2 -5//�ͻ�����ʱ�ڴ����ʧ��
#define ERROR_SER_NOR_CLOSE -6//�ͻ������˳�ʱ�ڴ����ʧ��
#define ERROR_SER_UNNOR_CLOSE -7//�ͻ��������˳�ʱ�ڴ����ʧ��

//client �����ķ��س���
#define ERROR_CLI_SOCK -1//��ȡ socket ʧ��
#define ERROR_CLI_CNCT -2//connect ʧ��
#define ERROR_CLI_SERVERCLOSE_1 -3//����������ر�����
#define ERROR_CLI_SERVERCLOSE_2 -4//������쳣�˳�
//int error_sock;//������Ϣ
/*
typedef struct socket_state {
	SOCKET socket;
	SOCKADDR_IN socket_addr;
}Client_state, Server_state;
*/
//----------------------------------------server����
int create_serversock(
	int port, //server �˿�
	struct timeval timv, //select ��ʱʱ�䣬��Ҫ�ṩһ�� timeval
	int (*client_coming_callback)(SOCKET client_sock, char* ip),//client �������� �ص�����
	void (*client_leave_callback)(SOCKET client_sock, char* ip, int state),//client �뿪 �ص����������ĸ�����
	void (*data_coming_callback)(SOCKET client_sock, char* ip, char* data),//client ���ݵ��� �ص�����
	void (*error_callback)(SOCKET client_sock, int error)//�쳣����ص�������������Ա debug
);

//------------------------------------------------------------client����
int create_clientsock(
	int port, //server �˿�
	char* ip, //server IP ��ַ
	struct timeval timv, //select ��ʱʱ�䣬��Ҫ�ṩһ�� timeval
	void (*server_leave_callback)(SOCKET client_sock, int error),//server �뿪 �ص�����
	void (*data_coming_callback)(SOCKET client_sock, char* data),//server ���ݵ��� �ص�����
	void (*connect_succeed_callback)(SOCKET client_sock),//connect �ɹ� �ص�����
	void (*error_callback)(int error)//�쳣����ص�������������Ա debug
);

//------------------------------------------------------------------------------------------------------��������
int send_msg(SOCKET client_sock, char* msg, int lenth);

int close_serversock(); //�ر� server ���� accept() ���� client socket

int close_clientsock();//�ر� client socket

//-----------��socket�⣨��ʼ��socket��
int startup_sock_api();

//-----------�����
int cleanup_sock_api();

//--------------------------------��ȡserver_socket
SOCKET get_server();

//--------------------------------��ȡclient_socket
SOCKET get_client();

//-----------��ȡ��һ�εĴ�����Ϣ
int get_error();

#endif
