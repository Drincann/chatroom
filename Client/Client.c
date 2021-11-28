
//socket �ӿ�
#include"Gao_socket_Class.h"//winsock2.h ������ windows.h
#pragma comment(lib,"Gao_socket_Class.lib")

//�߳�
#include <process.h>

//�ַ�������
#include <stdio.h>

//�ؼ����
#define EDIT_RECV (WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY)//���� recv �ı���� style
#define EDIT_SEND (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL)//���� send �ı���� style
#define BUTTON (WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON)//������ť�� style

//���������ڿ�߸ı�ʱ�ı������ݳ����쳣��/r/n�����⣩�����Ķ������ڱ߿��С���ɵ�������������������Ĵ��ڷ����ʱ���������Ժ�����
//�����ڷ��Ŀǰ���ʹ�� WS_OVERLAPPEDWINDOW ��������Ϸ�񣬰���WS_OVERLAPPED, WS_CAPTION, WS_SYSMENU, WS_THICKFRAME, WS_MINIMIZEBOX, WS_MAXIMIZEBOX
//#define WINDOWS (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

//��������Ϣ
#define PORT 50055//����˼����˿�
#define IP "47.94.7.74"//������IP
//#define IP "127.0.0.1"//����IP����Debug��

//����������
//���ھ�̬����д���˽��ջ�����Ϊ 1kb����Ȼ���Էֶ��ȡ�����ݣ�������ϣ����ÿ��ȡ����������Ϊ������һ��������
//�ͻ��˱�֤���͵����ݲ����� 1kb
#define BUFSIZE 1024


//-----------------------------------------------����-----------------------------------------------

//*************��������*******************
//���� edit ����Ϣ��Ҫ�� translatemsg ������ǰ�������أ��ʽ����о��ͳһ����Ϊȫ�ֱ���
HWND parent_hWnd, edit_send_hWnd, edit_recv_hWnd, button_hWnd;//�����ھ�����Ӵ��ڣ��ؼ������
HINSTANCE hinst;//ʵ�������̣����
MSG msg;//������Ϣ�ṹ��

HFONT font;//���壨�����ؼ�ʱ��Ϊ�м����ʹ�ã�����Ϊ���� textout ���������
LOGFONT LogFont;//����

SOCKET Client_Socket = 0;//�ͻ��� socket ������
Client client = { 0 };//Client ����δ��ʼ��

int sock_state = 0;//socket ����״̬

char UserName[100] = "*    ";//�û���
unsigned long UserName_size = 0;//�û�������

unsigned int member = 0;//��������

//*************��������*******************
//windows���
LRESULT CALLBACK wnd_proc_callback(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);//windows������Ϣ�Ļص�����
void Creat_ChildWindow(HWND hWnd);//�������ڿؼ�
void SetFont();//��������

//�ͻ��˻ص�����
void server_leave_callback(SOCKET client_sock, int error);//������뿪�ص�
void data_coming_callback(SOCKET client_sock, char* data);//���ݵ���ص�
void connect_succeed_callback(SOCKET client_sock);//connect �ɹ��ص�
void error_callback(int error);//�쳣 debug �ص�

//����
void AddEditStr(HWND hWnd, char* str);//�ı��� �����ı�
void SendData(char* str);//�����˷��� send�ı�������
void createsock(void*);//socket�߳�
void connect_server(HWND hWnd);//����������




//-----------------------------------------------������-----------------------------------------------

