#include "RBuffer.h"
#include "RWindow.h"
#include "tests.h"

void test_move_rbuffer();
void test_resize_rbuffer();
void test_cancel_charging();
void test_extend_bottom();
void test_extend_bottom_ovf();

CU_TestInfo RBufferTests[] = {
  {"resize RBuffer", test_resize_rbuffer},
  {"move RBuffer", test_move_rbuffer},
  {"cancel_charging", test_cancel_charging},
  {"extend_bottom", test_extend_bottom},
  {"extend_bottom overflow", test_extend_bottom_ovf},
  CU_TEST_INFO_NULL
};

// init the test suite
int RBufferCUInit() 
{
  return 0;
}

// clean the test suite
int RBufferCUClean() 
{
  return 0;
}

void test_move_rbuffer()
{
  char c[] = "12345678901";
  RSingleBuffer a(11, 11), b;
  a.reserve(11, 0);
  memcpy(a.data(), c, a.capacity());
  try {
    b.move(&a);
    CU_FAIL_FATAL("InvalidState exception wanted");
  }
  catch(const InvalidStateTransition& ex) {
    CU_ASSERT_EQUAL_FATAL
      (ex.from, RBuffer::chargingState);
    CU_ASSERT_EQUAL_FATAL
      (ex.to, RBuffer::moving_sourceState);
  }
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (a, RBuffer::chargingState));
  CU_ASSERT_EQUAL_FATAL(a.size(), 0);
  CU_ASSERT_EQUAL_FATAL(a.capacity(), 11);
  CU_ASSERT_PTR_NOT_NULL_FATAL(a.cdata());
  CU_ASSERT_NSTRING_EQUAL_FATAL(a.cdata(), c, 11);
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (b, RBuffer::dummyState));
  CU_ASSERT_EQUAL_FATAL(b.size(), 0);
  CU_ASSERT_EQUAL_FATAL(b.capacity(), 0);
  CU_ASSERT_PTR_NULL_FATAL(b.cdata());
  a.resize(10);
  b.move(&a);
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (a, RBuffer::dischargedState));
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (b, RBuffer::chargedState));
  CU_ASSERT_EQUAL_FATAL(b.size(), 10);
  CU_ASSERT_EQUAL_FATAL(b.capacity(), 11);
  CU_ASSERT_EQUAL_FATAL(a.size(), 0);
  CU_ASSERT_PTR_NULL_FATAL(a.cdata());
  CU_ASSERT_EQUAL_FATAL(a.capacity(), 11);
  CU_ASSERT_NSTRING_EQUAL_FATAL(b.cdata(), c, 10);
  b.clear();
}

