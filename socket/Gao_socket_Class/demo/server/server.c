#include"Gao_socket_Class.h"
#pragma comment(lib,"Gao_socket_Class.lib")
#include<stdio.h>

Server server = { 0 };
int client_coming_callback(SOCKET client_sock, char* ip) {//客户进入
	char buf[100] = { 0 };
	printf("\nclient_coming:\nip:%s\nsocket:%d", ip, client_sock);//显示客户信息
	sprintf(buf, "IP:%s\n", ip);
	printf("\nsend:%d", (*server.method_Send_msg)(server, client_sock, buf, strlen(buf)));//发送其信息
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
	char buf[100] = { 0 };
	printf("\n输入数据：");
	scanf("%s", buf);//输入数据
	printf("\nsend:%d", (*server.method_Send_msg)(server, client_sock, buf, strlen(buf)));//发送
}

void error_callback(SOCKET client_sock, int error) {//出现异常
	printf("\nerror:%d", error);
}

void main() {

	struct timeval tim = { 1,0 };//服务器轮询各客户端超时时间


	Get_object_server(&server);//置对象方法
	{//置对象成员属性
		server.member_buf_lenth = 1024;
		server.member_timeout = tim;
		server.member_callback_client_coming = client_coming_callback;
		server.member_callback_data_coming = data_coming_callback;
		server.member_callback_error = error_callback;
		server.member_callback_client_leave = client_leave_callback;
		printf("Monitored port："); scanf("%d", &(server.member_port));//输入欲监听的端口
	}
	if (Startup_sock_api() == -1)return;//startup_sock_api() 必须首先调用(同一进程调用一次即可)

	printf("\nstartup succeed");

	printf("\nreturn:%d", (*server.method_Create_serversock)(&server));
	getchar(); getchar();
}
