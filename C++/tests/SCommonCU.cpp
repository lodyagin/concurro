#include "SCommon.h"
#include "tests.h"
#include "SException.h"

void test_from_string();
void test_cast_exception();
CU_TestInfo SCommonTests[] = {
  {"a cast test",
   test_from_string},
  {"a cast exception test",
   test_cast_exception},
  CU_TEST_INFO_NULL
};

// init the test suite
int SCommoCUInit()
{
  return 0;
}

// clean the test suite
int SCommoCUClean()
{
  return 0;
}

void test_from_string() {
  CU_ASSERT_TRUE_FATAL(5.1 == fromString<double>("5.1"));
  std::string s = "2.1";
  float f = fromString<double>(s);
  CU_ASSERT_TRUE_FATAL(fabs(2.1 - f) < 0.001);
  CU_ASSERT_TRUE_FATAL(5 == fromString<int>("5"));
  CU_ASSERT_TRUE_FATAL(fabs(0.00 -
      fromString<double>(toString(0.00))) < 0.0001);
  CU_ASSERT_TRUE_FATAL("5" == toString(5));
}
void test_cast_exception(){
  bool goal = false;
  try {
    fromString<int>("yh");
  } catch (boost::bad_lexical_cast) {
    goal = true;
  }
  CU_ASSERT_TRUE_FATAL(goal);
  goal = false;
  try {
    fromString<float>("yh");
  } catch (SException) {
    goal = true;
  }
  CU_ASSERT_TRUE_FATAL(goal);
  goal = false;
  try {
    fromString<char>("yh");
  } catch (FromStringCastException) {
    goal = true;
  }
  CU_ASSERT_TRUE_FATAL(goal);
}
