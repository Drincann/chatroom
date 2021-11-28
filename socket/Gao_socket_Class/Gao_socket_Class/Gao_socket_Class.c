#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")//在VS2008以上的版本中，使用socket时需要链接库: Ws2_32.lib
typedef struct Server_ Server;//定义类时中需要自身，提前define
typedef struct Client_ Client;//定义类时中需要自身，提前define

WSADATA wsadata;

//socket + address 结构体
typedef struct socket_state {
	SOCKET socket;
	SOCKADDR_IN socket_addr;
}Client_state, Server_state;

//所有函数
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

//********************************************类定义********************************************



//--------------------服务端 类
struct Server_ {

	//private成员
	Server_state server, * s_client;//seversock
	char close_ser;//结束函数标记
	int error_sock;//错误信息

	//public成员
	unsigned short port;//端口
	struct timeval timeout;//select 超时时间，需要提供一个 timeval
	int buf_lenth;//接收缓冲区长度
	int (*client_coming_callback)(SOCKET client_sock, char* ip);//client 请求连接 回调函数
	void (*client_leave_callback)(SOCKET client_sock, char* ip, int state);//client 离开 回调函数
	void (*data_coming_callback)(SOCKET client_sock, char* ip, char* data);//client 数据到达 回调函数
	void (*error_callback)(SOCKET client_sock, int error);//异常错误回调函数，供程序员 debug

	//方法
	int (*Create_serversock)(Server* Server_Class);//创建服务端

	int (*Send_msg)(Server* Server_Class, SOCKET client_sock, char* msg, int lenth);//发送数据

	int (*Close_serversock)(Server* Server_Class);//关闭服务端

	int (*Get_error_server)(Server* Server_Class);//获取错误信息

	SOCKET (*Get_serversock)(Server* Server_Class);
};


//--------------------客户端 类
struct Client_ {

	//private成员
	Client_state client, c_server;//clientsock
	char close_cli;//结束函数标记
	int error_sock;//错误信息

	//public成员
	unsigned short port;//server 端口
	char* ip; //server IP 地址
	struct timeval timeout; //select 超时时间，需要提供一个 timeval
	int buf_lenth;//接收缓冲区长度
	void (*server_leave_callback)(SOCKET client_sock, int error);//server 离开 回调函数
	void (*data_coming_callback)(SOCKET client_sock, char* data);//server 数据到达 回调函数
	void (*connect_succeed_callback)(SOCKET client_sock); //connect 成功 回调函数
	void (*error_callback)(int error);//异常错误回调函数，供程序员 debug

	//方法
	int (*Create_clientsock)(Client* Client_Class);

	int (*Send_msg)(Client* Client_Class, char* msg, int lenth);

	int (*Close_clientsock)(Client* Client_Class);

	int (*Get_error_client)(Client* Client_Class);

	SOCKET (*Get_clientsock)(Client* Client_Class);
};
//********************************************类定义结束********************************************








//********************************************以下第二个函数必须在开头调用********************************************
//一个进程调用一次即可，多线程不用重复调用
//-----------清除绑定
int Cleanup_sock_api()
{
	return WSACleanup();
}
//-----------绑定socket库（初始化socket）
int Startup_sock_api()
{
	return WSAStartup(MAKEWORD(2, 2), &wsadata);
}
//*********************************************用于绑定 api 到程序**************************************************








//get服务端对象
void Get_object_server(Server* Server_Class)
{
	//方法
	Server_Class->Create_serversock = Create_serversock;
	Server_Class->Close_serversock = Close_serversock;
	Server_Class->Send_msg = Send_msg_server;
	Server_Class->Get_error_server = Get_error_server;
	Server_Class->Get_serversock = Get_serversock;
}
//get客户端对象
void Get_object_client(Client* Client_Class)
{
	//方法
	Client_Class->Create_clientsock = Create_clientsock;
	Client_Class->Close_clientsock = Close_clientsock;
	Client_Class->Send_msg = Send_msg_client;
	Client_Class->Get_error_client = Get_error_client;
	Client_Class->Get_clientsock = Get_clientsock;
}




