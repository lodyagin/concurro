#include "Event.h"
#include "tests.h"
#include "RThread.h"

void test_manual_reset();
void test_auto_reset();
void test_wait_for_any();

CU_TestInfo EventTests[] = {
  {"manual reset", test_manual_reset},
  {"auto reset", test_auto_reset},
  {"wait for any", test_wait_for_any},
  CU_TEST_INFO_NULL
};

// init the test suite
int EventCUInit() 
{
  return 0;
}

// clean the test suite
int EventCUClean() 
{
  return 0;
}

#if 0
class TestThread : public RT
{
public:
  TestThread(CompoundEvent&& e) 
	 : RT("TestThread"),
		ce(std::move(e)) {}

  ~TestThread() { destroy() };

  void run()
  {
	 RThreadState::move_to(*this, workingState);
	 ce.wait();
  }

  CompoundEvent ce;
};
#endif

#define TAU 100

void test_manual_reset()
{
  Event e1("e1", true, false);
  CU_ASSERT_FALSE_FATAL(e1.signalled());
  CU_ASSERT_FALSE_FATAL(e1.wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL(e1.signalled());
  CU_ASSERT_TRUE_FATAL(e1.wait(TAU));
  CU_ASSERT_TRUE_FATAL(e1.signalled());
  CU_ASSERT_TRUE_FATAL(e1.wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL(e1.signalled());
  CU_ASSERT_TRUE_FATAL(e1.wait(TAU));
  e1.reset();
  CU_ASSERT_FALSE_FATAL(e1.signalled());
  CU_ASSERT_FALSE_FATAL(e1.wait(TAU));
  CU_ASSERT_FALSE_FATAL(e1.signalled());
  CU_ASSERT_FALSE_FATAL(e1.wait(TAU));
  e1.reset();
  CU_ASSERT_FALSE_FATAL(e1.signalled());
  CU_ASSERT_FALSE_FATAL(e1.wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL(e1.signalled());
  CU_ASSERT_TRUE_FATAL(e1.wait(TAU));

  Event e2("e2", true, true);
  CU_ASSERT_TRUE_FATAL(e2.signalled());
  CU_ASSERT_TRUE_FATAL(e2.wait(TAU));
  CU_ASSERT_TRUE_FATAL(e2.signalled());
  CU_ASSERT_TRUE_FATAL(e2.wait(TAU));
  e2.set();
  CU_ASSERT_TRUE_FATAL(e2.signalled());
  CU_ASSERT_TRUE_FATAL(e2.wait(TAU));
  e2.reset();
  CU_ASSERT_FALSE_FATAL(e2.signalled());
  CU_ASSERT_FALSE_FATAL(e2.wait(TAU));
  CU_ASSERT_FALSE_FATAL(e2.signalled());
  CU_ASSERT_FALSE_FATAL(e2.wait(TAU));
  e2.reset();
  CU_ASSERT_FALSE_FATAL(e2.signalled());
  CU_ASSERT_FALSE_FATAL(e2.wait(TAU));
  e2.set();
  CU_ASSERT_TRUE_FATAL(e2.signalled());
  CU_ASSERT_TRUE_FATAL(e2.wait(TAU));
}

void test_auto_reset()
{
  Event e1("e1", false, false);
  CU_ASSERT_FALSE_FATAL(e1.wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL(e1.wait(TAU));
  CU_ASSERT_FALSE_FATAL(e1.wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL(e1.wait(TAU));
  e1.reset();
  CU_ASSERT_FALSE_FATAL(e1.wait(TAU));
  CU_ASSERT_FALSE_FATAL(e1.wait(TAU));
  e1.reset();
  CU_ASSERT_FALSE_FATAL(e1.wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL(e1.wait(TAU));

  Event e2("e2", false, true);
  CU_ASSERT_TRUE_FATAL(e2.wait(TAU));
  CU_ASSERT_FALSE_FATAL(e2.wait(TAU));
  e2.set();
  CU_ASSERT_TRUE_FATAL(e2.wait(TAU));
  e2.reset();
  CU_ASSERT_FALSE_FATAL(e2.wait(TAU));
  CU_ASSERT_FALSE_FATAL(e2.wait(TAU));
  e2.reset();
  CU_ASSERT_FALSE_FATAL(e2.wait(TAU));
  e2.set();
  CU_ASSERT_TRUE_FATAL(e2.wait(TAU));
}

void test_wait_for_any()
{
  Event e1("e1", false);
  CompoundEvent ce1(e1);
  CU_ASSERT_FALSE_FATAL(ce1.wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL(ce1.wait(TAU));
  Event e2("e2", false);
  ce1 |= e2;
  CU_ASSERT_FALSE_FATAL(ce1.wait(TAU));
  e2.set();
  CU_ASSERT_TRUE_FATAL(ce1.wait(TAU));

  Event e5("e5", true);
#if 0
  const Event& e5r = e5;
#endif
  const CompoundEvent ce2(Event("e3", false, true)
								  | Event("e4", false, false)
								  /*| e5 | e5r */);
#if 0
  //CompoundEvent ce2_1(ce2);
  CU_ASSERT_TRUE_FATAL(( Event("e3", false, true)
								  | Event("e4", false, false) 
								  | e5r ).wait(TAU));
  CU_ASSERT_TRUE_FATAL((ce1 | 
								( Event("e3", false, true)
								  | Event("e4", false, false) 
								  | e5r )).wait(TAU));
  CompoundEvent ce2_2;
  ce2_2 = ce2;
  CU_ASSERT_FALSE_FATAL((ce1 | std::move(ce2_2))
    . wait(TAU));
  ce1 |= ce2;
  CU_ASSERT_FALSE_FATAL(ce1.wait(TAU));
  CU_ASSERT_FALSE_FATAL((ce1 | ce2).wait(TAU));
  e5.set();
  CU_ASSERT_TRUE_FATAL(ce1.wait(TAU));
  e5.reset();
  CU_ASSERT_FALSE_FATAL(ce1.wait(TAU));
  CU_ASSERT_TRUE_FATAL(
	 (Event("e6", false, true) | ce1).wait(TAU));
  CU_ASSERT_FALSE_FATAL(
	 (Event("e7", false, false) | ce1).wait(TAU));
  CU_ASSERT_FALSE_FATAL(
	 (Event("e7", true, false) | ce1).wait(TAU));
  const CompoundEvent ce3(ce1 | ce2);
  CU_ASSERT_FALSE_FATAL(ce3 | (e1 | e2)).wait(TAU);
  e2.set();
  CU_ASSERT_TRUE_FATAL(ce3 | (e1 | e2)).wait(TAU);
  CU_ASSERT_FALSE_FATAL(ce3 | (e1 | e2)).wait(TAU);
  e5.set();
  CU_ASSERT_TRUE_FATAL(ce3 | (e1 | e2)).wait(TAU);
  e5.reset();
  CU_ASSERT_FALSE_FATAL(ce3 | (e1 | e2)).wait(TAU);
#endif
}

