#include "RThread.h"
#include "CUnit.h"
#include <thread>

void test_local_block();

CU_TestInfo RThreadTests[] = {
  {"a working thread must prevent exiting from a local block",
	test_local_block},
  CU_TEST_INFO_NULL
};

// init the test suite
int RThreadCUInit() 
{
  return 0;
}

// clean the test suite
int RThreadCUClean() 
{
  return 0;
}

typedef RThread<std::thread> RT;

void test_local_block()
{
  struct T : public RT 
  { 
	 typedef Logger<T> log;

	 T() : RT("test_local_block::T") {} 
	 void run() { 
		LOG_DEBUG(log, "Enter run()");
		std::this_thread::sleep_for
		  (std::chrono::hours::max());
		LOG_DEBUG(log, "Exit run()");
	 }
  } t1;

  t1.start();
  std::this_thread::sleep_for
		  (std::chrono::hours::max());
}