//*************WINAPI��ں���*******************
int	WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR szCmdLine, _In_  int nCmdShow)//���
{
	{//������Ϣǰ׺��username����
		DWORD size = 100;
		GetUserName(UserName + 5, &size);//�����ȡ�û���
		strcat(UserName, "��");//����ð��
		UserName_size = strlen(UserName);
	}


	WNDCLASS windows = { 0 }; hinst = hInstance;
	//���ô�������Ϣ
	{
		windows.cbClsExtra = 0; windows.cbWndExtra = 0;
		windows.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);//����
		windows.hCursor = LoadCursor(NULL, IDC_ARROW);//���
		windows.hIcon = LoadIcon(NULL, IDI_APPLICATION);//ͼ��
		windows.hInstance = hInstance;//ʵ�����
		windows.lpfnWndProc = wnd_proc_callback;//�ص�����
		windows.lpszClassName = "Chat";//����
		windows.lpszMenuName = NULL;//�˵���
		windows.style = CS_VREDRAW | CS_HREDRAW;//���ڷ�񣬸ı��͸�ʱ�ػ�
	}
	//ע�ᴰ����
	if (!RegisterClass(&windows))exit(-1);


	//��������
	parent_hWnd = //���ھ��
	CreateWindow
	(
		"Chat",//��ṹ����������ͬ
		"Chat Room(��ѧһ�꼶 ��һѧ�� C���Կγ����)",//���ڱ���
		WS_OVERLAPPEDWINDOW,//������ʽ����� WS_OVERLAPPED, WS_CAPTION, WS_SYSMENU, WS_THICKFRAME, WS_MINIMIZEBOX, WS_MAXIMIZEBOX
		(GetSystemMetrics(SM_CXSCREEN) - 800) / 2, //X ����
		(GetSystemMetrics(SM_CYSCREEN) - 600) / 2, //Y ����
		800, //��
		600, //��
		NULL,NULL,
		hInstance,//ʵ����������̣�
		NULL
	);


	ShowWindow(parent_hWnd,nCmdShow);//��ʾ����
	UpdateWindow(parent_hWnd);//���´��ڣ����Բ�д�������ƹ���Ϣ���У�ֱ�ӵ��� windows �ص��������� WM_PAINT ��Ϣ����ǰ���Ǵ��ڱ��������Ч���򣬷�����ԡ���

	//��ʼ����Ϣ�����л�ȡ��Ϣ
	while (GetMessage(&msg, NULL, 0, 0))//�ڶ��������� NULL �����ոý������д�����Ϣ
	{
		//���� �����ı��� �� ��ť�� ��Ϣ
		if (msg.hwnd == edit_send_hWnd && msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN)//�ж���Ϣ���� send�ı���
		{
			//�������ݸ�����ˣ����ﱣ֤���͵��������಻����1024k����ֹ�ص������ظ����ã�
			char buf[BUFSIZE] = { 0 };//sock��д���˻�����Ϊ1024B����������Ҳд1024B
			strcat(buf, UserName);
			GetWindowText(edit_send_hWnd,buf + UserName_size, BUFSIZE - UserName_size);//���������Ʒǳ��ܵ���żȻ���֣�����Ϊ�˷�ֹĩβ������0����д��һ���ֽ�ʱ����ʵ������д�������ֽڣ�Ҳ����˵�����Ὣ�ַ���ĩβ��0Ҳ������Ҫд��ĳ�����

			if (buf[UserName_size] != 0)//�û�����ǿ�
			{
				SetWindowText(edit_send_hWnd, NULL);
				SendData(buf);                               //����˷��ص��������ݰ���
			}												 //1.ʱ�� + ���� + ������ + ���� + ���ݣ� + ���� + ����
			else											 //�ͻ��˸��� �ͻ������� =������ + ���� + ���ݣ�
			{												 //����˸��� ʱ�� + ���� + ƴ�ӿͻ������� + ���� + ����
				MessageBeep(0xFFFFFFFF);
			}
			continue;//���� TranslateMessage(&msg) ����ֹ�ü�����Ϣ�Ĵ���
		}
		else if (msg.hwnd == button_hWnd && msg.message == WM_LBUTTONUP)//�ж���Ϣ���� ��ť
		{
			//�������ݸ������,���ﲻ continue���ô���ȱʡ������Ϣ������ؼ�����
			char buf[BUFSIZE] = { 0 };//sock��д���˻�����Ϊ1024B����������Ҳд1024B
			strcat(buf, UserName);
			GetWindowText(edit_send_hWnd, buf + UserName_size, BUFSIZE - UserName_size);

			if (buf[UserName_size] != 0)//�û�����ǿ�
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
		//������Ϣ���ûص��������ַ���Ϣ��������Ϣ������ص������д����Ӧ������
	}
}








//-----------------------------------------------windowsϵ����-----------------------------------------------

