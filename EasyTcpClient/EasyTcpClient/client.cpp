#include <thread>
#include "EasyTcpClient.hpp"

int main()
{
	EasyTcpClient client1;
	client1.InitSoket();	//再连接的过程中可以初始化，这里也可以不写

	client1.Connect("192.168.1.188",4567);//140.143.12.171
	//std::thread t1(cmdThread, &client1);       //第一个是函数名 第二个就是要传入的参数
	//t1.detach();            //和主线程进行分离  一定要进行分离，不然子线程的退出会直接导致主线程也退出，但是，主线程没有正常结束程序，就会产生问题
	Login login;
	strcpy(login.userName,"majun majun majun");
	strcpy(login.passWord,"0123456789");
	while (client1.isRun())
	{
		client1.SendData(&login);
		client1.OnRun();
	}
	client1.Close();
	cout << "已经退出" << endl;
	getchar();
	return 0;
}
