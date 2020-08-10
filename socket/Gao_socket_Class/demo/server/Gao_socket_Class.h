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

//socket + address �ṹ��
typedef struct socket_state {
	SOCKET socket;
	SOCKADDR_IN socket_addr;
}Client_state, Server_state;

//������ʱ����Ҫ������ǰdefine
typedef struct Server_ Server;
typedef struct Client_ Client;
//********************************************�ඨ��********************************************


//--------------------����� ��
struct Server_ {

	//private��Ա
	Server_state private_1, * private_2;
	char private_3;
	int private_4;

	//public��Ա
	unsigned short member_port;//�˿�
	struct timeval member_timeout;//select ��ʱʱ�䣬��Ҫ�ṩһ�� timeval
	int member_buf_lenth;//���ջ���������
	int (*member_callback_client_coming)(SOCKET client_sock, char* ip);//client �������� �ص�����
	void (*member_callback_client_leave)(SOCKET client_sock, char* ip, int state);//client �뿪 �ص�����
	void (*member_callback_data_coming)(SOCKET client_sock, char* ip, char* data);//client ���ݵ��� �ص�����
	void (*member_callback_error)(SOCKET client_sock, int error);//�쳣����ص�������������Ա debug

	//����
	int (*method_Create_serversock)(Server* Server_Class);//���������

	int (*method_Send_msg)(Server Server_Class, SOCKET client_sock, char* msg, int lenth);//��������

	int (*method_Close_serversock)(Server Server_Class);//�رշ����

	int (*method_Get_error_server)(Server Server_Class);//��ȡ������Ϣ

	SOCKET(*method_Get_serversock)(Server Server_Class);
};


//--------------------�ͻ��� ��
struct Client_ {

	//private��Ա
	Client_state private_1, private_2;
	char private_3;
	int private_4;

	//public��Ա
	unsigned short member_port;//server �˿�
	char member_ip[16]; //server IP ��ַ
	struct timeval member_timeout; //select ��ʱʱ�䣬��Ҫ�ṩһ�� timeval
	int member_buf_lenth;//���ջ���������
	void (*member_callback_server_leave)(SOCKET client_sock, int error);//server �뿪 �ص�����
	void (*member_callback_data_coming)(SOCKET client_sock, char* data);//server ���ݵ��� �ص�����
	void (*member_callback_connect_succeed)(SOCKET client_sock); //connect �ɹ� �ص�����
	void (*member_callback_error)(int error);//�쳣����ص�������������Ա debug

	//����
	int (*method_Create_clientsock)(Client* Client_Class);

	int (*method_Send_msg)(Client Client_Class, char* msg, int lenth);

	int (*method_Close_clientsock)(Client Client_Class);

	int (*method_Get_error_client)(Client Client_Class);

	SOCKET(*method_Get_clientsock)(Client Client_Class);
};
//********************************************�ඨ�����********************************************








//********************************************���µڶ������������ڿ�ͷ����********************************************
//һ�����̵���һ�μ��ɣ����̲߳����ظ�����

//-----------��socket�⣨��ʼ��socket��
int Startup_sock_api();

//-----------�����
int Cleanup_sock_api();

//*********************************************���ڰ� api ������**************************************************








//*********************************************get����**************************************************
//get����˶���
void Get_object_server(Server* Server_Class);

//get�ͻ��˶���
void Get_object_client(Client* Client_Class);
//*********************************************get����**************************************************

#endif
