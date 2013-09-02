#include "RWindow.hpp"
#include "tests.h"

void test_rconnectedwindow();
void test_extend_bottom_w();

CU_TestInfo RWindowTests[] = {
  {"RConnectedWindow", test_rconnectedwindow},
  {"bottom extending", test_extend_bottom_w},
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
  RConnectedWindow<int>* w = RConnectedWindow<int>::create(1);
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, *w, ready));
  w->forward_top(5);
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, *w, 
             wait_for_buffer));
  w->new_buffer(std::move(a));
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, *w, 
             wait_for_buffer));
  a.reset(new RSingleBuffer(10, 5));
  a->set_autoclear(true);
  memcpy(a->data(), "5", 1);
  a->resize(1);
  w->new_buffer(std::move(a));
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, *w, filled));
  CU_ASSERT_EQUAL_FATAL(w->filled_size(), 5);
  CU_ASSERT_NSTRING_EQUAL_FATAL(&(*w)[0], "12345", 5);
}

void test_extend_bottom_w()
{
  std::unique_ptr<RSingleBuffer> a1
    (new RSingleBuffer(10, 1));
  memcpy(a1->data(), "123", 3);
  a1->resize(3); a1->set_autoclear(true);


  std::unique_ptr<RSingleBuffer> a2
    (new RSingleBuffer(10, 3));
  memcpy(a2->data(), "4567", 4);
  a2->resize(4); a2->set_autoclear(true);

  std::unique_ptr<RSingleBuffer> a3
    (new RSingleBuffer(10, 7));
  memcpy(a3->data(), "8", 1);
  a3->resize(1); a3->set_autoclear(true);

  std::unique_ptr<RSingleBuffer> a4
    (new RSingleBuffer(2, 8));
  memcpy(a4->data(), "90", 3);
  a4->resize(2); a4->set_autoclear(true);

  auto* w = RConnectedWindow<int>::create(2);
  w->forward_top(10);
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, *w, 
              wait_for_buffer));
  w->new_buffer(std::move(a1));
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, *w, 
              wait_for_buffer));
  w->new_buffer(std::move(a2));
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, *w, 
              wait_for_buffer));
  w->new_buffer(std::move(a3));
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, *w, 
              wait_for_buffer));
  w->new_buffer(std::move(a4));
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, *w, 
              filled));
  CU_ASSERT_NSTRING_EQUAL_FATAL(&(*w)[0], "1234567890", 10);
}