//*************windows�ص�*******************
LRESULT CALLBACK wnd_proc_callback(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{//�ص�����
	static int Activate_flag = 1;//����״μ���״̬
	static int Roll_D = 0;//�������ײ������λ�õĲ�ֵ
	switch (msg)//���ﴦ�����ǹ��ĵ���Ϣ
	{
	case WM_CREATE://���ڴ���
	{
		Creat_ChildWindow(hWnd);//�����Ӵ��ڣ��ؼ���
		SetFont();//��������
		SetFocus(edit_send_hWnd);
		break;
	}
	case WM_ACTIVATE://�����ʧȥ����
	{
		if (Activate_flag == 1)
		{//�����״μ���,��������δ��ʾ���ȴ���һ�� WM_SIZE ����ʱ����ʾ
			Activate_flag = 0;
		}
		SetFocus(edit_send_hWnd);//���ڼ���ʱ�����ı����ȡ����
		return 0;//������� break ������Ĭ����Ϣ����������� WM_ACTIVATE ��ȱʡ������ʹ�����ڻ�ȡ���㣬�ʴ˴����ؼ��ɡ�
	}
	case WM_PAINT://�����ػ�
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		char buf[50] = { 0 };

		SelectObject(hdc, font);//ѡ���������
		SetBkMode(hdc, TRANSPARENT);//���屳��͸��

		sprintf(buf, "��ǰ����������%d", member);
		TextOut(hdc, 10, 10, buf, lstrlen(buf));
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_SIZING://���ڴ�С���ı�ǰ�յ�����Ϣ���������ù�������ʼλ��
	{
		int min, max;
		SCROLLBARINFO roll_info = { 0 }; roll_info.cbSize = sizeof(SCROLLBARINFO);

		GetScrollRange(edit_recv_hWnd, SB_VERT, &min, &max);//��ȡ���������ֵ
		GetScrollBarInfo(edit_recv_hWnd, OBJID_VSCROLL, &roll_info);//��ȡ SCROLLBARINFO �ṹ�壬���а����������߶�
		Roll_D = max - GetScrollPos(edit_recv_hWnd, SB_VERT) - roll_info.dxyLineButton;
		break;
	}
	case WM_SIZE://���ڴ�С���ı�
	{
		//���ڿɵ�����������12

		//���ڿؼ��ƶ�ʱ�ؼ����ݻᷢ���仯����ʱ�޷���������Ը����˹̶��߿򣬲�ע�͵��� movewindow ���ִ��롣
		RECT rect;
		GetClientRect(hWnd, &rect);//��ȡ�ͻ���
		//����λ��
		MoveWindow(edit_recv_hWnd,
			10,//���Ͻ� X ����
			40,//���Ͻ� Y ����
			rect.right - 10 - 10,//��
			rect.bottom - 10 - 50 - 30,//�� 
			FALSE
		);
		MoveWindow(edit_send_hWnd,
			10,//���Ͻ� X ����
			rect.bottom - 50 + 10,//���Ͻ� Y ����
			rect.right - 10 - 10 - 100,//��
			35,//�� 
			FALSE
		);
		MoveWindow(button_hWnd,
			10 + rect.right - 10 - 10 - 100 + 10,//���Ͻ� X ����
			rect.bottom - 50 + 10,//���Ͻ� Y ����
			90,//��
			35,//�� 
			TRUE
		);

		int min, max;
		SCROLLBARINFO roll_info = { 0 }; roll_info.cbSize = sizeof(SCROLLBARINFO);

		GetScrollRange(edit_recv_hWnd, SB_VERT, &min, &max);//��ȡ���������ֵ
		GetScrollBarInfo(edit_recv_hWnd, OBJID_VSCROLL, &roll_info);//��ȡ SCROLLBARINFO �ṹ�壬���а����������߶�
		//��recv�ı����� WM_VSCROLL ��Ϣ���䴫��ص������еĲ���Ϊ �������϶���ֹλ�� �� SB_THUMBPOSITION���ֱ��� wParam �ĸ��ֽں͵��ֽ�λ��
		//MAKELONG�� ��������һ��32λ�����ݣ���Ӧ�Ļ���MAKEWORD����������һ��16λ���ݣ�
		SendMessage(edit_recv_hWnd,WM_VSCROLL, MAKELONG(SB_THUMBPOSITION, max - roll_info.dxyLineButton - Roll_D), 0L);
		

		//�ػ�textout
		//1.ĳЩ����£���һ�λ��ػ�ʧ��
		//2.��WM_PAINT��Ϣ���ػ��ᵼ�´����״μ�����һ���ػ�ʧ��
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		char buf[50] = { 0 };

		SelectObject(hdc, font);//ѡ���������
		SetBkMode(hdc, TRANSPARENT);//���屳��͸��

		sprintf(buf, "��ǰ����������%d", member);
		TextOut(hdc, 10, 10, buf, lstrlen(buf));
		EndPaint(hWnd, &ps);
		

		//�״���ʾ����
		if (Activate_flag == 0)
		{//�״μ������ʾ���ڣ���ʱ�� get �� WM_SIZE ��Ϣ����ʱ�Ǵ��ڵ�һ����ʾ��
			Activate_flag = -1;//�״μ����¼����ٴ���
			//���� socket �̣߳���ʼ������Ϣ
			connect_server(parent_hWnd);//�������������⣬һ��ʼ����δ��������ʱ���ӷ���ˣ���ʱ�����δ���أ�����NULLʹmessagebox�ĸ����ھ��ΪNULL
			break;
		}
		break;
	}
	case WM_CLOSE://���ڱ��ر�
	{//���ڹرպͼ������ٵ�����
	 //���ڹرպ���Ϣѭ�������� get �� WM_CLOSE��Ȼ�󾭹�Ĭ�ϴ����� DefWindowProc �󷢳� WM_DESTROY ��Ϣ��Ȼ������ͨ�� PostQuitMessage �������������Ϣ��
	 //�˴��� return 0;�����̵߳���Ϣѭ���޷� get �� WM_DESTROY ��Ϣ������Ҳ�Ͳ������
		break;
	}
	case WM_DESTROY://���ڼ�������
	{
		//�ر� socket
		client.method_Close_clientsock(&client);//�ر� socket
		Cleanup_sock_api();//��� api ��
		//�����ʲôԭ��DefWindowProc() ����Ĭ�ϴ���������Ϣ��û���ᵼ�²��������޷��˳�����������Ϊ�������ˡ�
		PostQuitMessage(0);//����Ϣ�����з���һ�� quit ��Ϣ��getmessage() ȡ����Ϣ��᷵�� 0���Ӷ�������Ϣѭ����
		break;
	}
	}
	return DefWindowProc(hWnd, msg, wparam, lparam);//Ĭ�ϴ������ǲ����ĵ���Ϣ
}



//*************windows���ܺ���*******************
void Creat_ChildWindow(HWND hWnd)//�����Ӵ��ڣ��ؼ���
{
	RECT rect;
	GetClientRect(hWnd, &rect);//��ȡ�ͻ���
	//�����Ӵ��ڣ��ؼ���ʱ��menu ����Ϊ���ؼ�ID���������������ǲ���
	edit_send_hWnd = CreateWindow("edit", NULL, EDIT_SEND, 
		10,//���Ͻ� X ����
		rect.bottom - 50 + 10,//���Ͻ� Y ����
		rect.right - 10 - 10 - 100,//��
		35,//�� 
		hWnd, NULL, hinst, NULL);

	edit_recv_hWnd = CreateWindow("edit", NULL, EDIT_RECV, 
		10,//���Ͻ� X ����
		40,//���Ͻ� Y ����
		rect.right - 10 - 10,//��
		rect.bottom - 10 - 50 - 30,//�� 
		hWnd, NULL, hinst, NULL);

	button_hWnd = CreateWindow("button", "Send", BUTTON, 
		10 + rect.right - 10 - 10 - 100 + 10,//���Ͻ� X ����
		rect.bottom - 50 + 10,//���Ͻ� Y ����
		90,//��
		35,//�� 
		hWnd, NULL, hinst, NULL);
}



void SetFont()//��������
{
	//���ﲻ������д��
	//�������壬Ŀ�����Զ���ȡȱʡֵ
	font = CreateFontA(25, 0, 0, 0, 1000, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, "΢���ź�");//һЩ�������� 0 ����֪ԭ��....�����ǿ�ȱʡ'
	//��ȡ����ṹ�壬���������ȱʡֵ
	GetObject(font, sizeof LogFont, &LogFont);
	//������������
	font = CreateFontIndirectA(&LogFont);
	//��ռ䷢����Ϣ ��������
	SendMessage(edit_recv_hWnd, WM_SETFONT, (WPARAM)font, 0);

	
	//�������壬Ŀ�����Զ���ȡȱʡֵ
	font = CreateFontA(35, 0, 0, 0, 1000, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, "΢���ź�");
	//��ȡ����ṹ�壬���������ȱʡֵ
	GetObject(font, sizeof LogFont, &LogFont);
	//������������
	font = CreateFontIndirectA(&LogFont);
	//��ռ䷢����Ϣ ��������
	SendMessage(edit_send_hWnd, WM_SETFONT, (WPARAM)font, 0);


	//�����ǻ�ȡ textout �������,����Ҫһֱ��
	font = CreateFont(30, 0, 0, 0, 1000, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, "΢���ź�");
}



void AddEditStr(HWND hWnd, char* str)//�ı��� �����ı�
{
	SendMessageA(hWnd, EM_SETSEL, -2, -1);//ѡ��β���ı�
	SendMessageA(hWnd, EM_REPLACESEL, TRUE, (long)str);//����β���ı�
	SendMessageA(hWnd, WM_VSCROLL, SB_BOTTOM, 0);//���ֵ���
}








//-----------------------------------------------socketϵ����-----------------------------------------------

//*************socket�߳�*******************
void createsock(void* useless)
{
	struct timeval tim = { 2,0 }; //����ṹ�����ڴ������Ƕ�������ʱ�ĳ�ʱ�ȴ�ʱ��
	if (client.method_Create_clientsock == NULL)
	{//��ʼ������
		Get_object_client(&client);//�ö��󷽷�
		{//�ö����Ա����
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



//*************socket�ĸ��ص�*******************
void server_leave_callback(SOCKET client_sock, int error) {//������뿪�ص�
	//����ص��������߳��б����ã��ʵ�һ������������ھ���������������̡߳�
	if (MessageBox(parent_hWnd, "������ʧȥ���ӣ�", "����", MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_ICONWARNING) != IDRETRY)exit(-1);
	connect_server(parent_hWnd);
}
void data_coming_callback(SOCKET client_sock, char* data) {//���ݵ���ص�
	if (data[0] == ' ')//��������
	{
		RECT rect = {40,0,800,0};
		rect.bottom = 40;
		rect.right = 800;
		sscanf(data," %d", &member);
		InvalidateRect(parent_hWnd, &rect, TRUE);
		UpdateWindow(parent_hWnd);//���� WM_PAINT ��Ϣ����������Ϣ������,����ʹ�� sendmessage(WM_PAINT) Ҳ��
		return;
	}
	AddEditStr(edit_recv_hWnd, data);//���뵽�ı���
}
void connect_succeed_callback(SOCKET client_sock) {//connect �ɹ��ص�
	sock_state = 1;//��־ connect �ɹ�
}
void error_callback(int error) {//�쳣 debug �ص�
	
	char buf[100] = { 0 };
	wsprintf(buf, "socket�쳣�������룺%d\n", error);
	OutputDebugString(buf);
}



//*************socket���ܺ���*******************
void SendData(char* str)//��������
{
	client.method_Send_msg(&client, str, strlen(str));
}



void connect_server(HWND hWnd)//������ ����/����
{
	do
	{
		sock_state = 0;
		_beginthread(createsock, 0, NULL);
		while (1)
		{
			if (sock_state == 1)//connect �ɹ��������� createclient ���ûص��������޸� sock_state Ϊ1
			{
				break;
			}
			else if (sock_state == ERROR_CLI_SOCK)//socket ���ó��������� createclient ����-1
			{
				break;
			}
			else if (sock_state == ERROR_CLI_CNCT)//connect ���ó��������� createclient ����-2
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
			if (MessageBox(hWnd, "socket ��������ȡʧ�ܣ�", "����", MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_ICONWARNING) != IDRETRY)exit(-1);//֪ͨ�û�socket��ȡʧ��
		}
		else if (sock_state == ERROR_CLI_CNCT)
		{
			if (MessageBox(hWnd, "������û����Ӧ��", "����", MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_ICONWARNING) != IDRETRY)exit(-1);//֪ͨ�û�����˹ر�,����������رմ���
		}
	} while (1);
	Client_Socket = client.method_Get_clientsock(&client);
}
