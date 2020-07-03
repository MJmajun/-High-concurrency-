#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <windows.h>
//#pragma comment(lib,"ws2_32.lib");	//解决库调用  我们用通用的方法 已经在属性中添加了
int main()
{
	//启动window socket 环境
	WORD ver = MAKEWORD(2,2);
	WSADATA dat;
	WSAStartup(ver,&dat);


	WSACleanup();
	return 0;
}