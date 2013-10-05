#include "REvent.h"
#include "CUnit.h"
#include "RThread.h"
#include "state_objects.h"
#include "tests.h"
#include <string>
#include <thread>

static void test_arrival_event();
static void test_transitional_event();
void test_inheritance();
void test_splitting();

CU_TestInfo REventTests[] = {
  {"an arrival event test",
   test_arrival_event},
  {"a transitional event test",
   test_transitional_event},
  {"test inheritance", test_inheritance},
  {"test splitting", test_splitting},
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

static const unsigned int ms100 = 100;

DECLARE_AXIS(CDAxis, StateAxis);

DEFINE_AXIS(
  CDAxis,
  {"charged", "discharged"},
  {{"charged", "discharged"}, {"discharged", "charged"}}
  );

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

  std::string object_name() const override
  {
    return SFORMAT("Test:" << universal_object_id);
  }

  CompoundEvent is_terminal_state() const override
    {
      return CompoundEvent();
    }

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
  }
  
  std::atomic<uint32_t>& 
  current_state(const StateAxis& ax) override
    {
      return ax.current_state(this);
    }

  const std::atomic<uint32_t>& 
  current_state(const StateAxis& ax) const override
    {
      return ax.current_state(this);
    }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
    ax.update_events(this, trans_id, to);
  }

  /*StateAxis& get_axis() const override
  {
    return CDAxis::self();
    }*/

  DEFAULT_LOGGER(Test)
};

DEFINE_STATES(CDAxis);

DEFINE_STATE_CONST(Test, State, charged);
DEFINE_STATE_CONST(Test, State, discharged);

#define TEST_OBJ_STATE(obj, axis, state)        \
  {                                             \
    RState<axis> st(obj);                       \
    CU_ASSERT_EQUAL_FATAL(st, state);           \
  } while(0)

static void test_arrival_event()
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

static void test_transitional_event()
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

#define TAU 100
#define CAST 0

void test_inheritance()
{
  TestObject origin;
  DerivedObject derived;

  REvent<TestAxis> testS1(&origin, "s1");
#if CAST
  REvent<DerivedAxis> derivedS1(&origin, "s1");
  REvent<DerivedAxis> Q1(&origin, "s1");
#endif

#if CAST
  CU_ASSERT_TRUE_FATAL(testS1.wait(TAU));
  CU_ASSERT_TRUE_FATAL(derivedS1.wait(TAU));
#endif
  CU_ASSERT_TRUE_FATAL(derived.is_s1().wait(TAU));

  // s1 -> s2
  STATE_OBJ(TestObject, move_to, origin, s2);
  //RMixedAxis<DerivedAxis, TestAxis>::move_to
  //(derived, DerivedObject::s2State);
  STATE_OBJ(DerivedObject, move_to, derived, s2);
#if CAST
  CU_ASSERT_FALSE_FATAL(testS1.wait(TAU));
  CU_ASSERT_FALSE_FATAL(derivedS1.wait(TAU));
#endif
  CU_ASSERT_FALSE_FATAL(derived.is_s1().wait(TAU));
  CU_ASSERT_FALSE_FATAL(derived.is_s1().wait(TAU));

  // s2 -> s3
  STATE_OBJ(TestObject, move_to, origin, s3);
  STATE_OBJ(DerivedObject, move_to, derived, s3);
  CU_ASSERT_TRUE_FATAL(origin.is_s3().wait(TAU));
  CU_ASSERT_TRUE_FATAL(derived.is_s3().wait(TAU));
  CU_ASSERT_FALSE_FATAL(derived.is_s1().wait(TAU));

  // s3 -> s5
  STATE_OBJ(TestObject, move_to, origin, s5);
  STATE_OBJ(DerivedObject, move_to, derived, s5);
  CU_ASSERT_FALSE_FATAL(origin.is_s3().wait(TAU));
  CU_ASSERT_FALSE_FATAL(derived.is_s3().wait(TAU));

  // s5 -> q1
  STATE_OBJ(DerivedObject, move_to, origin, q1);
  //RMixedAxis<DerivedObject, TestObject>::move_to
//	 (origin, DerivedObject::q1State);
  STATE_OBJ(DerivedObject, move_to, derived, q1);
#if CAST
  CU_ASSERT_TRUE_FATAL(Q1.wait(TAU));
#endif
  CU_ASSERT_TRUE_FATAL(derived.is_q1().wait(TAU));

  // q1 -> s3
  STATE_OBJ(DerivedObject, move_to, origin, s3);
//  RMixedAxis<DerivedObject, TestObject>::move_to
//	 (origin, DerivedObject::q1State);
  STATE_OBJ(DerivedObject, move_to, derived, s3);
#if CAST
  CU_ASSERT_FALSE_FATAL(Q1.wait(TAU));
#endif
  CU_ASSERT_FALSE_FATAL(derived.is_q1().wait(TAU));
  CU_ASSERT_TRUE_FATAL(origin.is_s3().wait(TAU));
  CU_ASSERT_TRUE_FATAL(derived.is_s3().wait(TAU));
}

