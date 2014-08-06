#include "Process.h"
#include "CUnit.h"

using namespace curr;

void test_create();

CU_TestInfo ProcessTests[] = {
  {"create process", test_create},
  CU_TEST_INFO_NULL
};

// init the test suite
int ProcessCUInit() 
{
  return 0;
}

// clean the test suite
int ProcessCUClean() 
{
  return 0;
}

namespace {

  auto rep = ChildProcessRepository::instance();

}

void test_create()
{
  Process* p1 = rep->create_object(Process::Par{ "ls /etc" });
}
