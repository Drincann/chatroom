
// socket 接口
#include "Gao_socket.h"  //winsock2.h 包含了 windows.h
#pragma comment(lib, "Gao_socket.lib")

//字符串处理
#include <stdio.h>

//获取时间
#include <time.h>

//服务端信息
#define PORT 50055  //服务端监听端口

//加入/删除 客户时的返回值
#define SUCCESS 1
#define FAIL -1

//-----------------------------------------------声明-----------------------------------------------

//*************声明变量*******************
SOCKET* client;
int arr_Lowsub = -1;

//*************声明函数*******************
//服务端回调函数
int client_coming_callback(SOCKET client_sock, char* ip);
void client_leave_callback(SOCKET client_sock, char* ip, int state);
void data_coming_callback(SOCKET client_sock, char* ip, char* data);
void error_callback(SOCKET client_sock, int error);

//其他
int sock_add(SOCKET sock_data, SOCKET*(*socket), int* Lowsub);
int sock_del(SOCKET*(*arr), int sub, int* Lowsub);

//-----------------------------------------------主函数-----------------------------------------------
int main() {
  startup_sock_api();
  struct timeval timv = {2, 0};
  // socket 在主线程中调用
  create_serversock(PORT, timv, client_coming_callback, client_leave_callback,
                    data_coming_callback, error_callback);
}

//*************socket四个回调*******************
int client_coming_callback(SOCKET client_sock,
                           char* ip)  // client 请求连接 回调函数
{
  while (sock_add(client_sock, &client, &arr_Lowsub) == FAIL) {
    if (MessageBox(NULL, L"Client内存分配失败！", L"错误",
                   MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_ICONWARNING) !=
        IDRETRY)
      exit(-1);
  }

  //分发在线人数消息给客户端
  char buf[10] = {0};
  sprintf(buf, " %d", arr_Lowsub + 1);  //前缀空格代表发送的数据为在线人数
  for (int i = 0; i <= arr_Lowsub; i++) {
    send_msg(client[i], buf, strlen(buf));
  }
  printf("client 请求连接 回调函数:%s\nsocket:%d\narrsize:%d\n\n", ip,
         client_sock, arr_Lowsub);
  return ACCEPT_CLIENT;
}

void client_leave_callback(SOCKET client_sock,
                           char* ip,
                           int state)  // client 离开 回调函数
{
  int i = 0;
  for (; i <= arr_Lowsub; i++) {
    if (client[i] == client_sock)
      break;
  }
  while (sock_del(&client, i, &arr_Lowsub) == FAIL) {
    if (MessageBox(NULL, L"Client内存分配失败！", L"错误",
                   MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_ICONWARNING) !=
        IDRETRY)
      exit(-1);
  }

  //分发在线人数消息给客户端
  char buf[10] = {0};
  sprintf(buf, " %d", arr_Lowsub + 1);  //前缀空格代表发送的数据为在线人数
  for (int i = 0; i <= arr_Lowsub; i++) {
    send_msg(client[i], buf, strlen(buf));
  }
  printf("client 离开 回调函数:%s\nsocket:%d\nstate:%d\narrsize:%d\n\n", ip,
         client_sock, state, arr_Lowsub);
}

void data_coming_callback(SOCKET client_sock,
                          char* ip,
                          char* data)  // client 数据到达 回调函数
{
  printf("client 数据到达 回调函数:%s\nsocket:%d\ndata:%s\narrsize:%d\n\n", ip,
         client_sock, data, arr_Lowsub);
  //客户端保证我们接收的数据至多不超过1024k
  //拼接消息
  char str[1100] = {
      0};  //由于需要拼接时间文本，故缓冲区稍大一些，防止strcat溢出
  time_t tm = time(0);
  // 1.拼接时间
  strcat(str, ctime(&tm));
  str[strlen(str) - 1] = 0;  //去掉ctime末尾的\0
  strcat(str, "\r\n");

  // 2.拼接客户端数据
  strcat(str, data);
  //服务端返回的最终数据包括
  // 3.拼接换行									//1.时间 + 换行 + （姓名 +
  // 换行
  // + 数据） + 换行 + 换行
  str[1019] = 0;  //客户端负责 客户端数据 =（姓名 + 换行 + 数据）
  strcat(str,
         "\r\n\r\n");  //服务端负责 时间 + 换行 + 拼接客户端数据 + 换行 + 换行
  str[1023] = 0;  //末尾置0，保证发送的数据不大于 1kb

  //分发聊天消息给客户端
  for (int i = 0; i <= arr_Lowsub; i++) {
    send_msg(client[i], str, strlen(str));
  }
}

void error_callback(SOCKET client_sock,
                    int error)  //异常错误回调函数，供程序员 debug
{
  //调试断点时的极少数情况下会出现10038错误
  //接口里没写对10038错误码的处理，此问题的解释是服务端主动关闭某socket后仍试图使用该socket，暂时没解决思路
  printf("异常错误回调函数\nsocket:%d\nerror:%d\narrsize:%d\n\n", client_sock,
         error, arr_Lowsub);
  char buf[100] = {0};
  sprintf(buf, "error:%d\n", error);
  OutputDebugString(buf);
  printf("error:%d\n", error);
}

//*************数组操作函数*******************
//增加成员
int sock_add(SOCKET sock_data, SOCKET*(*socket), int* Lowsub) {
  SOCKET* temp = *socket;
  *socket = (SOCKET*)realloc(*socket, (*Lowsub + 2) * sizeof(unsigned int));
  if (*socket == NULL) {
    *socket = temp;
    return FAIL;  //失败
  } else {
    (*socket)[++(*Lowsub)] = sock_data;
    return SUCCESS;  //成功
  }
}

//删除成员
int sock_del(SOCKET*(*arr),
             int sub,
             int* Lowsub) {  // 1.数组首地址  2.欲删除下标  3.数组下界
  if (*Lowsub == 0)  //仅剩一个成员时其退出，数组下界为 0
  {
    free(*arr);   //直接释放
    *arr = NULL;  //置NULL
    (*Lowsub)--;
    return SUCCESS;  //返回 1,成功
  }

  SOCKET* temp = *arr;                   //临时指针保存原空间地址
  for (int i = sub; i < *Lowsub; i++) {  //复制
    (*arr)[i] = (*arr)[i + 1];
  }

  *arr = (SOCKET*)realloc(*arr, *Lowsub * sizeof(SOCKET));  //截断末尾的一个成员
  if (*arr == NULL) {
    *arr = temp;  //分配失败，保存原指针
    return FAIL;  //返回 -1,失败
  } else {
    (*Lowsub)--;
    return SUCCESS;  //返回 1,成功
  }
}