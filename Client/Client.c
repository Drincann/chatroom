
//socket 接口
#include"Gao_socket_Class.h"//winsock2.h 包含了 windows.h
#pragma comment(lib,"Gao_socket_Class.lib")

//线程
#include <process.h>

//字符串处理
#include <stdio.h>

//控件风格
#define EDIT_RECV (WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY)//创建 recv 文本框的 style
#define EDIT_SEND (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL)//创建 send 文本框的 style
#define BUTTON (WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON)//创建按钮的 style

//遗留：由于宽高改变时文本框内容出现异常（/r/n的问题），曾改动过窗口边框大小不可调，现在问题解决，这里的窗口风格暂时遗留，供以后重现
//主窗口风格，目前风格使用 WS_OVERLAPPEDWINDOW ，这是组合风格，包含WS_OVERLAPPED, WS_CAPTION, WS_SYSMENU, WS_THICKFRAME, WS_MINIMIZEBOX, WS_MAXIMIZEBOX
//#define WINDOWS (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

//服务器信息
#define PORT 50055//服务端监听端口
#define IP "47.94.7.74"//服务器IP
//#define IP "127.0.0.1"//本地IP——Debug用

//缓冲区长度
//我在静态库中写死了接收缓冲区为 1kb，虽然可以分多次取得数据，但是我希望把每次取到的数据作为完整的一条来处理。
//客户端保证发送的数据不大于 1kb
#define BUFSIZE 1024


//-----------------------------------------------声明-----------------------------------------------

//*************声明变量*******************
//由于 edit 的消息需要在 translatemsg 被调用前进行拦截，故将所有句柄统一声明为全局变量
HWND parent_hWnd, edit_send_hWnd, edit_recv_hWnd, button_hWnd;//主窗口句柄，子窗口（控件）句柄
HINSTANCE hinst;//实例（进程）句柄
MSG msg;//窗口消息结构体

HFONT font;//字体（创建控件时作为中间变量使用，并作为后期 textout 的字体对象）
LOGFONT LogFont;//字体

SOCKET Client_Socket = 0;//客户端 socket 描述符
Client client = { 0 };//Client 对象，未初始化

int sock_state = 0;//socket 创建状态

char UserName[100] = "*    ";//用户名
unsigned long UserName_size = 0;//用户名长度

unsigned int member = 0;//在线人数

//*************声明函数*******************
//windows相关
LRESULT CALLBACK wnd_proc_callback(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);//windows窗口消息的回调函数
void Creat_ChildWindow(HWND hWnd);//创建窗口控件
void SetFont();//设置字体

//客户端回调函数
void server_leave_callback(SOCKET client_sock, int error);//服务端离开回调
void data_coming_callback(SOCKET client_sock, char* data);//数据到达回调
void connect_succeed_callback(SOCKET client_sock);//connect 成功回调
void error_callback(int error);//异常 debug 回调

//其他
void AddEditStr(HWND hWnd, char* str);//文本框 加入文本
void SendData(char* str);//向服务端发送 send文本框数据
void createsock(void*);//socket线程
void connect_server(HWND hWnd);//服务器重连




//-----------------------------------------------主函数-----------------------------------------------

