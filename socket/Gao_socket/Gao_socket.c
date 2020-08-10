#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")//��VS2008���ϵİ汾�У�ʹ��socketʱ��Ҫ���ӿ�: Ws2_32.lib
//socket + address �ṹ��
typedef struct socket_state {
	SOCKET socket;
	SOCKADDR_IN socket_addr;
}Client_state, Server_state;


//--------------------ȫ�ֱ���
Client_state client = { 0 }, c_server = { 0 };//clientsock
Server_state server = { 0 }, * s_client = NULL;//seversock
WSADATA wsadata;
char close_ser = 0, close_cli = 0;//�����������
int error_sock = 0;//������Ϣ

//********************************************���µڶ������������ڿ�ͷ����********************************************
//-----------�����
int cleanup_sock_api()
{
	return WSACleanup();
}
//-----------��socket�⣨��ʼ��socket��
int startup_sock_api()
{
	return WSAStartup(MAKEWORD(2, 2), &wsadata);
}
//*********************************************���ڰ� api ������**************************************************

//--------------����ɾ����Ա�������壨˽�У�
void delete_member(struct socket_state* (*arr), int number, int size)
{//1.�����׵�ַ  2.��ɾ���±�  3.�����½�
	if (size == 0)//��ʣһ�� client ʱ���˳��������½�Ϊ 0 
	{
		free(*arr);//ֱ���ͷ�
		*arr = NULL;//��NULL
		return;//����
	}
	struct socket_state* temp = *arr;//��ʱָ�뱣��ԭ�ռ�
	for (int i = number; i < size; i++)
	{//����
		(*arr)[i] = (*arr)[i + 1];
	}
	*arr = (struct socket_state*) realloc(*arr, size * sizeof(struct socket_state));//�ض�ĩβ��һ����Ա
	if (*arr == NULL)
	{
		free(temp);//����ʧ�ܣ��ͷ�ԭ�ռ�
	}
}