void test_resize_rbuffer()
{
  char c[] = "1234567890";
  RSingleBuffer* b = new RSingleBuffer(10, 10);
  CU_ASSERT_EQUAL_FATAL(b->size(), 0);
  CU_ASSERT_EQUAL_FATAL(b->capacity(), 10);
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (*b, RBuffer::dischargedState));
  CU_ASSERT_PTR_NULL_FATAL(b->cdata());

  memcpy(b->data(), c, b->capacity());
  try {
    b->resize(15);
    CU_FAIL_FATAL("ResizeOverCapacity exception wanted");
  }
  catch(const RSingleBuffer::ResizeOverCapacity&) {}

  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (*b, RBuffer::chargingState));

  b->resize(5);
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (*b, RBuffer::chargedState));
  CU_ASSERT_EQUAL_FATAL(b->size(), 5);
  CU_ASSERT_EQUAL_FATAL(b->capacity(), 10);
  CU_ASSERT_PTR_NOT_NULL_FATAL(b->cdata());
  CU_ASSERT_NSTRING_EQUAL_FATAL(b->cdata(), c, 5);

  try {
    b->reserve(9, 0);
    CU_FAIL_FATAL("InvalidState exception wanted");
  } catch (const InvalidState&) {}

  b->resize(0);
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (*b, RBuffer::dischargedState));
  b->reserve(9, 0);
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (*b, RBuffer::dischargedState));
  CU_ASSERT_EQUAL_FATAL(b->size(), 0);
  CU_ASSERT_EQUAL_FATAL(b->capacity(), 9);
  CU_ASSERT_PTR_NULL_FATAL(b->cdata());

  try {
    b->resize(10);
    CU_FAIL_FATAL("ResizeOverCapacity exception wanted");
  }
  catch(const RSingleBuffer::ResizeOverCapacity&) {}

  try {
    b->resize(9);
    CU_FAIL_FATAL("InvalidState exception wanted");
  }
  catch(const InvalidState&) {}

  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (*b, RBuffer::dischargedState));
  b->start_charging();
  try {
    b->start_charging();
    CU_FAIL_FATAL
      ("InvalidStateTransition exception wanted");
  }
  catch(const InvalidStateTransition&) {}
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (*b, RBuffer::chargingState));
  CU_ASSERT_EQUAL_FATAL(b->size(), 0);
  CU_ASSERT_TRUE_FATAL (RBuffer::State::state_is
                        (*b, RBuffer::chargingState));
  b->resize(8);
  CU_ASSERT_EQUAL_FATAL(b->size(), 8);
  try {
    b->resize(8);
    CU_FAIL_FATAL("InvalidState exception wanted");
  }
  catch(const InvalidState&) {}
  CU_ASSERT_EQUAL_FATAL(b->size(), 8);
  CU_ASSERT_EQUAL_FATAL(b->capacity(), 9);
  CU_ASSERT_PTR_NOT_NULL_FATAL(b->cdata());
  try {
    b->data();
    CU_FAIL_FATAL
      ("InvalidStateTransition exception wanted");
  }
  catch(const InvalidStateTransition&) {}

  b->clear();
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
                       (*b, RBuffer::dischargedState));
  b->start_charging();  
  b->resize(1);
#if 0
  // it will holds forever
  delete b;
#endif
  b->resize(0);
  delete b;
}

void test_cancel_charging()
{
  RSingleBuffer a(10, 10);
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RSingleBuffer, state_is, a, discharged));
  a.data();
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RSingleBuffer, state_is, a, charging));
  a.cancel_charging();
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RSingleBuffer, state_is, a, discharged));
}

void test_extend_bottom()
{
  std::unique_ptr<RSingleBuffer> a  
    (new RSingleBuffer(10, 5));
  std::unique_ptr<RSingleBuffer> b 
    (new RSingleBuffer(10, 5));

  memcpy(a->data(), "12345", 5);
  memcpy(b->data(), "67890", 5);
  a->resize(5); b->resize(5);
  RConnectedWindow w(nullptr);
  w.forward_top(5);
  a->set_autoclear(true);
  w.new_buffer(std::move(a));
  b->extend_bottom(w);
  CU_ASSERT_NSTRING_EQUAL_FATAL(
    (const char*)b->cdata() - 5, "1234567890", 10);
  RWindow().move(w);
  b->resize(0);
}

void test_extend_bottom_ovf()
{
  std::unique_ptr<RSingleBuffer> a  
    (new RSingleBuffer(10, 5));
  std::unique_ptr<RSingleBuffer> b 
    (new RSingleBuffer(10, 4));

  memcpy(a->data(), "12345", 5);
  memcpy(b->data(), "67890", 5);
  a->resize(5); b->resize(5);
  RConnectedWindow w(nullptr);
  w.forward_top(5);
  a->set_autoclear(true);
  w.new_buffer(std::move(a));
  try {
    b->extend_bottom(w);
    CU_FAIL_FATAL("No exception.");
  }
  catch (const SException&) {
  }
  CU_ASSERT_NSTRING_EQUAL_FATAL(
    (const char*)b->cdata(), "67890", 5);
  RWindow().move(w);
  b->resize(0);
}


