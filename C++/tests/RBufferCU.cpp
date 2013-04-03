#include "RBuffer.h"
#include "CUnit.h"

//void test_move_rbuffer();
void test_resize_rbuffer();

CU_TestInfo RBufferTests[] = {
//  {"move RBuffer", test_move_rbuffer},
  {"resize RBuffer", test_resize_rbuffer},
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

#if 0
static void discharge(RSingleBuffer& b)
{
  b.resize(0);
  CU_ASSERT_TRUE_FATAL
	 (RBuffer::State::state_is
	  (b, RBuffer::dischargedState));
}
#endif

void test_resize_rbuffer()
{
  char c[] = "1234567890";
  RSingleBuffer* b = new RSingleBuffer(10);
  CU_ASSERT_EQUAL_FATAL(b->size(), 0);
  CU_ASSERT_EQUAL_FATAL(b->capacity(), 10);
  CU_ASSERT_PTR_NOT_NULL_FATAL(b->data());
  CU_ASSERT_TRUE_FATAL
	 (RBuffer::State::state_is
	  (*b, RBuffer::dischargedState));

  memcpy(b->data(), c, b->capacity());
  try {
	 b->resize(15);
	 CU_FAIL_FATAL("ResizeOverCapacity exception wanted");
  }
  catch(const RSingleBuffer::ResizeOverCapacity&) {}

  b->resize(5);
  CU_ASSERT_TRUE_FATAL
	 (RBuffer::State::state_is
	  (*b, RBuffer::chargedState));
  CU_ASSERT_EQUAL_FATAL(b->size(), 5);
  CU_ASSERT_EQUAL_FATAL(b->capacity(), 10);
  CU_ASSERT_PTR_NOT_NULL_FATAL(b->data());
  CU_ASSERT_NSTRING_EQUAL_FATAL(b->data(), c, 5);

  b->resize(0);
  CU_ASSERT_TRUE_FATAL
	 (RBuffer::State::state_is
	  (*b, RBuffer::dischargedState));
  CU_ASSERT_EQUAL_FATAL(b->size(), 0);
  CU_ASSERT_EQUAL_FATAL(b->capacity(), 10);
  CU_ASSERT_PTR_NOT_NULL_FATAL(b->data());

  try {
	 b->resize(11);
	 CU_FAIL_FATAL("ResizeOverCapacity exception wanted");
  }
  catch(const RSingleBuffer::ResizeOverCapacity&) {}

  b->resize(10);
  CU_ASSERT_TRUE_FATAL
	 (RBuffer::State::state_is
	  (*b, RBuffer::chargedState));
  CU_ASSERT_EQUAL_FATAL(b->size(), 10);
  CU_ASSERT_EQUAL_FATAL(b->capacity(), 10);
  CU_ASSERT_PTR_NOT_NULL_FATAL(b->data());
  CU_ASSERT_NSTRING_EQUAL_FATAL(b->data(), c, 10);
  try {
	 delete b;
	 CU_FAIL_FATAL("Bad state transition exception wntd");
  }
  catch (const InvalidStateTransition&) {}
}

