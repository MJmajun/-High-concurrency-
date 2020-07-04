#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include "stdio.h"
#include <vector>

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

std::vector<SOCKET> g_clients;	//存放所有的socket

int processor(SOCKET _cSock)
{

	//缓冲区
	char szRecv[4096] = {};
	int len = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (len <= 0)
	{
		printf("客户端已经退出，任务结束\n");
		return -1;
	}
	switch (header->cmd)
	{
		printf("switch\n");
		case CMD_LOGIN:
		{
			//注意 这里为什么要加 sizeof(DataHeader)和减去sizeof(DataHeader)  是因为 前面 我们已经接受了一次头  所以 这里要做地址偏移
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Login *login = (Login*)szRecv;
			printf("收到命令：CMD_LOGIN, 数据长度%d ,userName = %s Password = %s\n", login->dataLength, login->userName, login->passWord);
			//忽略用户密码是否正确的过程
			LoginResult ret;
			send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout *logout = (Logout*)szRecv;
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			printf("收到命令：CMD_LOGOUT, 数据长度%d ,userName = %s\n", logout->dataLength, logout->username);
			//忽略用户密码是否正确的过程
			LogoutResult ret;
			send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
			break;
		}
		default:
		{
			DataHeader header = { 0,CMD_ERROR };
			send(_cSock, (char*)&header, sizeof(header), 0);
			break;
		}
	}
	return 0;
}
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


	while (true)
	{
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock,&fdRead);
		FD_SET(_sock,&fdWrite);
		FD_SET(_sock,&fdExp);

		for (int n  =(int)g_clients.size()- 1;n>=0 ;n--)
		{
			FD_SET(g_clients[n],&fdRead);
		}

		//nfds是一个整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量，即是所有文件描述符最大值+1，再window中，该值可以写0
		int ret = select(_sock+1,&fdRead,&fdWrite,&fdExp,NULL );
		if (ret < 0)
		{
			printf("select 任务结束\n");
			break;
		}
		if (FD_ISSET(_sock,&fdRead))	//FD_ISSET:判断一个文件描述符是否在一个集合中，返回值:在1,不在0
		{
			FD_CLR(_sock,&fdRead);	//FD_CLR:将一个文件描述符从集合中移除
			
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
			g_clients.push_back(_cSock);	//新的套接字直接加入到动态数组中
			printf("新的客户端加入：IP = %s \n", inet_ntoa(clientAddr.sin_addr));
			
		}

		for (size_t n =0; n < fdRead.fd_count; n++)
		{
			int result = processor(fdRead.fd_array[n]);
			if (-1 == result)	//说明有程序退出了  我们就应该找到这个 然后移除它 
			{
				auto iter = find(g_clients.begin(),g_clients.end(),fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
	}
	for (int n = (int)g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]);	//关闭
	}

	//6、关闭套接字
	closesocket(_sock);

	//用Socket API 建立简易TCP客户端
	//1、建立一个socket
	//2、bind 绑定用于接收客户端里连接的网络端口
	WSACleanup();
	printf("已退出\n");
	getchar();
	return 0;
}