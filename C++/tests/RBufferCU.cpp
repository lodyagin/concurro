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
  CU_ASSERT_PTR_NULL_FATAL(b->cdata());
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
							  (*b, RBuffer::dischargedState));

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
	 b->reserve(9);
  } catch (const InvalidState&) {}

  b->resize(0);
  CU_ASSERT_TRUE_FATAL(RBuffer::State::state_is
							  (*b, RBuffer::dischargedState));
  b->reserve(9);
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

