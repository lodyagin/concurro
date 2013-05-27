#include "SCommon.h"
#include "tests.h"

void test_cast();
void test_cast_exception();


CU_TestInfo CommonTests[] = {
  {"test toString", test_cast},
  {"test throw exception when cast not  a number",
      test_cast_exception},
  CU_TEST_INFO_NULL
};

// init the test suite
int SCommonCUInit()
{
  return 0;
}

// clean the test suite
int SCommonCUClean()
{
  return 0;
}

void test_cast(){
  //bool c = 5 == toString<int>(std::string("5"))
  //CU_ASSERT_TRUE_FATAL(c);
}

void test_cast_exception(){

}
