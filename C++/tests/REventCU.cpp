#include "REvent.hpp"
#include "CUnit.h"
#include "RThread.h"
#include "tests.h"
#include <string>
#include <thread>

void test_simplest_event();

CU_TestInfo REventTests[] = {
  {"a simplest event test",
   test_simplest_event},
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
	 //Event wait_forever("Test::run::wait_forever",true);
	 //wait_forever.wait();
	 USLEEP(100);
	 State::move_to(*this, chargedState);
	 USLEEP(100);
	 State::move_to(*this, dischargedState);
  }

  DEFAULT_LOGGER(Test)
};

//DEFINE_STATES(Test, CDAxis, State)
RAxis<CDAxis> cd_axis
({ {"charged", "discharged"},
  {{"charged", "discharged"}, {"discharged", "charged"}}
});

DEFINE_STATE_CONST(Test, State, charged);
DEFINE_STATE_CONST(Test, State, discharged);

#define TEST_OBJ_STATE(obj, axis, state)			\
  { \
	 RState<axis> st(obj); \
	 CU_ASSERT_EQUAL_FATAL(st, state); \
  } while(0)

void test_simplest_event()
{
  Test obj;
  bool wt;

  obj.start();

  wt= REvent<CDAxis>("discharged","charged").wait(obj, 1);
  CU_ASSERT_FALSE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::dischargedState);
  wt = REvent<CDAxis>("discharged","charged").wait(obj);
  CU_ASSERT_TRUE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::chargedState);

  wt= REvent<CDAxis>("charged","discharged").wait(obj, 1);
  CU_ASSERT_FALSE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::chargedState);
  wt = REvent<CDAxis>("charged","discharged").wait(obj);
  CU_ASSERT_TRUE_FATAL(wt);
  TEST_OBJ_STATE(obj, CDAxis, Test::dischargedState);
}
