#include <iostream>
#include <thread>
#include <mutex>	//锁
#include <atomic>	//原子操作
#include "CELLTimestamp.hpp"
using namespace std;

mutex m;
const int tCount = 4;
atomic<int> sum = 0;	//atomic_int sum = 0;	//这sum作为公共的资源 我们给他原子操作，这样就不用上锁了，他本身就已经被上了锁
void workFunc(int index)
{
	for (int i = 0; i<20000000; i++)
	{
		//lock_guard<mutex> lg(m);	//自解锁  就是 再构造的时候自己加锁  析构的时候解锁
		//m.lock();	//临界区域-开始
		sum++;
		//m.unlock();	//临界区域-结束
	}
	//cout << "index = " << index << "  other Tread  i=" << i << endl;
}

int main()
{	
	thread t[tCount];
	for (int i= 0 ;i< tCount;i++)
	{
		t[i] = thread(workFunc,i);	
	}
	CELLTimestamp tTime;
	for (int i = 0; i< tCount; i++)
	{	
		t[i].join();
	}
	double t1 = tTime.getElapsedTimeInMilliSec() ;
	printf("t1 = %f",t1);
	cout << "  Main Thread, sum = "<<sum<< endl;
	getchar();
	return 0;
}

//thread t1(workFunc,);
//t1.detach();	//detach 会将子线程和主线程分开，那主线程跑完之后 就会结束掉所有子线程
//t1.join();	//join 会将子线程加入到主线程中，然后主线程会等待子线程跑完再运行！！！

//多线程中，不要频繁的去操作加锁和解锁  这样会消耗很多时间
//lock_guard<mutex> lg(m);	//自解锁  就是 再构造的时候自己加锁  析构的时候解锁


//来历： 我们要多线程，线程无法随意控制 -> 加锁使得线程之间有序->频繁加锁，导致时间消耗过多
//->采用自旋锁来解决->更为优化的方法就是->原子操作

//atomic<int> sum = 0;	//atomic_int sum = 0;	//这sum作为公共的资源 我们给他原子操作，这样就不用上锁了，他本身就已经被上了锁
