#include <thread>
#include "EasyTcpClient.hpp"


bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		std::cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			cout << "退出线程" << endl;
			break;
		}
	}
}

int main()
{
	const int count = 1;//FD_SETSIZE-1;
	EasyTcpClient* client[count];

	for (int i = 0; i< count;i++)
	{
		client[i] = new EasyTcpClient();
		client[i]->Connect("140.143.12.171", 4567);//140.143.12.171
	}
	

	std::thread t1(cmdThread);       //第一个是函数名 第二个就是要传入的参数
	t1.detach();

          //和主线程进行分离  一定要进行分离，不然子线程的退出会直接导致主线程也退出，但是，主线程没有正常结束程序，就会产生问题
	Login login;
	strcpy(login.userName,"majun majun majun");
	strcpy(login.passWord,"0123456789");
	while (g_bRun)
	{
		for (int i = 0; i <count;i++)
		{
			client[i]->SendData(&login);
			client[i]->OnRun();
		}

	}
	for (int i = 0; i <count; i++)
	{
		client[i]->Close();
	}

	cout << "已经退出" << endl;
	getchar();
	return 0;
}
