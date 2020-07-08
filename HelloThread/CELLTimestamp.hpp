#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

//#include <windows.h>			//以下的所有注释 都是windos很老的一套 获取时间   我们这里直接使用的c++11中  
#include <chrono>
using namespace std::chrono;

class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		//QueryPerformanceFrequency(&_frequency);
		//QueryPerformanceCounter(&_startCount);
		updata();
	}
	~CELLTimestamp()
	{

	}

	void  updata()
	{
		/*QueryPerformanceCounter(&_startCount);*/
		_begin = high_resolution_clock::now();	//更新当前时间
	}


	/*获取秒*/
	float getElapsedSecond()
	{
		return getElapsedTimeInMicroSec() * 0.000001;
	}

	/*获取毫秒*/
	float getElapsedTimeInMilliSec()
	{
		return this->getElapsedTimeInMicroSec() * 0.001;
	}

	/*获取微秒*/
	long long getElapsedTimeInMicroSec()
	{
		//LARGE_INTEGER endCount;
		//QueryPerformanceCounter(&endCount);

		//double startTTimeInMicroSec = _startCount.QuadPart * (1000000.0 / _frequency.QuadPart);
		//double endTimeInMicroSec = endCount.QuadPart * (1000000.0 / _frequency.QuadPart);

		//return endTimeInMicroSec - startTTimeInMicroSec;

		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}

private:

protected:
	//LARGE_INTEGER _frequency;
	//LARGE_INTEGER _startCount;
	time_point<high_resolution_clock> _begin;
};

#endif // !_CELLTimestamp_hpp_

