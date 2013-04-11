#include "RThread.h"
#include "CUnit.h"

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
	 T() : RT("test_local_block::T") {} 
	 void run() { sleep(1000); }
  } t1;

  t1.start();
}

