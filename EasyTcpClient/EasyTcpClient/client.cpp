#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include "stdio.h"


//#pragma comment(lib,"ws2_32.lib");	//解决库调用  我们用通用的方法 已经在属性中添加了

enum CMD	//枚举登录和登出
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};

struct DataHeader	//定义数据包头
{
	short dataLength;
	short cmd;
};

struct Login	//DatePackage	//定义一个结构体封装数据
{
	char userName[32];
	char passWord[32];
};

struct LoginResult		//返回登录的结果
{
	int result;
};

struct Logout		//返回谁要退出
{
	char username[32];
};
struct LogoutResult		//返回登出的结果
{
	int result;
};
int main()
{
	//启动window socket 环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	
	//1、建立一个套接字
	SOCKET _sock = socket(AF_INET,SOCK_STREAM,0);
	if (INVALID_SOCKET == _sock)
	{
		printf("错误，建立Socket失败...\n");
	}
	else
	{
		printf("建立Socket成功...\n");
	}

	//2、连接服务器 connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("错误，连接Socket失败...\n");
	}
	else
	{
		printf("连接Socket成功...\n");
	}
	char cmdBuf[128] = {};
	while (true)
	{
		//3、输入请求命令
		scanf("%s",cmdBuf);
		//4、处理请求命令
		if (0 == strcmp(cmdBuf,"exit"))
		{
			break;
		}
		else if (0 == strcmp(cmdBuf, "Login"))
		{
			Login login = { "majun","5188119" };
			DataHeader dh = { sizeof(login),CMD_LOGIN };
			//5、向服务器发送请求
			send(_sock, (const char *)&dh, sizeof(dh), 0);	//先发包头
			send(_sock, (const char *)&login, sizeof(login), 0);	//再发包体

			//再进行接受
			DataHeader retHeader = {};
			LoginResult loginret = {};

			recv(_sock,(char*)&retHeader,sizeof(retHeader),0 );
			recv(_sock, (char*)&loginret, sizeof(loginret), 0);
			printf("LoginResult = %d\n ",loginret.result);
		}
		else if (0 == strcmp(cmdBuf, "Logout"))
		{
			Logout logout = { "majun" };
			DataHeader dh = { sizeof(logout),CMD_LOGOUT };
			//5、向服务器发送请求
			send(_sock, (const char *)&dh, sizeof(dh), 0);	//先发包头
			send(_sock, (const char *)&logout, sizeof(logout), 0);	//再发包体

																	//再进行接受
			DataHeader retHeader = {};
			LogoutResult logoutret = {};

			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&logoutret, sizeof(logoutret), 0);
			printf("LoginResult = %d\n ", logoutret.result);
		}
		else
		{
			printf("不支持的命令，请重重新输入\n");
		}
	}
	
	WSACleanup();
	closesocket(_sock);
	getchar();
	return 0;
}