//*************WINAPI入口函数*******************
int	WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR szCmdLine, _In_  int nCmdShow)//入口
{
	{//制作消息前缀（username：）
		DWORD size = 100;
		GetUserName(UserName + 5, &size);//这里获取用户名
		strcat(UserName, "：");//加上冒号
		UserName_size = strlen(UserName);
	}


	WNDCLASS windows = { 0 }; hinst = hInstance;
	//设置窗口类信息
	{
		windows.cbClsExtra = 0; windows.cbWndExtra = 0;
		windows.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);//背景
		windows.hCursor = LoadCursor(NULL, IDC_ARROW);//鼠标
		windows.hIcon = LoadIcon(NULL, IDI_APPLICATION);//图标
		windows.hInstance = hInstance;//实例句柄
		windows.lpfnWndProc = wnd_proc_callback;//回调函数
		windows.lpszClassName = "Chat";//类名
		windows.lpszMenuName = NULL;//菜单名
		windows.style = CS_VREDRAW | CS_HREDRAW;//窗口风格，改变宽和高时重绘
	}
	//注册窗口类
	if (!RegisterClass(&windows))exit(-1);


	//创建窗口
	parent_hWnd = //窗口句柄
	CreateWindow
	(
		"Chat",//与结构体中类名相同
		"Chat Room(大学一年级 第一学期 C语言课程设计)",//窗口标题
		WS_OVERLAPPEDWINDOW,//窗口样式：组合 WS_OVERLAPPED, WS_CAPTION, WS_SYSMENU, WS_THICKFRAME, WS_MINIMIZEBOX, WS_MAXIMIZEBOX
		(GetSystemMetrics(SM_CXSCREEN) - 800) / 2, //X 坐标
		(GetSystemMetrics(SM_CYSCREEN) - 600) / 2, //Y 坐标
		800, //宽
		600, //高
		NULL,NULL,
		hInstance,//实例句柄（进程）
		NULL
	);


	ShowWindow(parent_hWnd,nCmdShow);//显示窗口
	UpdateWindow(parent_hWnd);//更新窗口（可以不写，用于绕过消息队列，直接调用 windows 回调函数处理 WM_PAINT 消息，但前提是窗口必须存在无效区域，否则忽略。）

	//开始从消息队列中获取消息
	while (GetMessage(&msg, NULL, 0, 0))//第二个参数填 NULL 即接收该进程所有窗口信息
	{
		//拦截 发送文本框 和 按钮的 消息
		if (msg.hwnd == edit_send_hWnd && msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN)//判断消息来自 send文本框
		{
			//发送数据给服务端（这里保证发送的数据至多不超过1024k，防止回调函数重复调用）
			char buf[BUFSIZE] = { 0 };//sock里写死了缓冲区为1024B，所以这里也写1024B
			strcat(buf, UserName);
			GetWindowText(edit_send_hWnd,buf + UserName_size, BUFSIZE - UserName_size);//这个函数设计非常周到，偶然发现，当我为了防止末尾不存在0而少写了一个字节时，他实际上少写了两个字节，也就是说，他会将字符串末尾的0也算入我要写入的长度中

			if (buf[UserName_size] != 0)//用户输入非空
			{
				SetWindowText(edit_send_hWnd, NULL);
				SendData(buf);                               //服务端返回的最终数据包括
			}												 //1.时间 + 换行 + （姓名 + 换行 + 数据） + 换行 + 换行
			else											 //客户端负责 客户端数据 =（姓名 + 换行 + 数据）
			{												 //服务端负责 时间 + 换行 + 拼接客户端数据 + 换行 + 换行
				MessageBeep(0xFFFFFFFF);
			}
			continue;//跳过 TranslateMessage(&msg) ，阻止该键盘消息的传递
		}
		else if (msg.hwnd == button_hWnd && msg.message == WM_LBUTTONUP)//判断消息来自 按钮
		{
			//发送数据给服务端,这里不 continue，让窗口缺省处理消息，否则控件卡死
			char buf[BUFSIZE] = { 0 };//sock里写死了缓冲区为1024B，所以这里也写1024B
			strcat(buf, UserName);
			GetWindowText(edit_send_hWnd, buf + UserName_size, BUFSIZE - UserName_size);

			if (buf[UserName_size] != 0)//用户输入非空
			{
				SetFocus(edit_send_hWnd);
				SetWindowText(edit_send_hWnd, NULL);
				SendData(buf);
			}
			else
			{
				SetFocus(edit_send_hWnd);
				MessageBeep(0xFFFFFFFF);
			}
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);		
		//其他消息调用回调函数（分发消息，根据消息种类向回调函数中传入对应参数）
	}
}








//-----------------------------------------------windows系函数-----------------------------------------------

