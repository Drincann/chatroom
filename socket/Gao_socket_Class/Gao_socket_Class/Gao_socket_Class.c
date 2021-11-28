#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")//��VS2008���ϵİ汾�У�ʹ��socketʱ��Ҫ���ӿ�: Ws2_32.lib
typedef struct Server_ Server;//������ʱ����Ҫ������ǰdefine
typedef struct Client_ Client;//������ʱ����Ҫ������ǰdefine

WSADATA wsadata;

//socket + address �ṹ��
typedef struct socket_state {
	SOCKET socket;
	SOCKADDR_IN socket_addr;
}Client_state, Server_state;

//���к���
int Create_serversock(Server* Server_Class);

int Create_clientsock(Client* Client_Class);

int Send_msg_server(Server* Server_Class, SOCKET client_sock, char* msg, int lenth);

int Send_msg_client(Client* Client_Class,  char* msg, int lenth);

int Close_serversock(Server* Server_Class);

int Close_clientsock(Client* Client_Class);

int Get_error_server(Server* Server_Class);

int Get_error_client(Client* Client_Class);

SOCKET Get_serversock(Server* Server_Class);

SOCKET Get_clientsock(Client* Client_Class);

int Cleanup_sock_api();

int Startup_sock_api();

void delete_member(struct socket_state* (*arr), int number, int size);

void Get_object_server(Server* Server_Class);

void Get_object_client(Client* Client_Class);

//********************************************�ඨ��********************************************



//--------------------����� ��
struct Server_ {

	//private��Ա
	Server_state server, * s_client;//seversock
	char close_ser;//�����������
	int error_sock;//������Ϣ

	//public��Ա
	unsigned short port;//�˿�
	struct timeval timeout;//select ��ʱʱ�䣬��Ҫ�ṩһ�� timeval
	int buf_lenth;//���ջ���������
	int (*client_coming_callback)(SOCKET client_sock, char* ip);//client �������� �ص�����
	void (*client_leave_callback)(SOCKET client_sock, char* ip, int state);//client �뿪 �ص�����
	void (*data_coming_callback)(SOCKET client_sock, char* ip, char* data);//client ���ݵ��� �ص�����
	void (*error_callback)(SOCKET client_sock, int error);//�쳣����ص�������������Ա debug

	//����
	int (*Create_serversock)(Server* Server_Class);//���������

	int (*Send_msg)(Server* Server_Class, SOCKET client_sock, char* msg, int lenth);//��������

	int (*Close_serversock)(Server* Server_Class);//�رշ����

	int (*Get_error_server)(Server* Server_Class);//��ȡ������Ϣ

	SOCKET (*Get_serversock)(Server* Server_Class);
};


//--------------------�ͻ��� ��
struct Client_ {

	//private��Ա
	Client_state client, c_server;//clientsock
	char close_cli;//�����������
	int error_sock;//������Ϣ

	//public��Ա
	unsigned short port;//server �˿�
	char* ip; //server IP ��ַ
	struct timeval timeout; //select ��ʱʱ�䣬��Ҫ�ṩһ�� timeval
	int buf_lenth;//���ջ���������
	void (*server_leave_callback)(SOCKET client_sock, int error);//server �뿪 �ص�����
	void (*data_coming_callback)(SOCKET client_sock, char* data);//server ���ݵ��� �ص�����
	void (*connect_succeed_callback)(SOCKET client_sock); //connect �ɹ� �ص�����
	void (*error_callback)(int error);//�쳣����ص�������������Ա debug

	//����
	int (*Create_clientsock)(Client* Client_Class);

	int (*Send_msg)(Client* Client_Class, char* msg, int lenth);

	int (*Close_clientsock)(Client* Client_Class);

	int (*Get_error_client)(Client* Client_Class);

	SOCKET (*Get_clientsock)(Client* Client_Class);
};
//********************************************�ඨ�����********************************************








