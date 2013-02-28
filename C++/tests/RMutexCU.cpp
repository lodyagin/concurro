#include "RMutex.h"
#include "CUnit.h"

//#include <iostream>
//#include "concurrent/C++/REvent.h"
//#include <boost/thread/thread.hpp>

//using namespace std;
//using namespace boost;

void test_same_thread_acquire();
void test_same_thread_overrelease();

CU_TestInfo RMutexTests[] = {
	{"RMutex can be acquired by the same thread several times",
	 test_same_thread_acquire },
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

