#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _CRT_SECURE_NO_DEPRECATE
	
	#include <WinSock2.h>
	#include <windows.h>
	#pragma comment(lib,"ws2_32.lib")	//解决库调用  我们用通用的方法 已经在属性中添加了
#else
	#include <unistd.h>             //unix的标准库
	#include <arpa/inet.h>
	#include <string.h>

	//解决宏定义 
	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR (-1)
#endif

#include <thread>
#include "EasyTcpClient.hpp"

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
			cout<<"退出线程"<<endl;
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
			cout<<"不支持的命令"<<endl;
		}
	}
}

int main()
{
	EasyTcpClient client1;
	EasyTcpClient client2;
	//client.InitSoket();	//再连接的过程中可以初始化，这里也可以不写

	client1.Connect("192.168.1.188",4567);
	client2.Connect("192.168.1.50",4567);
	std::thread t1(cmdThread, &client1);       //第一个是函数名 第二个就是要传入的参数
	t1.detach();            //和主线程进行分离  一定要进行分离，不然子线程的退出会直接导致主线程也退出，但是，主线程没有正常结束程序，就会产生问题

	std::thread t2(cmdThread,&client2);
	t2.detach();

	while (client1.isRun()|| client2.isRun())
	{
		client1.OnRun();
		client2.OnRun();
	}
	client1.Close();
	client2.Close();
	cout << "已经退出" << endl;
	getchar();
	return 0;
}
