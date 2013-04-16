#include "REvent.h"
#include "CUnit.h"
#include "RThread.h"
#include "tests.h"
#include <string>
#include <thread>

void test_arrival_event();
void test_transitional_event();

CU_TestInfo REventTests[] = {
  {"an arrival event test",
   test_arrival_event},
  {"a transitional event test",
   test_transitional_event},
  CU_TEST_INFO_NULL
};

// init the test suite
int REventCUInit() 
{
  return 0;
}

// clean the test suite
int REventCUClean() 
{
  return 0;
}

typedef RThread<std::thread> RT;

static const std::chrono::milliseconds ms100(100);

class CDAxis : public StateAxis {};
class Test : public RObjectWithEvents<CDAxis>,
				 public RT
{
public:
  DECLARE_STATES(CDAxis, State);
  DECLARE_STATE_CONST(State, charged);
  DECLARE_STATE_CONST(State, discharged);

  Test() : RObjectWithEvents<CDAxis>(dischargedState),
			  RT("Test") {}
  ~Test() { destroy(); }

  void run()
  {
	 RThreadState::move_to(*this, workingState);
	 USLEEP(100);
	 State::move_to(*this, chargedState);
	 USLEEP(100);
	 State::move_to(*this, dischargedState);
  }

  DEFAULT_LOGGER(Test)
};

DEFINE_STATES(CDAxis, 
 {"charged", "discharged"},
  {{"charged", "discharged"}, {"discharged", "charged"}}
);

DEFINE_STATE_CONST(Test, State, charged);
DEFINE_STATE_CONST(Test, State, discharged);

#define TEST_OBJ_STATE(obj, axis, state)			\
  { \
	 RState<axis> st(obj); \
	 CU_ASSERT_EQUAL_FATAL(st, state); \
  } while(0)

void test_arrival_event()
{
  Test obj;
  bool wt;

  obj.start();

  wt= REvent<CDAxis>(&obj, "charged").wait(1);
  CU_ASSERT_FALSE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::dischargedState);
  wt = REvent<CDAxis>(&obj, "charged").wait();
  CU_ASSERT_TRUE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::chargedState);

  wt= REvent<CDAxis>(&obj, "discharged").wait(1);
  CU_ASSERT_FALSE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::chargedState);
  wt = REvent<CDAxis>(&obj, "discharged").wait();
  CU_ASSERT_TRUE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::dischargedState);
}

void test_transitional_event()
{
  Test obj;
  bool wt;

  obj.start();

  wt= REvent<CDAxis>(&obj, "discharged","charged").wait(1);
  CU_ASSERT_FALSE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::dischargedState);
  wt = REvent<CDAxis>(&obj, "discharged","charged").wait();
  CU_ASSERT_TRUE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::chargedState);

  wt= REvent<CDAxis>(&obj, "charged","discharged").wait(1);
  CU_ASSERT_FALSE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::chargedState);
  wt = REvent<CDAxis>(&obj, "charged","discharged").wait();
  CU_ASSERT_TRUE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::dischargedState);
}