//********************************************���µڶ������������ڿ�ͷ����********************************************
//һ�����̵���һ�μ��ɣ����̲߳����ظ�����
//-----------�����
int Cleanup_sock_api()
{
	return WSACleanup();
}
//-----------��socket�⣨��ʼ��socket��
int Startup_sock_api()
{
	return WSAStartup(MAKEWORD(2, 2), &wsadata);
}
//*********************************************���ڰ� api ������**************************************************








//get����˶���
void Get_object_server(Server* Server_Class)
{
	//����
	Server_Class->Create_serversock = Create_serversock;
	Server_Class->Close_serversock = Close_serversock;
	Server_Class->Send_msg = Send_msg_server;
	Server_Class->Get_error_server = Get_error_server;
	Server_Class->Get_serversock = Get_serversock;
}
//get�ͻ��˶���
void Get_object_client(Client* Client_Class)
{
	//����
	Client_Class->Create_clientsock = Create_clientsock;
	Client_Class->Close_clientsock = Close_clientsock;
	Client_Class->Send_msg = Send_msg_client;
	Client_Class->Get_error_client = Get_error_client;
	Client_Class->Get_clientsock = Get_clientsock;
}




//--------------����ɾ����Ա�������壨private��
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
int Create_serversock(Server* Server_Class)//�쳣����ص�������������Ա debug

