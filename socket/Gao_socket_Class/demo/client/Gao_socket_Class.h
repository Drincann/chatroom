#pragma once
#ifndef GAO_LIHAI_sock_api
#define GAO_LIHAI_sock_api

#include <string.h>
#include <winsock2.h>

#define NORMAL_CLOSE 1  //调用者手动正常结束
// server client_leave_callback 回调函数的第四个参数常量
#define CLIENT_NOR_CLOSE -4    //客户正常退出
#define CLIENT_UNNOR_CLOSE -5  //客户非正常退出
// server client_coming_callback 回调函数的返回常量
#define ACCEPT_CLIENT 1   //服务器接受客户
#define REFUSE_CLIENT -1  //服务器忽略客户
// server 函数的返回常量
#define ERROR_SER_MEM_1 -1        //开始时内存分配失败
#define ERROR_SER_SOCK -2         //获取 socket 失败
#define ERROR_SER_BIND -3         // bind port 失败
#define ERROR_SER_LISTEN -4       // listen 失败
#define ERROR_SER_MEM_2 -5        //客户进入时内存分配失败
#define ERROR_SER_NOR_CLOSE -6    //客户正常退出时内存分配失败
#define ERROR_SER_UNNOR_CLOSE -7  //客户非正常退出时内存分配失败

// client 函数的返回常量
#define ERROR_CLI_SOCK -1           //获取 socket 失败
#define ERROR_CLI_CNCT -2           // connect 失败
#define ERROR_CLI_SERVERCLOSE_1 -3  //服务端正常关闭连接
#define ERROR_CLI_SERVERCLOSE_2 -4  //服务端异常退出

// socket + address 结构体
typedef struct socket_state {
  SOCKET socket;
  SOCKADDR_IN socket_addr;
} Client_state, Server_state;

//定义类时中需要自身，提前define
typedef struct Server_ Server;
typedef struct Client_ Client;
//********************************************类定义********************************************

//--------------------服务端 类
struct Server_ {
  // private成员
  Server_state private_1, *private_2;
  char private_3;
  int private_4;

  // public成员
  unsigned short member_port;  //端口
  struct timeval member_timeout;  // select 超时时间，需要提供一个 timeval
  int member_buf_lenth;           //接收缓冲区长度
  int (*member_callback_client_coming)(SOCKET client_sock,
                                       char* ip);  // client 请求连接 回调函数
  void (*member_callback_client_leave)(SOCKET client_sock,
                                       char* ip,
                                       int state);  // client 离开 回调函数
  void (*member_callback_data_coming)(SOCKET client_sock,
                                      char* ip,
                                      char* data);  // client 数据到达 回调函数
  void (*member_callback_error)(SOCKET client_sock,
                                int error);  //异常错误回调函数，供程序员 debug

  //方法
  int (*method_Create_serversock)(Server* Server_Class);  //创建服务端

  int (*method_Send_msg)(Server Server_Class,
                         SOCKET client_sock,
                         char* msg,
                         int lenth);  //发送数据

  int (*method_Close_serversock)(Server Server_Class);  //关闭服务端

  int (*method_Get_error_server)(Server Server_Class);  //获取错误信息

  SOCKET (*method_Get_serversock)(Server Server_Class);
};

//--------------------客户端 类
struct Client_ {
  // private成员
  Client_state private_1, private_2;
  char private_3;
  int private_4;

  // public成员
  unsigned short member_port;  // server 端口
  char member_ip[16];          // server IP 地址
  struct timeval member_timeout;  // select 超时时间，需要提供一个 timeval
  int member_buf_lenth;           //接收缓冲区长度
  void (*member_callback_server_leave)(SOCKET client_sock,
                                       int error);  // server 离开 回调函数
  void (*member_callback_data_coming)(SOCKET client_sock,
                                      char* data);  // server 数据到达 回调函数
  void (*member_callback_connect_succeed)(
      SOCKET client_sock);                   // connect 成功 回调函数
  void (*member_callback_error)(int error);  //异常错误回调函数，供程序员 debug

  //方法
  int (*method_Create_clientsock)(Client* Client_Class);

  int (*method_Send_msg)(Client Client_Class, char* msg, int lenth);

  int (*method_Close_clientsock)(Client Client_Class);

  int (*method_Get_error_client)(Client Client_Class);

  SOCKET (*method_Get_clientsock)(Client Client_Class);
};
//********************************************类定义结束********************************************

//********************************************以下第二个函数必须在开头调用********************************************
//一个进程调用一次即可，多线程不用重复调用

//-----------绑定socket库（初始化socket）
int Startup_sock_api();

//-----------清除绑定
int Cleanup_sock_api();

//*********************************************用于绑定 api
//到程序**************************************************

//*********************************************get对象**************************************************
// get服务端对象
void Get_object_server(Server* Server_Class);

// get客户端对象
void Get_object_client(Client* Client_Class);
//*********************************************get对象**************************************************

#endif
