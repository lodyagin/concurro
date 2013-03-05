#include "RMutex.h"
#include "CUnit.h"
#include "RThread.h"
#include <iostream>
#include <string>
#include <thread>
//#include "concurrent/C++/REvent.h"
//#include <boost/thread/thread.hpp>

//using namespace std;
//using namespace boost;

void test_same_thread_acquire();
void test_same_thread_overrelease();
void test_2_threads_try_acquire();

CU_TestInfo RMutexTests[] = {
	{"RMutex can be acquired by the same thread several times",
	 test_same_thread_acquire },
	 {"RMutex acquire in 1st thread and 2nd thread can't acquire it at same time",
	 	 test_2_threads_try_acquire },
	 	{"RMutex more releases than acquire in the same thread",
	 		 test_same_thread_overrelease },
	CU_TEST_INFO_NULL
};

// init the test suite
int RMutexCUInit() 
{
  return 0;
}

// clean the test suite
int RMutexCUClean() 
{
  return 0;
}

void test_same_thread_acquire()
{
  RMutex mx;

  mx.acquire();
  mx.acquire();
  mx.release();
  mx.release();
}

void test_same_thread_overrelease()
{
  RMutex mx;

  mx.acquire();
  mx.acquire();
  mx.release();
  mx.release();
  mx.release();
}
RMutex mx1;
class TestThread : public RThread<std::thread> {
public:
	TestThread(const std::string& id,
					   RMutex& mutex, int sleep_time)
   :RThread<std::thread>(id), mx(mutex), sleept(sleep_time)
{}
	int getResuilt(){return arg;}
protected:
	void run(){
		static volatile int test=0;
		mx.acquire();
		sleep(sleept);
		arg = ++test;
		std::cout << "\n\n\n\n  "<<test<< "\n\n\n\n";
		mx.release();
  }
	volatile int sleept;
	volatile int arg;
	RMutex& mx;
};

void test_2_threads_try_acquire(){
	RMutex mx;
	int id1 = 2, id2 = 0;
	std::string s = "11111";
	std::string s1 = "22222";
	TestThread thread1(s, mx, id1);
	thread1.start();
	sleep(1);
	TestThread thread2(s1, mx, id2);
	thread2.start();
	sleep(3);
	std::cout << "\n\n\n\n  "<<thread1.getResuilt()<< "\n\n\n\n";
	std::cout << "\n\n\n\n  "<<thread2.getResuilt()<< "\n\n\n\n";

	if (thread1.getResuilt() == 1 && thread2.getResuilt() == 2)
		CU_PASS(id1 == 1)
	else
		CU_FAIL(id1! = 1);
}

