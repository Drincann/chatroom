#include"Gao_socket_Class.h"
#pragma comment(lib,"Gao_socket_Class.lib")
#include<stdio.h>
Client client = { 0 };
void server_leave_callback(int error) {//服务端断开连接
	printf("\nerver_leave:%d", error);
}
void data_coming_callback(SOCKET client_sock, char* data) {//数据到达
	char buf[100] = { 0 };
	printf("\ndata_coming:%s", data);
	printf("\n输入数据：");
	scanf("%s", buf);//输入数据
	printf("\nsend:%d", (*client.method_Send_msg)(client, buf, strlen(buf)));//发送
}
void connect_succeed_callback(SOCKET client_sock) {//connect 成功 回调函数
	printf("\nconnect_succeed");
}
void error_callback(SOCKET client_sock, int error) {//出现异常
	printf("\nerror:%d", error);

}
void main() {
	struct timeval tim = { 1,0 };


	Get_object_client(&client);//置对象方法

	{//置对象成员属性
		client.member_buf_lenth = 1024;
		client.member_timeout = tim;
		client.member_callback_connect_succeed = connect_succeed_callback;
		client.member_callback_data_coming = data_coming_callback;
		client.member_callback_error = error_callback;
		client.member_callback_server_leave = server_leave_callback;
		printf("Ip："); scanf("%s",client.member_ip);//输入 server ip地址
		printf("Port："); scanf("%hu", &client.member_port);//输入 server 监听端口
	}

	if (Startup_sock_api() == -1)return;//startup_sock_api() 必须首先调用(同一进程调用一次即可)

	printf("\nstartup succeed");

	printf("\nreturn:%d", client.method_Create_clientsock(&client));
	printf("\nerror:%d", client.method_Get_error_client(client));

	getchar(); getchar();
}
