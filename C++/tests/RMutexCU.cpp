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
void test_local_thread_variable();

CU_TestInfo RMutexTests[] = {
	{"RMutex can be acquired by the same thread several times",
	 test_same_thread_acquire },
	{"RMutex acquire in 1st thread and 2nd thread can't acquire it at same time",
	 	 test_2_threads_try_acquire },
	{"we can't leave from local region while thread is run",
		test_local_thread_variable },
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
  RMutex mx("test_same_thread_acquire");

  MUTEX_ACQUIRE(mx);;
  MUTEX_ACQUIRE(mx);;
  MUTEX_RELEASE(mx);;
  MUTEX_RELEASE(mx);;
}

void test_same_thread_overrelease()
{
  RMutex mx("test_same_thread_overrelease");

  MUTEX_ACQUIRE(mx);;
  MUTEX_ACQUIRE(mx);;
  MUTEX_RELEASE(mx);;
  MUTEX_RELEASE(mx);;
  MUTEX_RELEASE(mx);;
}

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
		MUTEX_ACQUIRE(mx);
		sleep(sleept);
		arg = ++test;
		MUTEX_RELEASE(mx);
  }
	volatile int sleept;
	volatile int arg;
	RMutex& mx;
};

void test_2_threads_try_acquire(){
  RMutex mx("mx");
	int id1 = 2, id2 = 0;
	std::string s = "11111";
	std::string s1 = "22222";
	TestThread thread1(s, mx, id1);
	thread1.start();
	sleep(1);
	TestThread thread2(s1, mx, id2);
	thread2.start();
	sleep(3);
	if (thread1.getResuilt() == 1 && thread2.getResuilt() == 2)
		CU_PASS(id1 == 1)
	else
		CU_FAIL(id1! = 1);
}


class TestThread1 : public RThread<std::thread> {
public:
	TestThread1(const std::string& id,  volatile bool*flg)
   :RThread<std::thread>(id), flag(flg){}
protected:
	void run(){
		sleep(3);
		*flag = true;
  }
	volatile bool * flag;
};
void local(volatile bool *b){
	TestThread1 thread1(std::string("111122"),b);
	thread1.start();
}
void test_local_thread_variable(){
	volatile bool * b = false;
	local(b);
	if(b)CU_PASS(*b) else CU_FAIL(*b);
}