//--------------数组删除成员函数定义（private）
void delete_member(struct socket_state* (*arr), int number, int size)
{//1.数组首地址  2.欲删除下标  3.数组下界
	if (size == 0)//仅剩一个 client 时其退出，数组下界为 0 
	{
		free(*arr);//直接释放
		*arr = NULL;//置NULL
		return;//返回
	}
	struct socket_state* temp = *arr;//临时指针保存原空间
	for (int i = number; i < size; i++)
	{//复制
		(*arr)[i] = (*arr)[i + 1];
	}
	*arr = (struct socket_state*) realloc(*arr, size * sizeof(struct socket_state));//截断末尾的一个成员
	if (*arr == NULL)
	{
		free(temp);//分配失败，释放原空间
	}
}

//----------------------------------------server函数定义
int Create_serversock(Server* Server_Class)//异常错误回调函数，供程序员 debug

{
	if (Server_Class->s_client != NULL)
	{//释放 client 内存
		free(Server_Class->s_client); Server_Class->s_client = NULL;
	}

	int arr_size = -1;//client 数组下界
	Client_state sock_temp = { 0 };//临时 socket
	fd_set source_REDfd, temp_REDfd;//主文件描述符集合 与 临时文件描述符集合
	int size_addr = sizeof(struct sockaddr);//用于 accept() 第四个参数
	char *buf = calloc(1, Server_Class->buf_lenth);//数据接收缓冲区
	if (buf == NULL)return -1;
	{//设置 server_addr
		Server_Class->server.socket_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//inet_addr()用于将 十进制点分ip 转换为 网络字节顺序，逆转函数为 inet_ntoa()，
		Server_Class->server.socket_addr.sin_family = AF_INET;//IPV4协议
		Server_Class->server.socket_addr.sin_port = Server_Class->port == 0 ? htons(INADDR_ANY) : htons(Server_Class->port);//htons()（htonl（）长型）将 int 转换为主机字节顺序，逆转函数为 ntohs()（ntohl（）长型），
	}

	//获取一个 socket 描述符
	if (Server_Class->server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), Server_Class->server.socket < 0)
	{
		Server_Class->error_sock = WSAGetLastError();
		return -2;//socket 创建失败
	}

	//绑定端口（对于 server ，我们有必要知道我们的端口是多少，好让 client 知道它要连接到哪里）
	if (bind(Server_Class->server.socket, (struct sockaddr*) & (Server_Class->server.socket_addr), sizeof(struct sockaddr)) < 0)
	{
		Server_Class->error_sock = WSAGetLastError();
		return -3;//bind 端口失败
	}

	//开始监听端口消息
	if (listen(Server_Class->server.socket, 2) < 0)
	{
		Server_Class->error_sock = WSAGetLastError();													//FD_CLR(int fd, fd_set * set) //- 从集合中移去fd
		return -4;//listen 失败															//FD_SET(int fd, fd_set * set)// - 添加fd到集合
	}																					//FD_ISSET(int fd, fd_set * set)// - 测试fd是否在集合中
	FD_ZERO(&temp_REDfd);																//FD_ZERO(fd_set * set) //- 清除一个文件描述符集合
	FD_SET(Server_Class->server.socket, &temp_REDfd);

	FD_ZERO(&source_REDfd);
	FD_SET(Server_Class->server.socket, &source_REDfd);
	//循环消息，select非阻塞
	while (1)
	{
		select(0, &temp_REDfd, NULL, NULL, &(Server_Class->timeout));// windows 下第一个单数仅用于保持兼容性，可以忽略
		if (Server_Class->close_ser)//检测到关闭
		{
			Server_Class->close_ser = 0;//重置
			free(Server_Class->s_client); Server_Class->s_client = NULL;
			free(buf);
			return 1;//外部关闭 socket 后结束, 1 为正常结束
		}

		//1. 首先判断 server 的 listen 
		if (FD_ISSET(Server_Class->server.socket, &temp_REDfd))
		{//client 请求连接
			sock_temp.socket = accept(Server_Class->server.socket, (struct sockaddr*) & (sock_temp.socket_addr), &size_addr);
			//与调用者协议，回调函数返回 -1 认为丢弃此 client socket
			if ((Server_Class->client_coming_callback == NULL ? 1 : (*Server_Class->client_coming_callback)(sock_temp.socket, inet_ntoa(sock_temp.socket_addr.sin_addr))) != -1)//调用回调函数
			{
				//分配新内存
				struct socket_state* temp = Server_Class->s_client;
				Server_Class->s_client = (struct socket_state*)realloc(Server_Class->s_client, (++arr_size + 1) * sizeof(struct socket_state));//ar_size 为数组下界，故自增下界后 + 1 为数组新长度
				if (Server_Class->s_client == NULL)
				{
					free(temp);//分配失败，释放原空间
					return -5;//返回 -5
				}
				Server_Class->s_client[arr_size] = sock_temp;//得到 accept() 的 client 信息
				FD_SET(sock_temp.socket, &source_REDfd);//添加到主描述符集合
			}
			else
			{//丢弃此 socket
				closesocket(sock_temp.socket);//关闭 accept() 的 socket
			}
		}

		//2. 再判断各 client 是否可读 
		for (int i = 0; i <= arr_size; i++)
		{
			if (FD_ISSET(Server_Class->s_client[i].socket, &temp_REDfd))
			{

				memset(buf, 0, 1024);//清空缓冲区
				int temp = recv(Server_Class->s_client[i].socket, buf, 1023, 0);
				//取到数据
				if (temp == 0)//返回 0 ：client 关闭了 socket
				{//解释： 在 select() 得到的文件描述符中存在此 client sock，却未得到数据，说明 client 主动关闭了 socket，断开了连接
					FD_CLR(Server_Class->s_client[i].socket, &source_REDfd);//在集合中去除描述符
					closesocket(Server_Class->s_client[i].socket);//关闭离开 client socket
					if (Server_Class->client_leave_callback != NULL)
					{
						(*Server_Class->client_leave_callback)(Server_Class->s_client[i].socket, inet_ntoa(Server_Class->s_client[i].socket_addr.sin_addr), -4);//调用 client_leave 回调函数
					}
					delete_member(&(Server_Class->s_client), i, arr_size);//删除离开 client 的内存
					arr_size--;//数组下界 - 1 
					if (Server_Class->s_client == NULL && arr_size != -1)//存在数组 0 成员情况,此时 arr_size == -1 ，而新增成员需要调用 realloc(p,size)  (p指向NULL时 相当于 malloc(size))
					{//也就是说，当 s_client 指向 NULL 时不一定是内存分配失败，也可能是数组成员为 0 的等待 client 进入的状态
						//内存分配失败，delete_member()已释放过空间，直接返回即可
						return -6;//正常退出，返回 -6
					}
				}
				else if (WSAGetLastError() == 10054) //错误信息返回 10054 ：client 异常退出，此时 recv 返回 -1
				{//解释： 10054 表示 client 异常断开，一般是未关闭 socket 时 client 异常关闭
					FD_CLR(Server_Class->s_client[i].socket, &source_REDfd);
					closesocket(Server_Class->s_client[i].socket);//关闭离开 client socket
					if (Server_Class->client_leave_callback != NULL)
					{
						(*Server_Class->client_leave_callback)(Server_Class->s_client[i].socket, inet_ntoa(Server_Class->s_client[i].socket_addr.sin_addr), -5);//调用 client_leave 回调函数
					}
					delete_member(&(Server_Class->s_client), i, arr_size);//删除离开 client 的内存
					arr_size--;//数组下界 -1 
					if (Server_Class->s_client == NULL && arr_size != -1)
					{//内存分配失败，delete_member()已释放过空间，直接返回即可
						return -7;//异常退出，返回 -7
					}
				}
				else if (WSAGetLastError() == 10053) //错误信息返回 10053 ：client 异常退出，此时 recv 返回 -1
				{//解释： 10053 表示服务端中止了已建立的连接，可能是由于数据传输超时或协议错误造成的。
					FD_CLR(Server_Class->s_client[i].socket, &source_REDfd);
					closesocket(Server_Class->s_client[i].socket);//关闭离开 client socket
					if (Server_Class->client_leave_callback != NULL)
					{
						(*Server_Class->client_leave_callback)(Server_Class->s_client[i].socket, inet_ntoa(Server_Class->s_client[i].socket_addr.sin_addr), -5);//调用 client_leave 回调函数
					}
					delete_member(&(Server_Class->s_client), i, arr_size);//删除离开 client 的内存
					arr_size--;//数组下界 -1 
					if (Server_Class->s_client == NULL && arr_size != -1)
					{//内存分配失败，delete_member()已释放过空间，直接返回即可
						return -7;//异常退出，返回 -7
					}
				}
				else if (temp < 0)//其他异常，原因未知
				{
					Server_Class->error_sock = WSAGetLastError();
					if (Server_Class->error_callback != NULL)
					{
						(*Server_Class->error_callback)(Server_Class->s_client[i].socket, Server_Class->error_sock);//其他异常，调用异常回调函数，供程序员 debug
					}
				}
				else
				{
					if (Server_Class->data_coming_callback != NULL)
					{
						(*Server_Class->data_coming_callback)(Server_Class->s_client[i].socket, inet_ntoa(Server_Class->s_client[i].socket_addr.sin_addr), buf);//调用 data_coming 回调函数
					}
				}
				//-1 为出错，0 为正常关闭，>0 表示正常取到数据,      注意，出错并不意味着连接断开
				//为什么不判断 recv() < 0 的情况，转而判断错误信息的返回值，
				//因为recv()返回 -1 不一定是 client 断开连接，可能是其他原因，而错误信息 10054 代表 client 一定异常退出
			}
		}
		temp_REDfd = source_REDfd;
	}
}

