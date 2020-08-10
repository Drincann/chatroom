#include"Gao_socket.h"
#pragma comment(lib,"Gao_socket.lib")
#include<stdio.h>
void server_leave_callback(int error) {//服务端断开连接
	printf("\nerver_leave:%d", error);
}
void data_coming_callback(SOCKET client_sock, char* data) {//数据到达
	char buf[100] = { 0 };
	printf("\ndata_coming:%s", data);
	scanf("%s", buf);//输入数据
	printf("\nsend:%d", send_msg(get_client(), buf, strlen(buf)));//发送
}
void connect_succeed_callback(SOCKET client_sock) {//connect 成功 回调函数
	printf("\nconnect_succeed");
}
void error_callback(SOCKET client_sock, int error) {//出现异常
	printf("\nerror:%d", error);

}
void main() {
	struct timeval tim = { 1,0 };
	char ip[20];
	int port;
	printf("Ip："); gets(&ip);//输入 server ip地址
	printf("Port："); scanf("%d", &port);//输入 server 监听端口
	printf("\nstart:%d", startup_sock_api());//startup_sock_api() 必须首先调用
	printf("\nreturn:%d", create_clientsock(port, &ip, tim, server_leave_callback, data_coming_callback, connect_succeed_callback, error_callback));
	getchar(); getchar();
}
