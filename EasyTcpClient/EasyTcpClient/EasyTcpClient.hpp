#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS

	#include <WinSock2.h>
	#include <windows.h>
	#pragma comment(lib,"ws2_32.lib")       //��������  ������ͨ�õķ��� �Ѿ���������������
#else
	#include <unistd.h>             //unix�ı�׼��
	#include <arpa/inet.h>
	#include <string.h>

	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR (-1)
#endif

#include <iostream>
using namespace std;
#include "MessageHeader.hpp"
#define RECV_BUFF_SIZE 10240

class EasyTcpClient
{
private:
	SOCKET _sock;
	int _count=0;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//��ʼ��socket
	void InitSoket()
	{
		//����window socket �Ļ���
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//1������һ���׽���
		if (INVALID_SOCKET != _sock)
		{
			cout << "Socket = "<<_sock <<" �������Ѿ��ر�"<<endl;
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			cout<<"���󣬽���Socket = "<<_sock<<" ʧ��..."<<endl;
		}
		else
		{
			cout<< "����Socket = " << _sock << "�ɹ�..." << endl;
		}
	}

	//���ӷ�����
	int Connect(const char* ip,unsigned short port)
	{
		//2�����ӷ����� connect
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
			cout << "Socket = "<<_sock<<" ���ӷ�����<"<< ip <<" : "<<port<<" > ʧ��..." << endl;
		}
		else
		{
			cout <<"Socket = "<<_sock<<" ���ӷ�����<" << ip << " : " << port << " >�ɹ�..." << endl;
		}
		return ret;
	}

	//������

	//������
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
		
	}

	//��ѯ����
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdRead;
			FD_ZERO(&fdRead);
			FD_SET(_sock, &fdRead);

			timeval t = { 0,0 };//ǰ������ �����Ǻ���       ��selectÿ��1��ȥɨ��һ��
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			//cout <<"ret = "<< ret <<"select_count =" <<_count++ << endl;

			if (ret < 0)
			{
				Close();
				cout << "Socket = " << _sock<< " select �������2"<<endl;
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				if (-1 == RecvData(_sock))
				{
					cout << "Socket = " << _sock<< " select �������2"<<endl;
					Close();
					return false;
				}
			}		
		}
		return true;	
	}

	//�ж��Ƿ�������
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//�����ݾ�   ����ճ���Ͳ��
	int _lastPos = 0;
	int RecvData(SOCKET _cSock)
	{
		//������
		char _szRecv[RECV_BUFF_SIZE] = {};
		//�ڶ���������
		char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
		//int len = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		int len = recv(_cSock, _szRecv, RECV_BUFF_SIZE, 0);
		//cout << "len = " << len << endl;

		DataHeader* header = (DataHeader*)_szRecv;
		if (len <= 0)
		{
			cout<< "Socket = " <<_cSock<<" ��������Ͽ����ӣ��������"<<endl;
			return -1;
		}
		//����ȡ�������ݿ�������Ϣ������
		::memcpy(_szMsgBuf + _lastPos,_szRecv,len);
		//����Ϣ������������β��λ�ú���
		_lastPos += len;
		//ѭ���ж���Ϣ�������ĳ����Ƿ������Ϣ����
		while(_lastPos >= sizeof(DataHeader))	//���ճ��
		{
			//��ʱ��Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)_szMsgBuf;
			//�ж���Ϣ�������ĳ��ȴ�����Ϣ����
			if (_lastPos >= header->dataLength)		//����ٰ�
			{
				//��Ϣ������ʣ��δ���������ݵĳ���
				int nSize = _lastPos - header->dataLength;

				//����������Ϣ
				OnNetMsg(header);
				//����Ϣ������ʣ�������ǰ��
				::memcpy(_szMsgBuf,_szMsgBuf+header->dataLength,nSize);
				//��Ϣ������������β����ǰ�ƶ�
				_lastPos = nSize;
			}
			else 
			{
				//ʣ����Ϣ����һ������Ϣ
				break;
			}
		}
		return 0;
	}

	//��Ӧ����
	void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
			{
				LoginResult *login = (LoginResult*)header;
				//cout<<"�յ�����������Ϣ��CMD_LOGIN_RESULT, ���ݳ���:"<< login->dataLength<<endl;

				break;
			}
			case CMD_LOGOUT_RESULT:
			{
				LogoutResult *logout = (LogoutResult*)header;
				//cout << "�յ�����������Ϣ��CMD_LOGOUT_RESULT, ���ݳ���:" << logout->dataLength << endl;
				break;
			}
			case CMD_NEW_USER_JOIN:
			{
				NewUserJoin *userjoin = (NewUserJoin*)header;
				//cout << "�յ�����������Ϣ��CMD_NEW_USER_JOIN, ���ݳ���:"<< userjoin->dataLength<<endl;
				break;
			}
			case CMD_ERROR:
			{
				cout << "�յ�����������Ϣ��CMD_ERROR,���ݳ��ȣ�" << header->dataLength << endl;
				break;
			}
			default:
			{
				cout << "�յ�������δ�������Ϣ ���ݳ���: " << header->dataLength << endl;
			}
		}
	}

	//�ر�socket
	void Close()
	{
		//��ֹ�ظ�����
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
			cout << "�˳��߳�" << endl;
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
			cout << "��֧�ֵ�����" << endl;
		}
	}
}


#endif // !_EasyTcpClient_hpp_