//------------------------------------------------------------client函数定义
int Create_clientsock(Client* Client_Class)
{
	fd_set source_REDfd, temp_REDfd;
	char* buf = calloc(1, Client_Class->buf_lenth);//数据接收缓冲区
	if (buf == NULL)return -5;
	{//设置目标服务端 addr
		Client_Class->c_server.socket_addr.sin_addr.S_un.S_addr = inet_addr(Client_Class->ip);
		Client_Class->c_server.socket_addr.sin_family = AF_INET;
		Client_Class->c_server.socket_addr.sin_port = htons(Client_Class->port);
	}
	if (Client_Class->client.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), Client_Class->client.socket < 0)
	{
		Client_Class->error_sock = WSAGetLastError();
		return -1;//socket 失败
	}
	if (connect(Client_Class->client.socket, (struct sockaddr*) &(Client_Class->c_server.socket_addr), sizeof(struct sockaddr)) < 0)
	{
		Client_Class->error_sock = WSAGetLastError();
		return -2;//connect 失败
	}
	(*Client_Class->connect_succeed_callback)(Client_Class->client.socket);//调用 connect 成功回调函数

	FD_ZERO(&temp_REDfd);
	FD_SET(Client_Class->client.socket, &temp_REDfd);

	FD_ZERO(&source_REDfd);
	FD_SET(Client_Class->client.socket, &source_REDfd);

	//开始循环消息
	while (1)
	{
		select(0, &temp_REDfd, NULL, NULL, &(Client_Class->timeout));
		if (Client_Class->close_cli)
		{
			Client_Class->close_cli = 0;
			free(buf);
			return 1;//正常结束
		}
		if (FD_ISSET(Client_Class->client.socket, &temp_REDfd))//可读
		{
			memset(buf, 0, 1024);//清空缓冲区
			int temp = recv(Client_Class->client.socket, buf, 1023, 0);
			Client_Class->error_sock = WSAGetLastError();
			if (temp == 0)//-1 为出错，0 为正常关闭，>0 表示正常取到数据,      注意，出错并不意味着连接断开
			{//例：服务端关闭 client socket ，返回 0，        服务端非正常关闭，返回 -1，        取到正常数据 返回缓冲区字符串长度
				closesocket(Client_Class->client.socket);//关闭 socket，准备返回
				if (Client_Class->server_leave_callback != NULL)
				{
					(*Client_Class->server_leave_callback)(Client_Class->client.socket, -3);//调用回调函数，程序员处理事件
				}
				return -3;//服务端正常关闭连接，返回 -3
			}
			else if (WSAGetLastError() == 10054)//10054错误，server 异常退出
			{
				closesocket(Client_Class->client.socket);//关闭 socket，准备返回
				if (Client_Class->server_leave_callback != NULL)
				{
					(*Client_Class->server_leave_callback)(Client_Class->client.socket, -4);//调用回调函数，程序员处理事件
				}
				return -4;//服务端异常退出，返回 -4
			}
			else if (temp < 0)//异常，原因未知
			{
				Client_Class->error_sock = WSAGetLastError();
				if (Client_Class->error_callback != NULL)
				{
					(*Client_Class->error_callback)(Client_Class->error_sock);//未知原因，调用回调函数供程序员 debug
				}

			}
			else
			{
				if (Client_Class->data_coming_callback != NULL)
				{
					(*Client_Class->data_coming_callback)(Client_Class->client.socket, buf);//正常接收数据，调用回调函数
				}
			}

		}
		temp_REDfd = source_REDfd;
	}
}

