//#include "MessageHeader.hpp"
#include "EsayTcpServer.hpp"

int main()
{
	EasyTcpServer server;
	server.InitSocket();
	server.BindSocket(NULL,4567);
	server.Listen(5);
	

	while (server.isRun())
	{
		server.OnRun();
	}

	server.Close();
	cout<<"ÒÑÍË³ö"<<endl;
	getchar();
	return 0;
}
