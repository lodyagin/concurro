#include "REvent.h"
#include "CUnit.h"
#include "RThread.h"
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

class CDAxes : public StateAxis {};
class Test : public RObjectWithEvents<CDAxes>
{
public:
  DECLARE_STATES(CDAxes, State);
  DECLARE_STATE_CONST(State, charged);
  DECLARE_STATE_CONST(State, discharged);

  Test() : RObjectWithEvents<CDAxes>(dischargedState) {}

  DEFAULT_LOGGER(Test)
};

DEFINE_STATES(Test, CDAxes, State)
({ {"charged", "discharged"},
  {{"charged", "discharged"}, {"discharged", "charged"}}
});

DEFINE_STATE_CONST(Test, State, charged);
DEFINE_STATE_CONST(Test, State, discharged);

void test_simplest_event()
{
  Test obj;

  REvent<CDAxes>("discharged","charged").wait(obj);
  REvent<CDAxes>("charged","discharged").wait(obj);
}
