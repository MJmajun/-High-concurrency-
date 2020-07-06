#include "EsayTcpServer.hpp"

int main()
{
	//服务器初始化
	EasyTcpServer server;
	server.InitSocket();
	server.BindSocket(NULL,4567);
	server.Listen(5);
	
	while (server.isRun())
	{
		server.OnRun();
	}

	server.Close();
	return 0;
}
