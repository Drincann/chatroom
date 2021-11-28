#include"Gao_socket.h"
#pragma comment(lib,"Gao_socket.lib")
#include<stdio.h>


int client_coming_callback(SOCKET client_sock, char* ip) {//客户进入
	char buf[100] = {0};
	printf("\nclient_coming:\nip:%s\nsocket:%d", ip,client_sock);//显示客户信息
	sprintf(buf,"IP:%s\n",ip);
	printf("\nsend:%d", send_msg(client_sock, buf, strlen(buf)));//发送其信息
	return ACCEPT_CLIENT;//接受客户
}

void client_leave_callback(SOCKET client_sock, char* ip, int state) {//客户离开
	if (state == CLIENT_NOR_CLOSE)
	{
		printf("\nclient_normal_leave:\nip:%s\nsocket:%d", ip, client_sock);
	}
	else
	{
		printf("\nclient_unnormal_leave:\nip:%s\nsocket:%d", ip, client_sock);
	}

}

void data_coming_callback(SOCKET client_sock, char* ip, char* data) {//数据到达
	printf("\ndata_coming:\nip:%s\ndata:%s\nsocket:%d", ip, data, client_sock);//输出信息
}

void error_callback(SOCKET client_sock, int error) {//出现异常
	printf("\nerror:%d", error);
}

void main() {

	struct timeval timv = { 1,0 };//服务器轮询各客户端超时时间
	unsigned int port;
	printf("Monitored port："); scanf("%d",&port);//输入欲监听的端口
	printf("\nstart:%d", startup_sock_api());//startup_sock_api() 必须首先调用
	printf("\nreturn:%d", create_serversock(port, timv, client_coming_callback, client_leave_callback, data_coming_callback, error_callback));
	getchar(); getchar();
}
