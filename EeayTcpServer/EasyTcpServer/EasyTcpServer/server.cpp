#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include "stdio.h"

//#pragma comment(lib,"ws2_32.lib");	//解决库调用  我们用通用的方法 已经在属性中添加了

enum CMD	//枚举登录和登出
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR
};

struct DataHeader	//定义数据包头
{
	short dataLength;
	short cmd;
};

struct Login : public DataHeader		//定义一个结构体封装数据
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};

struct LoginResult : public DataHeader	//返回登录的结果
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 1;
	}
	int result;
};

struct Logout : public DataHeader		//返回谁要退出
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char username[32];
};
struct LogoutResult : public DataHeader		//返回登出的结果
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

int main()
{
	//启动window socket 环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//用Socket API 建立简易TCP客户端

	//1、建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//2、连接服务器connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;	//或者 inet_addr("127.0.0.1")
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("错误，绑定端口失败...\n");
	}
	else
	{
		printf("绑定端口成功...\n");
	}

	//3、listen 监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("错误，监听端口失败...\n");
	}
	else
	{
		printf("监听端口成功...\n");
	}

	//4、accept 等待接收客户端的连接
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("错误，接收到无效客户端...\n");
	}
	else
	{
		printf("监听端口成功...\n");
	}
	printf("新的客户端加入：IP = %s \n", inet_ntoa(clientAddr.sin_addr));

	while (true)
	{
		DataHeader header = {};
		int len = recv(_cSock,(char*)& header,sizeof(DataHeader),0);
		if (len <= 0)
		{
			printf("客户端已经退出，任务结束\n");
			break;
		}

		switch (header.cmd)
		{
			case CMD_LOGIN:
			{
				Login login = {};

				//注意 这里为什么要加 sizeof(DataHeader)和减去sizeof(DataHeader)  是因为 前面 我们已经接受了一次头  所以 这里要做地址偏移
				recv(_cSock, (char*)& login+sizeof(DataHeader), sizeof(Login)-sizeof(DataHeader), 0);
				printf("收到命令：CMD_LOGIN, 数据长度%d ,userName = %s Password = %s\n",login.dataLength,login.userName,login.passWord);
				//忽略用户密码是否正确的过程
				LoginResult ret;			
				send(_cSock,(char*)&ret,sizeof(LoginResult),0 );
				break;
			}
			case CMD_LOGOUT :
			{
				Logout logout = {};
				recv(_cSock, (char*)& logout+sizeof(DataHeader), sizeof(Logout)-sizeof(DataHeader), 0);
				printf("收到命令：CMD_LOGOUT, 数据长度%d ,userName = %s\n", logout.dataLength,logout.username);
				//忽略用户密码是否正确的过程
				LogoutResult ret;		
				send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
				break;
			}
			default:
				header.cmd = CMD_ERROR;
				header.dataLength = 0;
				send(_cSock,(char*)&header,sizeof(header),0);
				break;
		}
	}


	//6、关闭套接字
	closesocket(_sock);


	//用Socket API 建立简易TCP客户端
	//1、建立一个socket
	//2、bind 绑定用于接收客户端里连接的网络端口


	WSACleanup();
	return 0;
}