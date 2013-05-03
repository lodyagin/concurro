#include "Event.h"
#include "tests.h"
#include "RThread.h"

void test_manual_reset();
void test_auto_reset();
void test_wait_for_any();
void test_event_2threads();
void test_shadow();
void test_empty_compound();

CU_TestInfo EventTests[] = {
  {"manual reset", test_manual_reset},
  {"auto reset", test_auto_reset},
  {"wait for any", test_wait_for_any},
  {"2 threads wait while event not set", test_event_2threads},
  {"test a shadow of an event", test_shadow},
  {"an empty compound event", test_empty_compound},
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
  CU_ASSERT_EQUAL_FATAL(ce1.size(), 1);
  CU_ASSERT_FALSE_FATAL(ce1.wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL(ce1.wait(TAU));
  Event e2("e2", false);
  ce1 |= e2;
  CU_ASSERT_EQUAL_FATAL(ce1.size(), 2);
  // uniq test
  ce1 |= e2;
  ce1 |= e1;
  CU_ASSERT_EQUAL_FATAL(ce1.size(), 2);
  CU_ASSERT_FALSE_FATAL(ce1.wait(TAU));
  e2.set();
  CU_ASSERT_TRUE_FATAL(ce1.wait(TAU));

  CU_ASSERT_TRUE_FATAL(
	 (Event("e3", false, true) | Event("e4", false, false))
	  . wait(TAU));

  CompoundEvent ce2 {e1, e2};
  CU_ASSERT_FALSE_FATAL(ce2.wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL(ce2.wait(TAU));
  CU_ASSERT_FALSE_FATAL(ce2.wait(TAU));
  
  Event e3("e3", true);
  CU_ASSERT_FALSE_FATAL((ce2 | e3).wait(0));
  e3.set();
  CU_ASSERT_TRUE_FATAL((ce2 | e3).wait(0));
  CompoundEvent ce3 = ce2 | e3;
  CU_ASSERT_TRUE_FATAL(ce3.wait(0));
  e3.reset();
  CompoundEvent ce4 = ce2 | e3;
  CU_ASSERT_FALSE_FATAL(ce4.wait(0));
  e1.set();
  CU_ASSERT_TRUE_FATAL(ce4.wait(0));
  CU_ASSERT_FALSE_FATAL(ce4.wait(0));
  CU_ASSERT_FALSE_FATAL(ce3.wait(0));

  CompoundEvent ce5 = e1 | e2;
  CU_ASSERT_FALSE_FATAL(ce5.wait(0));
  e1.set();
  CU_ASSERT_TRUE_FATAL(ce5.wait(0));
  CU_ASSERT_FALSE_FATAL(ce5.wait(0));
  e2.set();
  CU_ASSERT_TRUE_FATAL(ce5.wait(0));
  CU_ASSERT_FALSE_FATAL(ce5.wait(0));

  Event e5("e5", true, false);
  const Event& e5r = e5;
  CU_ASSERT_TRUE_FATAL(( Event("e3", false, true)
								  | Event("e4", false, false) 
								  | e5r ).wait(TAU));
  CU_ASSERT_TRUE_FATAL((ce1 | 
								( Event("e3", false, true)
								  | Event("e4", false, false) 
								  | e5r )).wait(TAU));
  CompoundEvent ce2_2;
  ce2_2 = ce2;
  ce1 |= ce2;
  CU_ASSERT_FALSE_FATAL(ce1.wait(TAU));
  CU_ASSERT_FALSE_FATAL((ce1 | ce2).wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL(ce1.wait(TAU));
  e1.reset();
  CU_ASSERT_FALSE_FATAL(ce1.wait(TAU));
  CU_ASSERT_TRUE_FATAL(
	 (Event("e6", false, true) | ce1).wait(TAU));
  CU_ASSERT_FALSE_FATAL(
	 (Event("e7", false, false) | ce1).wait(TAU));
  CU_ASSERT_FALSE_FATAL(
	 (Event("e7", true, false) | ce1).wait(TAU));
  const CompoundEvent ce6(ce1 | ce2);
  CU_ASSERT_FALSE_FATAL((ce6 | (e1 | e2)).wait(TAU));
  e2.set();
  CU_ASSERT_TRUE_FATAL((ce6 | (e1 | e2)).wait(TAU));
  CU_ASSERT_FALSE_FATAL((ce6 | (e1 | e2)).wait(TAU));
  e1.set();
  CU_ASSERT_TRUE_FATAL((ce6 | (e1 | e2)).wait(TAU));
  e1.reset();
  CU_ASSERT_FALSE_FATAL((ce6 | (e1 | e2)).wait(TAU));
  CU_ASSERT_EQUAL_FATAL(ce1.size(), 2);
  CU_ASSERT_EQUAL_FATAL(ce2.size(), 2);
  CU_ASSERT_EQUAL_FATAL(ce2_2.size(), 2);
  CU_ASSERT_EQUAL_FATAL(ce3.size(), 3);
  CU_ASSERT_EQUAL_FATAL(ce4.size(), 3);
  CU_ASSERT_EQUAL_FATAL(ce5.size(), 2);
  CU_ASSERT_EQUAL_FATAL(ce6.size(), 2);
}

void test_event_2threads()
{
  static Event e("test_event_2threads::e", true, false);

  struct S: public RT {
  	S() : RT("T1") {}
  	~S() { destroy(); }
  	void run() {
  		RT::ThreadState::move_to(*this, workingState);
  		e.set();
  	}
  } s1;
  struct W: public RT {
  	W() : RT("T2") {}
  	~W() { destroy(); }
  	void run() {
  		RT::ThreadState::move_to(*this, workingState);
  		e.wait();
  	}
  } w1;
  W w2;

  w1.start();
  w2.start();
  USLEEP(100);
  CU_ASSERT_TRUE_FATAL(w1.is_running());
  CU_ASSERT_TRUE_FATAL(w2.is_running());
  CU_ASSERT_FALSE_FATAL(s1.is_running());
  s1.start();
  USLEEP(100);
  CU_ASSERT_FALSE_FATAL(w1.is_running());
  CU_ASSERT_FALSE_FATAL(w2.is_running());
  CU_ASSERT_FALSE_FATAL(s1.is_running());
}

void test_shadow()
{
  Event ev("EventCU::test_shadow::ev", true);
  CU_ASSERT_FALSE_FATAL(ev.get_shadow());
  CU_ASSERT_FALSE_FATAL(ev.wait_shadow(TAU));
  ev.reset();
  CU_ASSERT_FALSE_FATAL(ev.get_shadow());
  ev.wait(TAU);
  CU_ASSERT_FALSE_FATAL(ev.get_shadow());
  CU_ASSERT_FALSE_FATAL(ev.wait_shadow(TAU));
  ev.set();
  CU_ASSERT_TRUE_FATAL(ev.get_shadow());
  CU_ASSERT_TRUE_FATAL(ev.get_shadow());
  CU_ASSERT_TRUE_FATAL(ev.wait_shadow(TAU));
  CU_ASSERT_TRUE_FATAL(ev.get_shadow());
  ev.wait(TAU);
  CU_ASSERT_TRUE_FATAL(ev.get_shadow());
  ev.reset();
  CU_ASSERT_TRUE_FATAL(ev.get_shadow());
  CU_ASSERT_FALSE_FATAL(ev.wait(0));
  CU_ASSERT_TRUE_FATAL(ev.wait_shadow(0));
  CU_ASSERT_FALSE_FATAL(ev.wait(TAU));
  CU_ASSERT_TRUE_FATAL(ev.wait_shadow(TAU));
  const Event ev2 = ev;
  CU_ASSERT_TRUE_FATAL(ev.wait_shadow(TAU));
}

void test_empty_compound()
{
  CompoundEvent ce;
  CU_ASSERT_TRUE_FATAL(ce.wait(TAU));
  CU_ASSERT_TRUE_FATAL(ce.isSignalled());
}
