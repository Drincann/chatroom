
//socket �ӿ�
#include"Gao_socket.h"//winsock2.h ������ windows.h
#pragma comment(lib,"Gao_socket.lib")

//�ַ�������
#include<stdio.h>


//��ȡʱ��
#include<time.h>

// history string
#include "HistoryMessage.cpp"
//�������Ϣ
#define PORT 50055//����˼����˿�

//����/ɾ�� �ͻ�ʱ�ķ���ֵ
#define SUCCESS 1
#define FAIL -1



//-----------------------------------------------����-----------------------------------------------

//*************��������*******************
SOCKET* client;
int arr_Lowsub = -1;
HistoryMessage history;

//*************��������*******************
//����˻ص�����
int client_coming_callback(SOCKET client_sock, char* ip);
void client_leave_callback(SOCKET client_sock, char* ip, int state);
void data_coming_callback(SOCKET client_sock, char* ip, char* data);
void error_callback(SOCKET client_sock, int error);

//����
int sock_add(SOCKET sock_data, SOCKET* (*socket), int* Lowsub);
int sock_del(SOCKET* (*arr), int sub, int* Lowsub);




//-----------------------------------------------������-----------------------------------------------
int main()
{
	startup_sock_api();
	struct timeval timv = { 2,0 };
	//socket �����߳��е���
	create_serversock(PORT, timv, client_coming_callback, client_leave_callback, data_coming_callback, error_callback);
}




//*************socket�ĸ��ص�*******************
int client_coming_callback(SOCKET client_sock, char* ip)//client �������� �ص�����
{
	while (sock_add(client_sock, &client, &arr_Lowsub) == FAIL)
	{
		if (MessageBox(NULL, "Client�ڴ����ʧ�ܣ�", "����", MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_ICONWARNING) != IDRETRY)
			exit(-1);
	}

	//�ַ�����������Ϣ���ͻ���
	char buf[10] = { 0 };
	sprintf(buf, " %d", arr_Lowsub + 1);//ǰ׺�ո�����͵�����Ϊ��������
	for (int i = 0; i <= arr_Lowsub; i++)
	{
		send_msg(client[i], buf, strlen(buf));
	}
	send_msg(client_sock, (char*)history.getHistoryStr(), strlen(history.getHistoryStr()));
	printf("client �������� �ص�����:%s\nsocket:%d\narrsize:%d\n\n", ip, client_sock, arr_Lowsub);
	return ACCEPT_CLIENT;
}


void client_leave_callback(SOCKET client_sock, char* ip, int state)//client �뿪 �ص�����
{
	int i = 0;
	for (; i <= arr_Lowsub; i++)
	{
		if (client[i] == client_sock)break;
	}
	while (sock_del(&client, i, &arr_Lowsub) == FAIL)
	{
		if (MessageBox(NULL, "Client�ڴ����ʧ�ܣ�", "����", MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_ICONWARNING) != IDRETRY)
			exit(-1);
	}

	//�ַ�����������Ϣ���ͻ���
	char buf[10] = { 0 };
	sprintf(buf, " %d", arr_Lowsub + 1);//ǰ׺�ո�����͵�����Ϊ��������
	for (int i = 0; i <= arr_Lowsub; i++)
	{
		send_msg(client[i], buf, strlen(buf));
	}
	printf("client �뿪 �ص�����:%s\nsocket:%d\nstate:%d\narrsize:%d\n\n", ip, client_sock, state, arr_Lowsub);
}


void data_coming_callback(SOCKET client_sock, char* ip, char* data)//client ���ݵ��� �ص�����
{
	printf("client ���ݵ��� �ص�����:%s\nsocket:%d\ndata:%s\narrsize:%d\n\n", ip, client_sock, data, arr_Lowsub);
	//�ͻ��˱�֤���ǽ��յ��������಻����1024k
	//ƴ����Ϣ
	char str[1100] = { 0 };//������Ҫƴ��ʱ���ı����ʻ������Դ�һЩ����ֹstrcat���
	time_t tm = time(0);
	  //1.ƴ��ʱ��
	strcat(str, ctime(&tm));
	str[strlen(str) - 1] = 0;//ȥ��ctimeĩβ��\0
	strcat(str, "\r\n");

	  //2.ƴ�ӿͻ�������
	strcat(str, data);
													//����˷��ص��������ݰ���
	  //3.ƴ�ӻ���									//1.ʱ�� + ���� + ������ + ���� + ���ݣ� + ���� + ����
	str[1019] = 0;									//�ͻ��˸��� �ͻ������� =������ + ���� + ���ݣ�
	strcat(str, "\r\n\r\n");                        //����˸��� ʱ�� + ���� + ƴ�ӿͻ������� + ���� + ����
	str[1023] = 0;//ĩβ��0����֤���͵����ݲ����� 1kb
	history.pushMessage(str);
	//�ַ�������Ϣ���ͻ���
	for (int i = 0; i <= arr_Lowsub; i++)
	{
		send_msg(client[i],str,strlen(str));
	}
}


void error_callback(SOCKET client_sock, int error)//�쳣����ص�������������Ա debug
{
	//���Զϵ�ʱ�ļ���������»����10038����
	//�ӿ���ûд��10038������Ĵ���������Ľ����Ƿ���������ر�ĳsocket������ͼʹ�ø�socket����ʱû���˼·
	printf("�쳣����ص�����\nsocket:%d\nerror:%d\narrsize:%d\n\n", client_sock, error, arr_Lowsub);
	char buf[100] = { 0 };
	sprintf(buf,"error:%d\n",error);
	OutputDebugString(buf);
	printf("error:%d\n",error);
}




//*************�����������*******************
//���ӳ�Ա
int sock_add(SOCKET sock_data,SOCKET* (*socket), int* Lowsub)
{
	SOCKET* temp = *socket;
	*socket = (SOCKET*)realloc(*socket, (*Lowsub + 2) * sizeof(unsigned int));
	if (*socket == NULL)
	{
		*socket = temp;
		return FAIL;//ʧ��
	}
	else
	{

		(*socket)[++(*Lowsub)] = sock_data;
		return SUCCESS;//�ɹ�
	}

}

//ɾ����Ա
int sock_del(SOCKET* (*arr), int sub, int* Lowsub)
{//1.�����׵�ַ  2.��ɾ���±�  3.�����½�
	if (*Lowsub == 0)//��ʣһ����Աʱ���˳��������½�Ϊ 0 
	{
		free(*arr);//ֱ���ͷ�
		*arr = NULL;//��NULL
		(*Lowsub)--;
		return SUCCESS;//���� 1,�ɹ�
	}

	SOCKET* temp = *arr;//��ʱָ�뱣��ԭ�ռ��ַ
	for (int i = sub; i < *Lowsub; i++)
	{//����
		(*arr)[i] = (*arr)[i + 1];
	}

	*arr = (SOCKET*) realloc(*arr, *Lowsub * sizeof(SOCKET));//�ض�ĩβ��һ����Ա
	if (*arr == NULL)
	{
		*arr = temp;//����ʧ�ܣ�����ԭָ��
		return FAIL;//���� -1,ʧ��
	}
	else
	{
		(*Lowsub)--;
		return SUCCESS;//���� 1,�ɹ�
	}
}