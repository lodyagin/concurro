#include "SCommon.h"
#include "tests.h"
#include "SException.h"

void test_from_string();
void test_cast_exception();
void test_sformat();

CU_TestInfo SCommonTests[] = {
  {"a cast test", test_from_string},
  {"a cast exception test", test_cast_exception},
  {"sformat test", test_sformat},
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
  } catch (const BadCastBase&) {
    goal = true;
  }
  CU_ASSERT_TRUE_FATAL(goal);
  goal = false;
  try {
    fromString<float>("yh");
  } catch (const SException&) {
    goal = true;
  }
  CU_ASSERT_TRUE_FATAL(goal);
  goal = false;
  try {
    fromString<char>("yh");
  } catch (const BadCast<char,const char (&)[3]>& bc) 
  {
    CU_ASSERT_EQUAL_FATAL(bc.source, std::string("yh"));
    goal = true;
  }
  CU_ASSERT_TRUE_FATAL(goal);
}

void test_sformat()
{
  CU_ASSERT_EQUAL_FATAL(
    sformat("ab", 4, 'd', "g", "c1"),
    "ab4dgc1"
  );
}
