#pragma once
#ifndef GAO_LIHAI_sock_api
#define GAO_LIHAI_sock_api

#include<winsock2.h>

#include<string.h>

#define NORMAL_CLOSE 1//调用者手动正常结束

//server client_leave_callback 回调函数的第四个参数常量
#define CLIENT_NOR_CLOSE -4//客户正常退出
#define CLIENT_UNNOR_CLOSE -5//客户非正常退出
//server client_coming_callback 回调函数的返回常量
#define ACCEPT_CLIENT 1//服务器接受客户
#define REFUSE_CLIENT -1//服务器忽略客户
//server 函数的返回常量
#define ERROR_SER_MEM_1 -1//开始时内存分配失败
#define ERROR_SER_SOCK -2//获取 socket 失败
#define ERROR_SER_BIND -3//bind port 失败
#define ERROR_SER_LISTEN -4//listen 失败
#define ERROR_SER_MEM_2 -5//客户进入时内存分配失败
#define ERROR_SER_NOR_CLOSE -6//客户正常退出时内存分配失败
#define ERROR_SER_UNNOR_CLOSE -7//客户非正常退出时内存分配失败

//client 函数的返回常量
#define ERROR_CLI_SOCK -1//获取 socket 失败
#define ERROR_CLI_CNCT -2//connect 失败
#define ERROR_CLI_SERVERCLOSE_1 -3//服务端正常关闭连接
#define ERROR_CLI_SERVERCLOSE_2 -4//服务端异常退出
//int error_sock;//错误信息
/*
typedef struct socket_state {
	SOCKET socket;
	SOCKADDR_IN socket_addr;
}Client_state, Server_state;
*/
//----------------------------------------server函数
int create_serversock(
	int port, //server 端口
	struct timeval timv, //select 超时时间，需要提供一个 timeval
	int (*client_coming_callback)(SOCKET client_sock, char* ip),//client 请求连接 回调函数
	void (*client_leave_callback)(SOCKET client_sock, char* ip, int state),//client 离开 回调函数，第四个参数
	void (*data_coming_callback)(SOCKET client_sock, char* ip, char* data),//client 数据到达 回调函数
	void (*error_callback)(SOCKET client_sock, int error)//异常错误回调函数，供程序员 debug
);

//------------------------------------------------------------client函数
int create_clientsock(
	int port, //server 端口
	char* ip, //server IP 地址
	struct timeval timv, //select 超时时间，需要提供一个 timeval
	void (*server_leave_callback)(SOCKET client_sock, int error),//server 离开 回调函数
	void (*data_coming_callback)(SOCKET client_sock, char* data),//server 数据到达 回调函数
	void (*connect_succeed_callback)(SOCKET client_sock),//connect 成功 回调函数
	void (*error_callback)(int error)//异常错误回调函数，供程序员 debug
);

//------------------------------------------------------------------------------------------------------发送数据
int send_msg(SOCKET client_sock, char* msg, int lenth);

int close_serversock(); //关闭 server 及其 accept() 到的 client socket

int close_clientsock();//关闭 client socket

//-----------绑定socket库（初始化socket）
int startup_sock_api();

//-----------清除绑定
int cleanup_sock_api();

//--------------------------------获取server_socket
SOCKET get_server();

//--------------------------------获取client_socket
SOCKET get_client();

//-----------获取上一次的错误信息
int get_error();

#endif
