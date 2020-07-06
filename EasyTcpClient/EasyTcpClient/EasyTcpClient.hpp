#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS

	#include <WinSock2.h>
	#include <windows.h>
	#pragma comment(lib,"ws2_32.lib")       //解决库调用  我们用通用的方法 已经在属性中添加了
#else
	#include <unistd.h>             //unix的标准库
	#include <arpa/inet.h>
	#include <string.h>

	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR (-1)
#endif

#include <iostream>
using namespace std;
#include "MessageHeader.hpp"

class EasyTcpClient
{
private:
	SOCKET _sock;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//初始化socket
	void InitSoket()
	{
		//启动window socket 的环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//1、建立一个套接字
		if (INVALID_SOCKET != _sock)
		{
			cout << "Socket = "<<_sock <<" 的连接已经关闭"<<endl;
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			cout<<"错误，建立Socket = "<<_sock<<" 失败..."<<endl;
		}
		else
		{
			cout<< "建立Socket = " << _sock << "成功..." << endl;
		}
	}

	//连接服务器
	int Connect(const char* ip,unsigned short port)
	{
		//2、连接服务器 connect
		//if (_sock == INVALID_SOCKET)
		//{
		//	InitSoket();
		//}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);	//127.0.0.1
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			cout << "Socket = "<<_sock<<" 连接服务器<"<< ip <<" : "<<port<<" > 失败..." << endl;
		}
		else
		{
			cout <<"Socket = "<<_sock<<" 连接服务器<" << ip << " : " << port << " >成功..." << endl;
		}
		return ret;
	}

	//收数据

	//发数据
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
		
	}

	//查询数据
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdRead;
			FD_ZERO(&fdRead);
			FD_SET(_sock, &fdRead);

			timeval t = { 1,0 };//前面是秒 后面是毫秒       让select每隔1秒去扫描一下
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);

			if (ret < 0)
			{
				Close();
				cout << "Socket = " << _sock<< " select 任务结束2"<<endl;
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				if (-1 == RecvData(_sock))
				{
					cout << "Socket = " << _sock<< " select 任务结束2"<<endl;
					Close();
					return false;
				}
			}		
		}
		return true;	
	}

	//判断是否有连接
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//收数据据   处理粘包和拆包
	int RecvData(SOCKET _cSock)
	{
		//缓冲区
		char szRecv[4096] = {};
		int len = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (len <= 0)
		{
			cout<< "Socket = " <<_cSock<<" 与服务器断开连接，任务结束"<<endl;
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);
		return 0;
	}

	//响应网络
	void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
			{
				LoginResult *login = (LoginResult*)header;
				cout<<"收到服务器的消息：CMD_LOGIN_RESULT, 数据长度:"<< login->dataLength<<endl;

				break;
			}
			case CMD_LOGOUT_RESULT:
			{
				LogoutResult *logout = (LogoutResult*)header;
				cout << "收到服务器的消息：CMD_LOGOUT_RESULT, 数据长度:" << logout->dataLength << endl;
				break;
			}
			case CMD_NEW_USER_JOIN:
			{
				NewUserJoin *userjoin = (NewUserJoin*)header;
				cout << "收到服务器的消息：CMD_NEW_USER_JOIN, 数据长度:"<< userjoin->dataLength<<endl;
				break;
			}
		}
	}

	//关闭socket
	void Close()
	{
		//防止重复调用
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			WSACleanup();
			closesocket(_sock);
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}

	}
};

void cmdThread(EasyTcpClient* client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		/*scanf("%s", cmdBuf);*/
		cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->Close();
			cout << "退出线程" << endl;
			break;
		}
		else if (0 == strcmp(cmdBuf, "Login"))
		{
			Login login;
			strcpy(login.userName, "majun");
			strcpy(login.passWord, "518811");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "Logout"))
		{
			Logout logout;
			strcpy(logout.username, "majun");
			client->SendData(&logout);
		}
		else {
			cout << "不支持的命令" << endl;
		}
	}
}


#endif // !_EasyTcpClient_hpp_

