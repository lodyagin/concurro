#include "RMutex.h"
#include "CUnit.h"
#include "pthread.h"
#include <iostream>
//#include <iostream>
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
void* threadFunc(void *arg){
	mx1.acquire();
	static volatile int test = 0;
	sleep(*(int *)arg);
	*(int*)arg = ++test;
	mx1.release();
}
void test_2_threads_try_acquire(){
	pthread_t thread1, thread2;
	int id1 = 2, id2 = 0;
	pthread_create(&thread1, NULL, threadFunc, &id1);
	sleep(1);
	pthread_create(&thread2, NULL, threadFunc, &id2);
	sleep(3);
	if (id1 == 1 && id2 == 2)
		CU_PASS(id1 == 1)
	else
		CU_FAIL(id1! = 1);
}