#define STATE_DERIVED(action, object, state)    \
  RMixedAxis<DerivedAxis, TestAxis>::action     \
  ((object), TestObject::state ## State);

void test_splitting()
{
  TestObject origin;
  SplittedStateObject derived(&origin);

  REvent<TestAxis> testS1(&origin, "s1");
  REvent<DerivedAxis> derivedS1(&derived, "s1");
  REvent<DerivedAxis> Q1(&derived, "q1");

  CU_ASSERT_TRUE_FATAL(testS1.wait(TAU));
  CU_ASSERT_TRUE_FATAL(derivedS1.wait(TAU));
  CU_ASSERT_TRUE_FATAL(derived.is_s1().wait(TAU));

  // s1 -> s2
  STATE_OBJ(TestObject, move_to, origin, s2);
//#if CAST
  CU_ASSERT_FALSE_FATAL(testS1.wait(TAU));
  CU_ASSERT_FALSE_FATAL(derivedS1.wait(TAU));
//#endif
  CU_ASSERT_FALSE_FATAL(derived.is_s1().wait(TAU));
  CU_ASSERT_FALSE_FATAL(derived.is_s1().wait(TAU));

  // s2 -> s3
  STATE_OBJ(TestObject, move_to, origin, s3);
  CU_ASSERT_TRUE_FATAL(origin.is_s3().wait(TAU));
  CU_ASSERT_TRUE_FATAL(derived.is_s3().wait(TAU));
  CU_ASSERT_FALSE_FATAL(derived.is_s1().wait(TAU));

  // s3 -> s5
  STATE_OBJ(TestObject, move_to, origin, s5);
  CU_ASSERT_FALSE_FATAL(origin.is_s3().wait(TAU));
  CU_ASSERT_FALSE_FATAL(derived.is_s3().wait(TAU));

  // s5 -> q1
  RMixedAxis<DerivedAxis, TestAxis>::move_to
    (origin, SplittedStateObject::q1State);
//#if CAST
  CU_ASSERT_TRUE_FATAL(Q1.wait(TAU));
//#endif
  CU_ASSERT_TRUE_FATAL(derived.is_q1().wait(TAU));

  // q1 -> s3
  STATE_DERIVED(move_to, origin, s3);
//#if CAST
  CU_ASSERT_FALSE_FATAL(Q1.wait(TAU));
//#endif
  CU_ASSERT_FALSE_FATAL(derived.is_q1().wait(TAU));
  CU_ASSERT_TRUE_FATAL(origin.is_s3().wait(TAU));
  CU_ASSERT_TRUE_FATAL(derived.is_s3().wait(TAU));

  STATE_DERIVED(move_to, derived, s2);
  CU_ASSERT_TRUE_FATAL(origin.is_s3().wait(TAU));
  CU_ASSERT_TRUE_FATAL(derived.is_s2().wait(TAU));
}



