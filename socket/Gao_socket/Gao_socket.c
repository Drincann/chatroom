#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")//在VS2008以上的版本中，使用socket时需要链接库: Ws2_32.lib
//socket + address 结构体
typedef struct socket_state {
	SOCKET socket;
	SOCKADDR_IN socket_addr;
}Client_state, Server_state;


//--------------------全局变量
Client_state client = { 0 }, c_server = { 0 };//clientsock
Server_state server = { 0 }, * s_client = NULL;//seversock
WSADATA wsadata;
char close_ser = 0, close_cli = 0;//结束函数标记
int error_sock = 0;//错误信息

//********************************************以下第二个函数必须在开头调用********************************************
//-----------清除绑定
int cleanup_sock_api()
{
	return WSACleanup();
}
//-----------绑定socket库（初始化socket）
int startup_sock_api()
{
	return WSAStartup(MAKEWORD(2, 2), &wsadata);
}
//*********************************************用于绑定 api 到程序**************************************************

//--------------数组删除成员函数定义（私有）
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
int create_serversock(
	int port, //server 端口，填 0 为任意端口
	struct timeval timv, //select 超时时间，需要提供一个 timeval
	int (*client_coming_callback)(SOCKET client_sock, char* ip),//client 请求连接 回调函数
	void (*client_leave_callback)(SOCKET client_sock, char* ip,int state),//client 离开 回调函数
	void (*data_coming_callback)(SOCKET client_sock, char* ip, char* data),//client 数据到达 回调函数
	void (*error_callback)(SOCKET client_sock, int error)//异常错误回调函数，供程序员 debug
)
{
	server.socket = 0;
	free(s_client); s_client = NULL;//释放 client 内存
	int arr_size = -1;//client 数组下界
	Client_state sock_temp = { 0 };//临时 socket
	fd_set source_REDfd, temp_REDfd;//主文件描述符集合 与 临时文件描述符集合
	int size_addr = sizeof(struct sockaddr);//用于 accept() 第四个参数
	char buf[1024] = { 0 };//数据接收缓冲区

	{//设置 server_addr
		server.socket_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//inet_addr()用于将 十进制点分ip 转换为 网络字节顺序，逆转函数为 inet_ntoa()，
		server.socket_addr.sin_family = AF_INET;//IPV4协议
		server.socket_addr.sin_port = port == 0 ? htons(INADDR_ANY) : htons(port);//htons()（htonl（）长型）将 int 转换为主机字节顺序，逆转函数为 ntohs()（ntohl（）长型），
	}

	//获取一个 socket 描述符
	if (server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), server.socket < 0)
	{
		free(s_client);
		error_sock = WSAGetLastError();
		return -2;//socket 创建失败
	}

	//绑定端口（对于 server ，我们有必要知道我们的端口是多少，好让 client 知道它要连接到哪里）
	if (bind(server.socket, (struct sockaddr*) & (server.socket_addr), sizeof(struct sockaddr)) < 0)
	{
		free(s_client);
		error_sock = WSAGetLastError();
		return -3;//bind 端口失败
	}

	//开始监听端口消息
	if (listen(server.socket, 2) < 0)
	{
		free(s_client);
		error_sock = WSAGetLastError();													//FD_CLR(int fd, fd_set * set) //- 从集合中移去fd
		return -4;//listen 失败															//FD_SET(int fd, fd_set * set)// - 添加fd到集合
	}																					//FD_ISSET(int fd, fd_set * set)// - 测试fd是否在集合中
	FD_ZERO(&temp_REDfd);																//FD_ZERO(fd_set * set) //- 清除一个文件描述符集合
	FD_SET(server.socket, &temp_REDfd);

	FD_ZERO(&source_REDfd);
	FD_SET(server.socket, &source_REDfd);
	//循环消息，select非阻塞
	while (1)
	{
		select(0, &temp_REDfd, NULL, NULL, &timv);// windows 下第一个单数仅用于保持兼容性，可以忽略
		if (close_ser)
		{
			close_ser = 0;//重置
			return 1;//外部关闭 socket 后结束, 1 为正常结束
		}

		//1. 首先判断 server 的 listen 
		if (FD_ISSET(server.socket, &temp_REDfd))
		{//client 请求连接
			sock_temp.socket = accept(server.socket, (struct sockaddr*) & (sock_temp.socket_addr), &size_addr);
			//与调用者协议，回调函数返回 -1 认为丢弃此 client socket
			if (client_coming_callback(sock_temp.socket, inet_ntoa(sock_temp.socket_addr.sin_addr)) != -1)//调用回调函数
			{
				//分配新内存
				struct socket_state* temp = s_client;
				s_client = (struct socket_state*)realloc(s_client, (++arr_size + 1) * sizeof(struct socket_state));//ar_size 为数组下界，故自增下界后 + 1 为数组新长度
				if (s_client == NULL)
				{
					free(temp);//分配失败，释放原空间
					return -5;//返回 -5
				}
				s_client[arr_size] = sock_temp;//得到 accept() 的 client 信息
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
			if (FD_ISSET(s_client[i].socket, &temp_REDfd))
			{

				memset(buf, 0, 1024);//清空缓冲区
				int temp = recv(s_client[i].socket, buf, 1023, 0);
				//取到数据
				if (temp == 0 )//返回 0 ：client 关闭了 socket
				{//解释： 在 select() 得到的文件描述符中存在此 client sock，却未得到数据，说明 client 主动关闭了 socket，断开了连接
					FD_CLR(s_client[i].socket, &source_REDfd);//在集合中去除描述符
					closesocket(s_client[i].socket);//关闭离开 client socket
					client_leave_callback(s_client[i].socket, inet_ntoa(s_client[i].socket_addr.sin_addr),-4);//调用 client_leave 回调函数
					delete_member(&s_client, i, arr_size);//删除离开 client 的内存
					arr_size--;//数组下界 - 1 
					if (s_client == NULL && arr_size != -1)//存在数组 0 成员情况,此时 arr_size == -1 ，而新增成员需要调用 realloc(p,size)  (p指向NULL时 相当于 malloc(size))
					{//也就是说，当 s_client 指向 NULL 时不一定是内存分配失败，也可能是数组成员为 0 的等待 client 进入的状态
						//内存分配失败，delete_member()已释放过空间，直接返回即可
						return -6;//正常退出，返回 -6
					}
				}
				else if (WSAGetLastError() == 10054) //错误信息返回 10054 ：client 异常退出，此时 recv 返回 -1
				{//解释： 10054 表示 client 异常断开，一般是未关闭 socket 时 client 异常关闭
					FD_CLR(s_client[i].socket, &source_REDfd);
					closesocket(s_client[i].socket);//关闭离开 client socket
					client_leave_callback(s_client[i].socket, inet_ntoa(s_client[i].socket_addr.sin_addr),-5);//调用 client_leave 回调函数
					delete_member(&s_client, i, arr_size);//删除离开 client 的内存
					arr_size--;//数组下界 -1 
					if (s_client == NULL && arr_size != -1)
					{//内存分配失败，delete_member()已释放过空间，直接返回即可
						return -7;//异常退出，返回 -7
					}
				}
				else if (WSAGetLastError() == 10053) //错误信息返回 10053
				{//解释： 10053 表示服务端中止了已建立的连接，可能是由于数据传输超时或协议错误造成的。
					FD_CLR(s_client[i].socket, &source_REDfd);
					closesocket(s_client[i].socket);//关闭离开 client socket
					client_leave_callback(s_client[i].socket, inet_ntoa(s_client[i].socket_addr.sin_addr), -5);//调用 client_leave 回调函数
					delete_member(&s_client, i, arr_size);//删除离开 client 的内存
					arr_size--;//数组下界 -1 
					if (s_client == NULL && arr_size != -1)
					{//内存分配失败，delete_member()已释放过空间，直接返回即可
						return -7;//异常退出，返回 -7
					}
				}
				else if (temp < 0)//其他异常，原因未知
				{
					error_sock = WSAGetLastError();
					error_callback(s_client[i].socket, error_sock);//其他异常，调用异常回调函数，供程序员 debug
				}
				else
				{
					data_coming_callback(s_client[i].socket, inet_ntoa(s_client[i].socket_addr.sin_addr), buf);//调用 data_coming 回调函数
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
int create_clientsock(
	int port, //server 端口
	char* ip, //server IP 地址
	struct timeval timv, //select 超时时间，需要提供一个 timeval
	void (*server_leave_callback)(SOCKET client_sock,int error),//server 离开 回调函数
	void (*data_coming_callback)(SOCKET client_sock, char* data),//server 数据到达 回调函数
	void (*connect_succeed_callback)(SOCKET client_sock),//connect 成功 回调函数
	void (*error_callback)(int error)//异常错误回调函数，供程序员 debug
)
{
	client.socket = 0;
	fd_set source_REDfd, temp_REDfd;
	char buf[1024] = { 0 };
	{//设置目标服务端 addr
		c_server.socket_addr.sin_addr.S_un.S_addr = inet_addr(ip);
		c_server.socket_addr.sin_family = AF_INET;
		c_server.socket_addr.sin_port = htons(port);
	}
	if (client.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), client.socket < 0)
	{
		error_sock = WSAGetLastError();
		return -1;//socket 失败
	}
	if (connect(client.socket, (struct sockaddr*) & (c_server.socket_addr), sizeof(struct sockaddr)) < 0)
	{
		error_sock = WSAGetLastError();
		return -2;//connect 失败
	}
	connect_succeed_callback(client.socket);//调用 connect 成功回调函数

	FD_ZERO(&temp_REDfd);
	FD_SET(client.socket, &temp_REDfd);

	FD_ZERO(&source_REDfd);
	FD_SET(client.socket, &source_REDfd);

	//开始循环消息
	while (1)
	{
		select(0, &temp_REDfd, NULL, NULL, &timv);
		if (close_cli)
		{
			close_cli = 0;
			return 1;//正常结束
		}
		if (FD_ISSET(client.socket, &temp_REDfd))//可读
		{
			memset(buf, 0, 1024);//清空缓冲区
			int temp = recv(client.socket, buf, 1023, 0);
			error_sock = WSAGetLastError();
			if (temp == 0)//-1 为出错，0 为正常关闭，>0 表示正常取到数据,      注意，出错并不意味着连接断开
			{//例：服务端关闭 client socket ，返回 0，        服务端非正常关闭，返回 -1，        取到正常数据 返回缓冲区字符串长度
				closesocket(client.socket);//关闭 socket，准备返回
				server_leave_callback(client.socket,-3);//调用回调函数，程序员处理事件
				return -3;//服务端正常关闭连接，返回 -3
			}
			else if (WSAGetLastError() == 10054)//10054错误，server 异常退出
			{
				closesocket(client.socket);//关闭 socket，准备返回
				server_leave_callback(client.socket, -4);//调用回调函数，程序员处理事件
				return -4;//服务端异常退出，返回 -4
			}
			else if (temp < 0)//异常，原因未知
			{
				error_sock = WSAGetLastError();
				error_callback(error_sock);//未知原因，调用回调函数供程序员 debug
			}
			else
			{
				data_coming_callback(client.socket, buf);//正常接收数据，调用回调函数

			}

		}
		temp_REDfd = source_REDfd;
	}
}

//------------------------------------------------------------------------------------------------------发送数据
int send_msg(SOCKET client_sock, char* msg, int lenth)
{//保证缓冲区有结尾 \0 ,或提供精准的lenth
	if (send(client_sock, msg, lenth, 0) < 0)//出错
	{
		error_sock = WSAGetLastError();
		return -1;
	}
	return 0;

}

//--------------------------------------------------------------------------------close server
int close_serversock() //关闭 server 及其 accept() 到的 client socket
{//成功关闭返回 0 ,错误则返回 SOCKET_ERROR
	close_ser = 1;//标志置 1 ，令 server 结束
	free(s_client);//释放空间
	return closesocket(server.socket);
}

//--------------------------------------------------------------------------------close client
int close_clientsock() //关闭 client socket
{//成功关闭返回 0 ,错误则返回 SOCKET_ERROR
	close_cli = 1;//标志置 1 ，令 client 结束
	return closesocket(client.socket);
}

//--------------------------------获取client_socket
SOCKET get_client()
{
	return client.socket;
}
//--------------------------------获取server_socket
SOCKET get_server()
{
	return server.socket;
}
//-----------获取上一次的错误信息
int get_error()
{
	return error_sock;
}



