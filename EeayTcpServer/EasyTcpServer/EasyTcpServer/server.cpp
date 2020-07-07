#include "EsayTcpServer.hpp"

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		cin >> cmdBuf;
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
	//服务器初始化
	EasyTcpServer server;
	server.InitSocket();
	server.BindSocket(NULL,4567);
	server.Listen(5);

	std::thread t2(cmdThread);
	t2.detach();

	while (g_bRun)
	{
		server.OnRun();
	}

	server.Close();
	return 0;
}
