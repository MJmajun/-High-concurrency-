#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include "stdio.h"


//#pragma comment(lib,"ws2_32.lib");	//解决库调用  我们用通用的方法 已经在属性中添加了
int main()
{
	//启动window socket 环境
	WORD ver = MAKEWORD(2,2);
	WSADATA dat;
	WSAStartup(ver,&dat);
	
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
	if (SOCKET_ERROR == listen(_sock,5))
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
	char msgBuf[] = "hello,im server";
	while (true)
	{
		accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _cSock)
		{
			printf("错误，接收到无效客户端...\n");
		}
		else
		{
			printf("监听端口成功...\n");
		}
		printf("新的客户端加入：IP = %s \n",inet_ntoa(clientAddr.sin_addr));
		//5、send 向客户端发送一条数据

		send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
	}


	//6、关闭套接字
	closesocket(_sock);


	//用Socket API 建立简易TCP客户端
	//1、建立一个socket
	//2、bind 绑定用于接收客户端里连接的网络端口
	

	WSACleanup();
	return 0;
}


