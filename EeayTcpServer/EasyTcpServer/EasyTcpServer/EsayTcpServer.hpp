#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

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
#include <vector>
#include "MessageHeader.hpp"
using namespace std;
#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 10240
#endif // !RECV_BUFF_SIZE

class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		_lastpos = 0;
		memset(_szMsgBuf,0,sizeof(_szMsgBuf));
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	char* msgBuf()
	{
		return _szMsgBuf;
	}

	int getlast()
	{
		return _lastpos;
	}

	void setLast(int pos)
	{
		_lastpos = pos;
	}
	~ClientSocket() {}

private:
	SOCKET _sockfd;
	int _lastpos ;	//消息换冲区的数据尾部位置
	char _szMsgBuf[RECV_BUFF_SIZE * 10];	//第二缓冲区
};



class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;  //存放所有的socket
public:
	EasyTcpServer() 
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	//初始化socket
	void InitSocket()
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
			cout << "Socket = " << _sock << " 的连接已经关闭" << endl;
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			cout << "错误，建立Socket = " << _sock << " 失败..." << endl;
		}
		else
		{
			cout << "建立Socket = " << _sock << " 成功..." << endl;
		}
	}
	
	//绑定端口
	void BindSocket(const char* ip,unsigned short port)
	{
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		if (ip)	//ip不为空
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip); //或者 inet_addr("127.0.0.1"
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY; //或者 inet_addr("127.0.0.1")
		}
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);      //或者 inet_addr("127.0.0.1")
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY ;      //或者 inet_addr("127.0.0.1")
		}		
#endif // _WIN32
		
		int ret =::bind(_sock, (sockaddr*)&_sin, sizeof(_sin));		//由于std中的bing会和socket中的bind冲突，这里改用::进行访问
		if (SOCKET_ERROR == ret)
		{
			cout << "Socket = " << _sock << " 绑定端口失败" << endl;
		}
		else
		{
			cout << "Socket = " << _sock << " 绑定端口成功" << endl;
		}
	}
	//监听端口
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			cout << "Socket = " << _sock << " 监听端口失败" << endl;
		}
		else
		{
			cout << "Socket = " << _sock << " 监听端口成功" << endl;
		}
		return ret;
	}
	//接受客户端连接
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif // _WIN32

		if (INVALID_SOCKET == cSock)
		{
			cout << "Socket = " << cSock << "错误，接收到无效客户端" << endl;
		}
		else
		{
			NewUserJoin userjoin;
			SendDataToAll(&userjoin);
			ClientSocket* newSock =new  ClientSocket(cSock);
			_clients.push_back(newSock);    //直接将这个封装的客户端连接加入
			cout<< "新的客户端加入 Socket = " << cSock <<endl;
		}
		return cSock;
	}

	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			//伯克利套接字 BSD socket
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;

			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET maxSock = _sock;
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd() , &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}

			//nfds是一个整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量，
			//即是所有文件描述符最大值+1，再window中，该值可以写0   注意  是最大值加一！！！
			timeval t = { 1,0 };    //定时
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				cout << "select 任务结束" << endl;
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))   //FD_ISSET:判断一个文件描述符是否在一个集合中，返回值:在1,不在0
			{
				FD_CLR(_sock, &fdRead); //FD_CLR:将一个文件描述符从集合中移除
				Accept();
			}

			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{
					int result = RecvData(_clients[n]);           //处理接受到消息的套接字的信息
					if (-1 == result)       //说明有程序退出了  我们就应该找到这个 然后移除它
					{
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())
						{
							delete _clients[n];
							_clients.erase(iter);
						}
					}
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
	//是否在工作中
	//接受数据  处理粘包 拆分包
	int RecvData(ClientSocket* pClient)            //专门处理接受到的消息
	{
		//缓冲区
		char szRecv[4096] = {};
		int len = (int)recv(pClient->sockfd(), szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (len <= 0)
		{
			cout<<"Sokect = "<< pClient->sockfd()<< " 已经退出，任务结束"<<endl;
			return -1;
		}

		//将收取到的数据拷贝到消息缓冲区
		::memcpy(pClient->msgBuf()+pClient->getlast(), szRecv, len);
		//将消息缓冲区的数据尾部位置后移
		pClient->setLast(pClient->getlast() + len);
		//循环判断消息缓冲区的长度是否大于消息长度
		while (pClient->getlast() >= sizeof(DataHeader))	//解决粘包
		{
			//这时候就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//判断消息缓冲区的长度大于消息长度
			if (pClient->getlast() >= header->dataLength)		//解决少包
			{
				//消息缓冲区剩余未处理的数据的长度
				int nSize = pClient->getlast() - header->dataLength;

				//处理网络消息
				OnNetMsg(pClient->sockfd(), header);
				//将消息缓冲区剩余的数据前移
				::memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区的数据尾部向前移动
				pClient->setLast( nSize);
			}
			else
			{
				//剩余消息不足一完整消息
				break;
			}
		}
		return 0;
	}

	//相应网络消息
	virtual void OnNetMsg(SOCKET cSock,DataHeader* header)
	{
		switch (header->cmd)
		{
			case CMD_LOGIN:
			{
				//注意 这里为什么要加 sizeof(DataHe ader)和减去sizeof(DataHeader)  是因为 前面 我们已经接受了一次头  所以 这里要做地址偏移
				
				Login *login = (Login*)header;
				//cout<<"收到客户端 "<<cSock<<" 的命令：CMD_LOGIN, 数据长度= "<< login->dataLength <<" ,userName = "<< login->userName <<" Password = "<< login->passWord<<endl;
				//忽略用户密码是否正确的过程
				LoginResult ret;
				SendData(cSock, &ret);
				break;
			}
			case CMD_LOGOUT:
			{
				Logout *logout = (Logout*)header;
				//cout << "收到客户端 " << cSock << " 的命令：CMD_LOGOUT, 数据长度= " << logout->dataLength << " ,userName = " << logout->username << endl;
				//忽略用户密码是否正确的过程
				//LogoutResult ret;
				//SendData(cSock, &ret);
				break;
			}
			default:
			{
				cout << "收到 "<< cSock << " 未定义的消息 数据长度: " << header->dataLength << endl;
				//DataHeader ret;
				//SendData(cSock,&ret);
				break;
			}
		}
	}

	//发指定的数据
	int SendData(SOCKET cSock,DataHeader* header)
	{
		if (isRun() && header)
		{
			send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	//群发的数据
	void SendDataToAll( DataHeader* header)
	{
		//新的客户端还没有加入到vector之后，就先群发给所有其他成员
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}
	//关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
			for (int n = (int)_clients.size() - 1; n >= 0; n--)    //如果程序正常退出，该for语句是不会得到执行的
			{
#ifdef _WIN32
				closesocket(_clients[n]->sockfd());      //关闭
				delete _clients[n];	//我们new了之后 要记得关闭 不然会导致内存泄漏
#else
				close(_clients[n].sockfd() );
				delete _clients[n];
#endif
			}

			//6、关闭套接字
#ifdef _WIN32
			WSACleanup();
			closesocket(_sock);
#else
			close(_sock);
#endif
			_clients.clear();
		}
	}



};




#endif // !_EasyTcpServer_hpp_

