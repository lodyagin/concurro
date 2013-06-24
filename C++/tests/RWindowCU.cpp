#include "RWindow.h"
#include "tests.h"

void test_rconnectedwindow();

CU_TestInfo RWindowTests[] = {
  {"RConnectedWindow", test_rconnectedwindow},
  CU_TEST_INFO_NULL
};

// init the test suite
int RWindowCUInit() 
{
  return 0;
}

// clean the test suite
int RWindowCUClean() 
{
  return 0;
}

void test_rconnectedwindow()
{
  std::unique_ptr<RSingleBuffer> a  
    (new RSingleBuffer(10, 5));
  a->set_autoclear(true);

  memcpy(a->data(), "1234", 5);
  a->resize(4);
  RConnectedWindow w(nullptr);
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow, state_is, w, ready));
  w.forward_top(5);
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow, state_is, w, 
             wait_for_buffer));
  w.new_buffer(std::move(a));
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow, state_is, w, 
             wait_for_buffer));
  a.reset(new RSingleBuffer(10, 5));
  a->set_autoclear(true);
  memcpy(a->data(), "5", 1);
  a->resize(1);
  w.new_buffer(std::move(a));
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow, state_is, w, filled));
  CU_ASSERT_EQUAL_FATAL(w.filled_size(), 5);
  CU_ASSERT_NSTRING_EQUAL_FATAL(&w[0], "12345", 5);
  RWindow().move(w);
}