//------------------------------------------------------------------------------------------------------server发送数据
int Send_msg_server(Server* Server_Class, SOCKET client_sock, char* msg, int lenth)//服务端填接收端socket，客户端填发送端socket，因为服务端可能建立了多个连接，而客户端仅建立了一个连接
{//保证缓冲区有结尾 \0 ,或提供精准的lenth
	if (send(client_sock, msg, lenth, 0) < 0)//出错
	{
		Server_Class->error_sock = WSAGetLastError();
		return -1;
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------client发送数据
int Send_msg_client(Client* Client_Class, char* msg, int lenth)//服务端填接收端socket，客户端填发送端socket，因为服务端可能建立了多个连接，而客户端仅建立了一个连接
{//保证缓冲区有结尾 \0 ,或提供精准的lenth
	if (send(Client_Class->client.socket, msg, lenth, 0) < 0)//出错
	{
		Client_Class->error_sock = WSAGetLastError();
		return -1;
	}
	return 0;
}

//--------------------------------------------------------------------------------close server
int Close_serversock(Server* Server_Class) //关闭 server 及其 accept() 到的 client socket
{//成功关闭返回 0 ,错误则返回 SOCKET_ERROR
	Server_Class->close_ser = 1;//标志置 1 ，令 server 结束
	return closesocket(Server_Class->server.socket);
}

//--------------------------------------------------------------------------------close client
int Close_clientsock(Client* Client_Class) //关闭 client socket
{//成功关闭返回 0 ,错误则返回 SOCKET_ERROR
	Client_Class->close_cli = 1;//标志置 1 ，令 client 结束
	return closesocket(Client_Class->client.socket);
}


//--------------------------------获取server_socket
SOCKET Get_serversock(Server* Server_Class)
{
	return Server_Class->server.socket;
}

//--------------------------------获取client_socket
SOCKET Get_clientsock(Client* Client_Class)
{
	return Client_Class->client.socket;
}



//-----------服务端获取上一次的错误信息
int Get_error_server(Server* Server_Class)
{
	return Server_Class->error_sock;
}

//-----------客户端获取上一次的错误信息
int Get_error_client(Client* Client_Class)
{
	return Client_Class->error_sock;
}