//----------------------------------------server��������
int create_serversock(
	int port, //server �˿ڣ��� 0 Ϊ����˿�
	struct timeval timv, //select ��ʱʱ�䣬��Ҫ�ṩһ�� timeval
	int (*client_coming_callback)(SOCKET client_sock, char* ip),//client �������� �ص�����
	void (*client_leave_callback)(SOCKET client_sock, char* ip,int state),//client �뿪 �ص�����
	void (*data_coming_callback)(SOCKET client_sock, char* ip, char* data),//client ���ݵ��� �ص�����
	void (*error_callback)(SOCKET client_sock, int error)//�쳣����ص�������������Ա debug
)
{
	server.socket = 0;
	free(s_client); s_client = NULL;//�ͷ� client �ڴ�
	int arr_size = -1;//client �����½�
	Client_state sock_temp = { 0 };//��ʱ socket
	fd_set source_REDfd, temp_REDfd;//���ļ����������� �� ��ʱ�ļ�����������
	int size_addr = sizeof(struct sockaddr);//���� accept() ���ĸ�����
	char buf[1024] = { 0 };//���ݽ��ջ�����

	{//���� server_addr
		server.socket_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//inet_addr()���ڽ� ʮ���Ƶ��ip ת��Ϊ �����ֽ�˳����ת����Ϊ inet_ntoa()��
		server.socket_addr.sin_family = AF_INET;//IPV4Э��
		server.socket_addr.sin_port = port == 0 ? htons(INADDR_ANY) : htons(port);//htons()��htonl�������ͣ��� int ת��Ϊ�����ֽ�˳����ת����Ϊ ntohs()��ntohl�������ͣ���
	}

	//��ȡһ�� socket ������
	if (server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), server.socket < 0)
	{
		free(s_client);
		error_sock = WSAGetLastError();
		return -2;//socket ����ʧ��
	}

	//�󶨶˿ڣ����� server �������б�Ҫ֪�����ǵĶ˿��Ƕ��٣����� client ֪����Ҫ���ӵ����
	if (bind(server.socket, (struct sockaddr*) & (server.socket_addr), sizeof(struct sockaddr)) < 0)
	{
		free(s_client);
		error_sock = WSAGetLastError();
		return -3;//bind �˿�ʧ��
	}

	//��ʼ�����˿���Ϣ
	if (listen(server.socket, 2) < 0)
	{
		free(s_client);
		error_sock = WSAGetLastError();													//FD_CLR(int fd, fd_set * set) //- �Ӽ�������ȥfd
		return -4;//listen ʧ��															//FD_SET(int fd, fd_set * set)// - ���fd������
	}																					//FD_ISSET(int fd, fd_set * set)// - ����fd�Ƿ��ڼ�����
	FD_ZERO(&temp_REDfd);																//FD_ZERO(fd_set * set) //- ���һ���ļ�����������
	FD_SET(server.socket, &temp_REDfd);

	FD_ZERO(&source_REDfd);
	FD_SET(server.socket, &source_REDfd);
	//ѭ����Ϣ��select������
	while (1)
	{
		select(0, &temp_REDfd, NULL, NULL, &timv);// windows �µ�һ�����������ڱ��ּ����ԣ����Ժ���
		if (close_ser)
		{
			close_ser = 0;//����
			return 1;//�ⲿ�ر� socket �����, 1 Ϊ��������
		}

		//1. �����ж� server �� listen 
		if (FD_ISSET(server.socket, &temp_REDfd))
		{//client ��������
			sock_temp.socket = accept(server.socket, (struct sockaddr*) & (sock_temp.socket_addr), &size_addr);
			//�������Э�飬�ص��������� -1 ��Ϊ������ client socket
			if (client_coming_callback(sock_temp.socket, inet_ntoa(sock_temp.socket_addr.sin_addr)) != -1)//���ûص�����
			{
				//�������ڴ�
				struct socket_state* temp = s_client;
				s_client = (struct socket_state*)realloc(s_client, (++arr_size + 1) * sizeof(struct socket_state));//ar_size Ϊ�����½磬�������½�� + 1 Ϊ�����³���
				if (s_client == NULL)
				{
					free(temp);//����ʧ�ܣ��ͷ�ԭ�ռ�
					return -5;//���� -5
				}
				s_client[arr_size] = sock_temp;//�õ� accept() �� client ��Ϣ
				FD_SET(sock_temp.socket, &source_REDfd);//��ӵ�������������
			}
			else
			{//������ socket
				closesocket(sock_temp.socket);//�ر� accept() �� socket
			}
		}

		//2. ���жϸ� client �Ƿ�ɶ� 
		for (int i = 0; i <= arr_size; i++)
		{
			if (FD_ISSET(s_client[i].socket, &temp_REDfd))
			{

				memset(buf, 0, 1024);//��ջ�����
				int temp = recv(s_client[i].socket, buf, 1023, 0);
				//ȡ������
				if (temp == 0 )//���� 0 ��client �ر��� socket
				{//���ͣ� �� select() �õ����ļ��������д��ڴ� client sock��ȴδ�õ����ݣ�˵�� client �����ر��� socket���Ͽ�������
					FD_CLR(s_client[i].socket, &source_REDfd);//�ڼ�����ȥ��������
					closesocket(s_client[i].socket);//�ر��뿪 client socket
					client_leave_callback(s_client[i].socket, inet_ntoa(s_client[i].socket_addr.sin_addr),-4);//���� client_leave �ص�����
					delete_member(&s_client, i, arr_size);//ɾ���뿪 client ���ڴ�
					arr_size--;//�����½� - 1 
					if (s_client == NULL && arr_size != -1)//�������� 0 ��Ա���,��ʱ arr_size == -1 ����������Ա��Ҫ���� realloc(p,size)  (pָ��NULLʱ �൱�� malloc(size))
					{//Ҳ����˵���� s_client ָ�� NULL ʱ��һ�����ڴ����ʧ�ܣ�Ҳ�����������ԱΪ 0 �ĵȴ� client �����״̬
						//�ڴ����ʧ�ܣ�delete_member()���ͷŹ��ռ䣬ֱ�ӷ��ؼ���
						return -6;//�����˳������� -6
					}
				}
				else if (WSAGetLastError() == 10054) //������Ϣ���� 10054 ��client �쳣�˳�����ʱ recv ���� -1
				{//���ͣ� 10054 ��ʾ client �쳣�Ͽ���һ����δ�ر� socket ʱ client �쳣�ر�
					FD_CLR(s_client[i].socket, &source_REDfd);
					closesocket(s_client[i].socket);//�ر��뿪 client socket
					client_leave_callback(s_client[i].socket, inet_ntoa(s_client[i].socket_addr.sin_addr),-5);//���� client_leave �ص�����
					delete_member(&s_client, i, arr_size);//ɾ���뿪 client ���ڴ�
					arr_size--;//�����½� -1 
					if (s_client == NULL && arr_size != -1)
					{//�ڴ����ʧ�ܣ�delete_member()���ͷŹ��ռ䣬ֱ�ӷ��ؼ���
						return -7;//�쳣�˳������� -7
					}
				}
				else if (WSAGetLastError() == 10053) //������Ϣ���� 10053
				{//���ͣ� 10053 ��ʾ�������ֹ���ѽ��������ӣ��������������ݴ��䳬ʱ��Э�������ɵġ�
					FD_CLR(s_client[i].socket, &source_REDfd);
					closesocket(s_client[i].socket);//�ر��뿪 client socket
					client_leave_callback(s_client[i].socket, inet_ntoa(s_client[i].socket_addr.sin_addr), -5);//���� client_leave �ص�����
					delete_member(&s_client, i, arr_size);//ɾ���뿪 client ���ڴ�
					arr_size--;//�����½� -1 
					if (s_client == NULL && arr_size != -1)
					{//�ڴ����ʧ�ܣ�delete_member()���ͷŹ��ռ䣬ֱ�ӷ��ؼ���
						return -7;//�쳣�˳������� -7
					}
				}
				else if (temp < 0)//�����쳣��ԭ��δ֪
				{
					error_sock = WSAGetLastError();
					error_callback(s_client[i].socket, error_sock);//�����쳣�������쳣�ص�������������Ա debug
				}
				else
				{
					data_coming_callback(s_client[i].socket, inet_ntoa(s_client[i].socket_addr.sin_addr), buf);//���� data_coming �ص�����
				}
				//-1 Ϊ����0 Ϊ�����رգ�>0 ��ʾ����ȡ������,      ע�⣬��������ζ�����ӶϿ�
				//Ϊʲô���ж� recv() < 0 �������ת���жϴ�����Ϣ�ķ���ֵ��
				//��Ϊrecv()���� -1 ��һ���� client �Ͽ����ӣ�����������ԭ�򣬶�������Ϣ 10054 ���� client һ���쳣�˳�
			}
		}
		temp_REDfd = source_REDfd;
	}
}