{
	if (Server_Class->s_client != NULL)
	{//�ͷ� client �ڴ�
		free(Server_Class->s_client); Server_Class->s_client = NULL;
	}

	int arr_size = -1;//client �����½�
	Client_state sock_temp = { 0 };//��ʱ socket
	fd_set source_REDfd, temp_REDfd;//���ļ����������� �� ��ʱ�ļ�����������
	int size_addr = sizeof(struct sockaddr);//���� accept() ���ĸ�����
	char *buf = calloc(1, Server_Class->buf_lenth);//���ݽ��ջ�����
	if (buf == NULL)return -1;
	{//���� server_addr
		Server_Class->server.socket_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//inet_addr()���ڽ� ʮ���Ƶ��ip ת��Ϊ �����ֽ�˳����ת����Ϊ inet_ntoa()��
		Server_Class->server.socket_addr.sin_family = AF_INET;//IPV4Э��
		Server_Class->server.socket_addr.sin_port = Server_Class->port == 0 ? htons(INADDR_ANY) : htons(Server_Class->port);//htons()��htonl�������ͣ��� int ת��Ϊ�����ֽ�˳����ת����Ϊ ntohs()��ntohl�������ͣ���
	}

	//��ȡһ�� socket ������
	if (Server_Class->server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), Server_Class->server.socket < 0)
	{
		Server_Class->error_sock = WSAGetLastError();
		return -2;//socket ����ʧ��
	}

	//�󶨶˿ڣ����� server �������б�Ҫ֪�����ǵĶ˿��Ƕ��٣����� client ֪����Ҫ���ӵ����
	if (bind(Server_Class->server.socket, (struct sockaddr*) & (Server_Class->server.socket_addr), sizeof(struct sockaddr)) < 0)
	{
		Server_Class->error_sock = WSAGetLastError();
		return -3;//bind �˿�ʧ��
	}

	//��ʼ�����˿���Ϣ
	if (listen(Server_Class->server.socket, 2) < 0)
	{
		Server_Class->error_sock = WSAGetLastError();													//FD_CLR(int fd, fd_set * set) //- �Ӽ�������ȥfd
		return -4;//listen ʧ��															//FD_SET(int fd, fd_set * set)// - ���fd������
	}																					//FD_ISSET(int fd, fd_set * set)// - ����fd�Ƿ��ڼ�����
	FD_ZERO(&temp_REDfd);																//FD_ZERO(fd_set * set) //- ���һ���ļ�����������
	FD_SET(Server_Class->server.socket, &temp_REDfd);

	FD_ZERO(&source_REDfd);
	FD_SET(Server_Class->server.socket, &source_REDfd);
	//ѭ����Ϣ��select������
	while (1)
	{
		select(0, &temp_REDfd, NULL, NULL, &(Server_Class->timeout));// windows �µ�һ�����������ڱ��ּ����ԣ����Ժ���
		if (Server_Class->close_ser)//��⵽�ر�
		{
			Server_Class->close_ser = 0;//����
			free(Server_Class->s_client); Server_Class->s_client = NULL;
			free(buf);
			return 1;//�ⲿ�ر� socket �����, 1 Ϊ��������
		}

		//1. �����ж� server �� listen 
		if (FD_ISSET(Server_Class->server.socket, &temp_REDfd))
		{//client ��������
			sock_temp.socket = accept(Server_Class->server.socket, (struct sockaddr*) & (sock_temp.socket_addr), &size_addr);
			//�������Э�飬�ص��������� -1 ��Ϊ������ client socket
			if ((Server_Class->client_coming_callback == NULL ? 1 : (*Server_Class->client_coming_callback)(sock_temp.socket, inet_ntoa(sock_temp.socket_addr.sin_addr))) != -1)//���ûص�����
			{
				//�������ڴ�
				struct socket_state* temp = Server_Class->s_client;
				Server_Class->s_client = (struct socket_state*)realloc(Server_Class->s_client, (++arr_size + 1) * sizeof(struct socket_state));//ar_size Ϊ�����½磬�������½�� + 1 Ϊ�����³���
				if (Server_Class->s_client == NULL)
				{
					free(temp);//����ʧ�ܣ��ͷ�ԭ�ռ�
					return -5;//���� -5
				}
				Server_Class->s_client[arr_size] = sock_temp;//�õ� accept() �� client ��Ϣ
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
			if (FD_ISSET(Server_Class->s_client[i].socket, &temp_REDfd))
			{

				memset(buf, 0, 1024);//��ջ�����
				int temp = recv(Server_Class->s_client[i].socket, buf, 1023, 0);
				//ȡ������
				if (temp == 0)//���� 0 ��client �ر��� socket
				{//���ͣ� �� select() �õ����ļ��������д��ڴ� client sock��ȴδ�õ����ݣ�˵�� client �����ر��� socket���Ͽ�������
					FD_CLR(Server_Class->s_client[i].socket, &source_REDfd);//�ڼ�����ȥ��������
					closesocket(Server_Class->s_client[i].socket);//�ر��뿪 client socket
					if (Server_Class->client_leave_callback != NULL)
					{
						(*Server_Class->client_leave_callback)(Server_Class->s_client[i].socket, inet_ntoa(Server_Class->s_client[i].socket_addr.sin_addr), -4);//���� client_leave �ص�����
					}
					delete_member(&(Server_Class->s_client), i, arr_size);//ɾ���뿪 client ���ڴ�
					arr_size--;//�����½� - 1 
					if (Server_Class->s_client == NULL && arr_size != -1)//�������� 0 ��Ա���,��ʱ arr_size == -1 ����������Ա��Ҫ���� realloc(p,size)  (pָ��NULLʱ �൱�� malloc(size))
					{//Ҳ����˵���� s_client ָ�� NULL ʱ��һ�����ڴ����ʧ�ܣ�Ҳ�����������ԱΪ 0 �ĵȴ� client �����״̬
						//�ڴ����ʧ�ܣ�delete_member()���ͷŹ��ռ䣬ֱ�ӷ��ؼ���
						return -6;//�����˳������� -6
					}
				}
				else if (WSAGetLastError() == 10054) //������Ϣ���� 10054 ��client �쳣�˳�����ʱ recv ���� -1
				{//���ͣ� 10054 ��ʾ client �쳣�Ͽ���һ����δ�ر� socket ʱ client �쳣�ر�
					FD_CLR(Server_Class->s_client[i].socket, &source_REDfd);
					closesocket(Server_Class->s_client[i].socket);//�ر��뿪 client socket
					if (Server_Class->client_leave_callback != NULL)
					{
						(*Server_Class->client_leave_callback)(Server_Class->s_client[i].socket, inet_ntoa(Server_Class->s_client[i].socket_addr.sin_addr), -5);//���� client_leave �ص�����
					}
					delete_member(&(Server_Class->s_client), i, arr_size);//ɾ���뿪 client ���ڴ�
					arr_size--;//�����½� -1 
					if (Server_Class->s_client == NULL && arr_size != -1)
					{//�ڴ����ʧ�ܣ�delete_member()���ͷŹ��ռ䣬ֱ�ӷ��ؼ���
						return -7;//�쳣�˳������� -7
					}
				}
				else if (WSAGetLastError() == 10053) //������Ϣ���� 10053 ��client �쳣�˳�����ʱ recv ���� -1
				{//���ͣ� 10053 ��ʾ�������ֹ���ѽ��������ӣ��������������ݴ��䳬ʱ��Э�������ɵġ�
					FD_CLR(Server_Class->s_client[i].socket, &source_REDfd);
					closesocket(Server_Class->s_client[i].socket);//�ر��뿪 client socket
					if (Server_Class->client_leave_callback != NULL)
					{
						(*Server_Class->client_leave_callback)(Server_Class->s_client[i].socket, inet_ntoa(Server_Class->s_client[i].socket_addr.sin_addr), -5);//���� client_leave �ص�����
					}
					delete_member(&(Server_Class->s_client), i, arr_size);//ɾ���뿪 client ���ڴ�
					arr_size--;//�����½� -1 
					if (Server_Class->s_client == NULL && arr_size != -1)
					{//�ڴ����ʧ�ܣ�delete_member()���ͷŹ��ռ䣬ֱ�ӷ��ؼ���
						return -7;//�쳣�˳������� -7
					}
				}
				else if (temp < 0)//�����쳣��ԭ��δ֪
				{
					Server_Class->error_sock = WSAGetLastError();
					if (Server_Class->error_callback != NULL)
					{
						(*Server_Class->error_callback)(Server_Class->s_client[i].socket, Server_Class->error_sock);//�����쳣�������쳣�ص�������������Ա debug
					}
				}
				else
				{
					if (Server_Class->data_coming_callback != NULL)
					{
						(*Server_Class->data_coming_callback)(Server_Class->s_client[i].socket, inet_ntoa(Server_Class->s_client[i].socket_addr.sin_addr), buf);//���� data_coming �ص�����
					}
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
int Create_clientsock(Client* Client_Class)
{
	fd_set source_REDfd, temp_REDfd;
	char* buf = calloc(1, Client_Class->buf_lenth);//���ݽ��ջ�����
	if (buf == NULL)return -5;
	{//����Ŀ������ addr
		Client_Class->c_server.socket_addr.sin_addr.S_un.S_addr = inet_addr(Client_Class->ip);
		Client_Class->c_server.socket_addr.sin_family = AF_INET;
		Client_Class->c_server.socket_addr.sin_port = htons(Client_Class->port);
	}
	if (Client_Class->client.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), Client_Class->client.socket < 0)
	{
		Client_Class->error_sock = WSAGetLastError();
		return -1;//socket ʧ��
	}
	if (connect(Client_Class->client.socket, (struct sockaddr*) &(Client_Class->c_server.socket_addr), sizeof(struct sockaddr)) < 0)
	{
		Client_Class->error_sock = WSAGetLastError();
		return -2;//connect ʧ��
	}
	(*Client_Class->connect_succeed_callback)(Client_Class->client.socket);//���� connect �ɹ��ص�����

	FD_ZERO(&temp_REDfd);
	FD_SET(Client_Class->client.socket, &temp_REDfd);

	FD_ZERO(&source_REDfd);
	FD_SET(Client_Class->client.socket, &source_REDfd);

	//��ʼѭ����Ϣ
	while (1)
	{
		select(0, &temp_REDfd, NULL, NULL, &(Client_Class->timeout));
		if (Client_Class->close_cli)
		{
			Client_Class->close_cli = 0;
			free(buf);
			return 1;//��������
		}
		if (FD_ISSET(Client_Class->client.socket, &temp_REDfd))//�ɶ�
		{
			memset(buf, 0, 1024);//��ջ�����
			int temp = recv(Client_Class->client.socket, buf, 1023, 0);
			Client_Class->error_sock = WSAGetLastError();
			if (temp == 0)//-1 Ϊ����0 Ϊ�����رգ�>0 ��ʾ����ȡ������,      ע�⣬��������ζ�����ӶϿ�
			{//��������˹ر� client socket ������ 0��        ����˷������رգ����� -1��        ȡ���������� ���ػ������ַ�������
				closesocket(Client_Class->client.socket);//�ر� socket��׼������
				if (Client_Class->server_leave_callback != NULL)
				{
					(*Client_Class->server_leave_callback)(Client_Class->client.socket, -3);//���ûص�����������Ա�����¼�
				}
				return -3;//����������ر����ӣ����� -3
			}
			else if (WSAGetLastError() == 10054)//10054����server �쳣�˳�
			{
				closesocket(Client_Class->client.socket);//�ر� socket��׼������
				if (Client_Class->server_leave_callback != NULL)
				{
					(*Client_Class->server_leave_callback)(Client_Class->client.socket, -4);//���ûص�����������Ա�����¼�
				}
				return -4;//������쳣�˳������� -4
			}
			else if (temp < 0)//�쳣��ԭ��δ֪
			{
				Client_Class->error_sock = WSAGetLastError();
				if (Client_Class->error_callback != NULL)
				{
					(*Client_Class->error_callback)(Client_Class->error_sock);//δ֪ԭ�򣬵��ûص�����������Ա debug
				}

			}
			else
			{
				if (Client_Class->data_coming_callback != NULL)
				{
					(*Client_Class->data_coming_callback)(Client_Class->client.socket, buf);//�����������ݣ����ûص�����
				}
			}

		}
		temp_REDfd = source_REDfd;
	}
}

//------------------------------------------------------------------------------------------------------server��������
int Send_msg_server(Server* Server_Class, SOCKET client_sock, char* msg, int lenth)//���������ն�socket���ͻ�����Ͷ�socket����Ϊ����˿��ܽ����˶�����ӣ����ͻ��˽�������һ������
{//��֤�������н�β \0 ,���ṩ��׼��lenth
	if (send(client_sock, msg, lenth, 0) < 0)//����
	{
		Server_Class->error_sock = WSAGetLastError();
		return -1;
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------client��������
int Send_msg_client(Client* Client_Class, char* msg, int lenth)//���������ն�socket���ͻ�����Ͷ�socket����Ϊ����˿��ܽ����˶�����ӣ����ͻ��˽�������һ������
{//��֤�������н�β \0 ,���ṩ��׼��lenth
	if (send(Client_Class->client.socket, msg, lenth, 0) < 0)//����
	{
		Client_Class->error_sock = WSAGetLastError();
		return -1;
	}
	return 0;
}

//--------------------------------------------------------------------------------close server
int Close_serversock(Server* Server_Class) //�ر� server ���� accept() ���� client socket
{//�ɹ��رշ��� 0 ,�����򷵻� SOCKET_ERROR
	Server_Class->close_ser = 1;//��־�� 1 ���� server ����
	return closesocket(Server_Class->server.socket);
}

//--------------------------------------------------------------------------------close client
int Close_clientsock(Client* Client_Class) //�ر� client socket
{//�ɹ��رշ��� 0 ,�����򷵻� SOCKET_ERROR
	Client_Class->close_cli = 1;//��־�� 1 ���� client ����
	return closesocket(Client_Class->client.socket);
}


//--------------------------------��ȡserver_socket
SOCKET Get_serversock(Server* Server_Class)
{
	return Server_Class->server.socket;
}

//--------------------------------��ȡclient_socket
SOCKET Get_clientsock(Client* Client_Class)
{
	return Client_Class->client.socket;
}



//-----------����˻�ȡ��һ�εĴ�����Ϣ
int Get_error_server(Server* Server_Class)
{
	return Server_Class->error_sock;
}

//-----------�ͻ��˻�ȡ��һ�εĴ�����Ϣ
int Get_error_client(Client* Client_Class)
{
	return Client_Class->error_sock;
}

