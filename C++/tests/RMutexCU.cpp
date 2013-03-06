#include "RMutex.h"
#include "CUnit.h"
#include "Logging.h"

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