//------------------------------------------------------------client��������
int create_clientsock(
	int port, //server �˿�
	char* ip, //server IP ��ַ
	struct timeval timv, //select ��ʱʱ�䣬��Ҫ�ṩһ�� timeval
	void (*server_leave_callback)(SOCKET client_sock,int error),//server �뿪 �ص�����
	void (*data_coming_callback)(SOCKET client_sock, char* data),//server ���ݵ��� �ص�����
	void (*connect_succeed_callback)(SOCKET client_sock),//connect �ɹ� �ص�����
	void (*error_callback)(int error)//�쳣����ص�������������Ա debug
)
{
	client.socket = 0;
	fd_set source_REDfd, temp_REDfd;
	char buf[1024] = { 0 };
	{//����Ŀ������ addr
		c_server.socket_addr.sin_addr.S_un.S_addr = inet_addr(ip);
		c_server.socket_addr.sin_family = AF_INET;
		c_server.socket_addr.sin_port = htons(port);
	}
	if (client.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), client.socket < 0)
	{
		error_sock = WSAGetLastError();
		return -1;//socket ʧ��
	}
	if (connect(client.socket, (struct sockaddr*) & (c_server.socket_addr), sizeof(struct sockaddr)) < 0)
	{
		error_sock = WSAGetLastError();
		return -2;//connect ʧ��
	}
	connect_succeed_callback(client.socket);//���� connect �ɹ��ص�����

	FD_ZERO(&temp_REDfd);
	FD_SET(client.socket, &temp_REDfd);

	FD_ZERO(&source_REDfd);
	FD_SET(client.socket, &source_REDfd);

	//��ʼѭ����Ϣ
	while (1)
	{
		select(0, &temp_REDfd, NULL, NULL, &timv);
		if (close_cli)
		{
			close_cli = 0;
			return 1;//��������
		}
		if (FD_ISSET(client.socket, &temp_REDfd))//�ɶ�
		{
			memset(buf, 0, 1024);//��ջ�����
			int temp = recv(client.socket, buf, 1023, 0);
			error_sock = WSAGetLastError();
			if (temp == 0)//-1 Ϊ����0 Ϊ�����رգ�>0 ��ʾ����ȡ������,      ע�⣬��������ζ�����ӶϿ�
			{//��������˹ر� client socket ������ 0��        ����˷������رգ����� -1��        ȡ���������� ���ػ������ַ�������
				closesocket(client.socket);//�ر� socket��׼������
				server_leave_callback(client.socket,-3);//���ûص�����������Ա�����¼�
				return -3;//����������ر����ӣ����� -3
			}
			else if (WSAGetLastError() == 10054)//10054����server �쳣�˳�
			{
				closesocket(client.socket);//�ر� socket��׼������
				server_leave_callback(client.socket, -4);//���ûص�����������Ա�����¼�
				return -4;//������쳣�˳������� -4
			}
			else if (temp < 0)//�쳣��ԭ��δ֪
			{
				error_sock = WSAGetLastError();
				error_callback(error_sock);//δ֪ԭ�򣬵��ûص�����������Ա debug
			}
			else
			{
				data_coming_callback(client.socket, buf);//�����������ݣ����ûص�����

			}

		}
		temp_REDfd = source_REDfd;
	}
}

//------------------------------------------------------------------------------------------------------��������
int send_msg(SOCKET client_sock, char* msg, int lenth)
{//��֤�������н�β \0 ,���ṩ��׼��lenth
	if (send(client_sock, msg, lenth, 0) < 0)//����
	{
		error_sock = WSAGetLastError();
		return -1;
	}
	return 0;

}

//--------------------------------------------------------------------------------close server
int close_serversock() //�ر� server ���� accept() ���� client socket
{//�ɹ��رշ��� 0 ,�����򷵻� SOCKET_ERROR
	close_ser = 1;//��־�� 1 ���� server ����
	free(s_client);//�ͷſռ�
	return closesocket(server.socket);
}

//--------------------------------------------------------------------------------close client
int close_clientsock() //�ر� client socket
{//�ɹ��رշ��� 0 ,�����򷵻� SOCKET_ERROR
	close_cli = 1;//��־�� 1 ���� client ����
	return closesocket(client.socket);
}

//--------------------------------��ȡclient_socket
SOCKET get_client()
{
	return client.socket;
}
//--------------------------------��ȡserver_socket
SOCKET get_server()
{
	return server.socket;
}
//-----------��ȡ��һ�εĴ�����Ϣ
int get_error()
{
	return error_sock;
}