//*************windows回调*******************
LRESULT CALLBACK wnd_proc_callback(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{//回调函数
	static int Activate_flag = 1;//标记首次激活状态
	static int Roll_D = 0;//滚动条底部距最大位置的差值
	switch (msg)//这里处理我们关心的消息
	{
	case WM_CREATE://窗口创建
	{
		Creat_ChildWindow(hWnd);//创建子窗口（控件）
		SetFont();//设置字体
		SetFocus(edit_send_hWnd);
		break;
	}
	case WM_ACTIVATE://激活或失去激活
	{
		if (Activate_flag == 1)
		{//这里首次激活,但窗口尚未显示，等待下一次 WM_SIZE 到来时才显示
			Activate_flag = 0;
		}
		SetFocus(edit_send_hWnd);//窗口激活时发送文本框获取焦点
		return 0;//这里如果 break 则会调用默认消息处理函数，其对 WM_ACTIVATE 的缺省处理是使主窗口获取焦点，故此处返回即可。
	}
	case WM_PAINT://窗口重绘
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		char buf[50] = { 0 };

		SelectObject(hdc, font);//选择字体对象
		SetBkMode(hdc, TRANSPARENT);//字体背景透明

		sprintf(buf, "当前在线人数：%d", member);
		TextOut(hdc, 10, 10, buf, lstrlen(buf));
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_SIZING://窗口大小被改变前收到的消息，在这里获得滚动条初始位置
	{
		int min, max;
		SCROLLBARINFO roll_info = { 0 }; roll_info.cbSize = sizeof(SCROLLBARINFO);

		GetScrollRange(edit_recv_hWnd, SB_VERT, &min, &max);//获取滚动条最大值
		GetScrollBarInfo(edit_recv_hWnd, OBJID_VSCROLL, &roll_info);//获取 SCROLLBARINFO 结构体，其中包含滚动条高度
		Roll_D = max - GetScrollPos(edit_recv_hWnd, SB_VERT) - roll_info.dxyLineButton;
		break;
	}
	case WM_SIZE://窗口大小被改变
	{
		//窗口可调的遗留代码12

		//由于控件移动时控件内容会发生变化，暂时无法解决，所以改用了固定边框，并注释掉了 movewindow 部分代码。
		RECT rect;
		GetClientRect(hWnd, &rect);//获取客户区
		//调整位置
		MoveWindow(edit_recv_hWnd,
			10,//左上角 X 坐标
			40,//左上角 Y 坐标
			rect.right - 10 - 10,//宽
			rect.bottom - 10 - 50 - 30,//高 
			FALSE
		);
		MoveWindow(edit_send_hWnd,
			10,//左上角 X 坐标
			rect.bottom - 50 + 10,//左上角 Y 坐标
			rect.right - 10 - 10 - 100,//宽
			35,//高 
			FALSE
		);
		MoveWindow(button_hWnd,
			10 + rect.right - 10 - 10 - 100 + 10,//左上角 X 坐标
			rect.bottom - 50 + 10,//左上角 Y 坐标
			90,//宽
			35,//高 
			TRUE
		);

		int min, max;
		SCROLLBARINFO roll_info = { 0 }; roll_info.cbSize = sizeof(SCROLLBARINFO);

		GetScrollRange(edit_recv_hWnd, SB_VERT, &min, &max);//获取滚动条最大值
		GetScrollBarInfo(edit_recv_hWnd, OBJID_VSCROLL, &roll_info);//获取 SCROLLBARINFO 结构体，其中包含滚动条高度
		//向recv文本框发送 WM_VSCROLL 消息，其传入回调函数中的参数为 滚动条拖动终止位置 和 SB_THUMBPOSITION，分别传于 wParam 的高字节和低字节位。
		//MAKELONG宏 用于制作一个32位的数据（相应的还有MAKEWORD宏用于制作一个16位数据）
		SendMessage(edit_recv_hWnd,WM_VSCROLL, MAKELONG(SB_THUMBPOSITION, max - roll_info.dxyLineButton - Roll_D), 0L);
		

		//重画textout
		//1.某些情况下，第一次会重画失败
		//2.在WM_PAINT消息下重画会导致窗口首次激活后第一次重画失败
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		char buf[50] = { 0 };

		SelectObject(hdc, font);//选择字体对象
		SetBkMode(hdc, TRANSPARENT);//字体背景透明

		sprintf(buf, "当前在线人数：%d", member);
		TextOut(hdc, 10, 10, buf, lstrlen(buf));
		EndPaint(hWnd, &ps);
		

		//首次显示窗口
		if (Activate_flag == 0)
		{//首次激活后显示窗口，此时会 get 到 WM_SIZE 消息，此时是窗口第一次显示。
			Activate_flag = -1;//首次激活事件不再触发
			//创建 socket 线程，开始接收消息
			connect_server(parent_hWnd);//参数是遗留问题，一开始在尚未创建窗口时连接服务端，此时句柄还未返回，传入NULL使messagebox的父窗口句柄为NULL
			break;
		}
		break;
	}
	case WM_CLOSE://窗口被关闭
	{//窗口关闭和即将销毁的区别：
	 //窗口关闭后，消息循环中首先 get 到 WM_CLOSE，然后经过默认处理函数 DefWindowProc 后发出 WM_DESTROY 消息，然后我们通过 PostQuitMessage 处理这个销毁消息。
	 //此处若 return 0;，主线程的消息循环无法 get 到 WM_DESTROY 消息，窗口也就不会结束
		break;
	}
	case WM_DESTROY://窗口即将销毁
	{
		//关闭 socket
		client.method_Close_clientsock(&client);//关闭 socket
		Cleanup_sock_api();//解除 api 绑定
		//不清楚什么原因，DefWindowProc() 不会默认处理销毁消息，没这句会导致残留进程无法退出，这里我们为他处理了。
		PostQuitMessage(0);//向消息队列中发送一个 quit 消息，getmessage() 取到消息后会返回 0，从而结束消息循环。
		break;
	}
	}
	return DefWindowProc(hWnd, msg, wparam, lparam);//默认处理我们不关心的消息
}



//*************windows功能函数*******************
void Creat_ChildWindow(HWND hWnd)//创建子窗口（控件）
{
	RECT rect;
	GetClientRect(hWnd, &rect);//获取客户区
	//创建子窗口（控件）时，menu 参数为“控件ID”，不过这里我们不用
	edit_send_hWnd = CreateWindow("edit", NULL, EDIT_SEND, 
		10,//左上角 X 坐标
		rect.bottom - 50 + 10,//左上角 Y 坐标
		rect.right - 10 - 10 - 100,//宽
		35,//高 
		hWnd, NULL, hinst, NULL);

	edit_recv_hWnd = CreateWindow("edit", NULL, EDIT_RECV, 
		10,//左上角 X 坐标
		40,//左上角 Y 坐标
		rect.right - 10 - 10,//宽
		rect.bottom - 10 - 50 - 30,//高 
		hWnd, NULL, hinst, NULL);

	button_hWnd = CreateWindow("button", "Send", BUTTON, 
		10 + rect.right - 10 - 10 - 100 + 10,//左上角 X 坐标
		rect.bottom - 50 + 10,//左上角 Y 坐标
		90,//宽
		35,//高 
		hWnd, NULL, hinst, NULL);
}



void SetFont()//设置字体
{
	//这里不是正规写法
	//创建字体，目的是自动获取缺省值
	font = CreateFontA(25, 0, 0, 0, 1000, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, "微软雅黑");//一些风格均可置 0 ，不知原理....可能是可缺省'
	//获取对象结构体，里面填充了缺省值
	GetObject(font, sizeof LogFont, &LogFont);
	//真正创建字体
	font = CreateFontIndirectA(&LogFont);
	//向空间发送消息 设置字体
	SendMessage(edit_recv_hWnd, WM_SETFONT, (WPARAM)font, 0);

	
	//创建字体，目的是自动获取缺省值
	font = CreateFontA(35, 0, 0, 0, 1000, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, "微软雅黑");
	//获取对象结构体，里面填充了缺省值
	GetObject(font, sizeof LogFont, &LogFont);
	//真正创建字体
	font = CreateFontIndirectA(&LogFont);
	//向空间发送消息 设置字体
	SendMessage(edit_send_hWnd, WM_SETFONT, (WPARAM)font, 0);


	//这里是获取 textout 字体对象,往后要一直用
	font = CreateFont(30, 0, 0, 0, 1000, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, "微软雅黑");
}



void AddEditStr(HWND hWnd, char* str)//文本框 加入文本
{
	SendMessageA(hWnd, EM_SETSEL, -2, -1);//选择尾部文本
	SendMessageA(hWnd, EM_REPLACESEL, TRUE, (long)str);//覆盖尾部文本
	SendMessageA(hWnd, WM_VSCROLL, SB_BOTTOM, 0);//滚轮到底
}








//-----------------------------------------------socket系函数-----------------------------------------------

//*************socket线程*******************
void createsock(void* useless)
{
	struct timeval tim = { 2,0 }; //这个结构体用于储存我们读描述符时的超时等待时间
	if (client.method_Create_clientsock == NULL)
	{//初始化对象
		Get_object_client(&client);//置对象方法
		{//置对象成员属性
			client.member_buf_lenth = 1024;
			client.member_timeout = tim;
			client.member_callback_connect_succeed = connect_succeed_callback;
			client.member_callback_data_coming = data_coming_callback;
			client.member_callback_error = error_callback;
			client.member_callback_server_leave = server_leave_callback;
			client.member_ip = IP;
			client.member_port = PORT;
		}
	}

	Startup_sock_api();
	sock_state = client.method_Create_clientsock(&client);
}



//*************socket四个回调*******************
void server_leave_callback(SOCKET client_sock, int error) {//服务端离开回调
	//这个回调函数在线程中被调用，故第一个参数必填父窗口句柄，用来阻塞主线程。
	if (MessageBox(parent_hWnd, "服务器失去连接！", "错误", MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_ICONWARNING) != IDRETRY)exit(-1);
	connect_server(parent_hWnd);
}
void data_coming_callback(SOCKET client_sock, char* data) {//数据到达回调
	if (data[0] == ' ')//更新人数
	{
		RECT rect = {40,0,800,0};
		rect.bottom = 40;
		rect.right = 800;
		sscanf(data," %d", &member);
		InvalidateRect(parent_hWnd, &rect, TRUE);
		UpdateWindow(parent_hWnd);//发送 WM_PAINT 消息到父窗口消息队列中,这里使用 sendmessage(WM_PAINT) 也可
		return;
	}
	AddEditStr(edit_recv_hWnd, data);//加入到文本框
}
void connect_succeed_callback(SOCKET client_sock) {//connect 成功回调
	sock_state = 1;//标志 connect 成功
}
void error_callback(int error) {//异常 debug 回调
	
	char buf[100] = { 0 };
	wsprintf(buf, "socket异常，错误码：%d\n", error);
	OutputDebugString(buf);
}



//*************socket功能函数*******************
void SendData(char* str)//发送数据
{
	client.method_Send_msg(&client, str, strlen(str));
}



void connect_server(HWND hWnd)//服务器 连接/重连
{
	do
	{
		sock_state = 0;
		_beginthread(createsock, 0, NULL);
		while (1)
		{
			if (sock_state == 1)//connect 成功，这里是 createclient 调用回调函数后修改 sock_state 为1
			{
				break;
			}
			else if (sock_state == ERROR_CLI_SOCK)//socket 调用出错，这里是 createclient 返回-1
			{
				break;
			}
			else if (sock_state == ERROR_CLI_CNCT)//connect 调用出错，这里是 createclient 返回-2
			{
				break;
			}
		}
		if (sock_state == 1)
		{
			break;
		}
		else if (sock_state == ERROR_CLI_SOCK)
		{
			if (MessageBox(hWnd, "socket 描述符获取失败！", "错误", MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_ICONWARNING) != IDRETRY)exit(-1);//通知用户socket获取失败
		}
		else if (sock_state == ERROR_CLI_CNCT)
		{
			if (MessageBox(hWnd, "服务器没有响应！", "错误", MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_ICONWARNING) != IDRETRY)exit(-1);//通知用户服务端关闭,若不重试则关闭窗口
		}
	} while (1);
	Client_Socket = client.method_Get_clientsock(&client);
}
