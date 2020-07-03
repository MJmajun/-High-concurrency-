#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include "stdio.h"


//#pragma comment(lib,"ws2_32.lib");	//解决库调用  我们用通用的方法 已经在属性中添加了
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

	//3、接收服务器信息
	char recvBuf[256] = {};
	int nlen = recv(_sock,recvBuf,256,0 );
	if(nlen>0)
	{
		printf("接收到数据： %s",recvBuf);
	}

	
	WSACleanup();
	getchar();
	return 0;
}


