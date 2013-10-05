#include "ClassWithStates.h"
#include "CUnit.h"
#include "SSingleton.hpp"
#include "tests.h"
#include <string>

static void test_arrival_event();
static void test_transitional_event();

CU_TestInfo ClassWithStatesTests[] = {
  {"an arrival event test",
   test_arrival_event},
  {"a transitional event test",
   test_transitional_event},
  CU_TEST_INFO_NULL
};

// init the test suite
int ClassWithStatesCUInit() 
{
  return 0;
}

// clean the test suite
int ClassWithStatesCUClean() 
{
  return 0;
}

//typedef RThread<std::thread> RT;

//static const unsigned int ms100 = 100;

DECLARE_AXIS(ClassWithStatesCUAxis, StateAxis);

DEFINE_AXIS(
  ClassWithStatesCUAxis,
  {"charged", "discharged"},
  {{"charged", "discharged"}, {"discharged", "charged"}}
  );

char test_class_initial_state[] = "discharged";

#if 0
class TestStates
{
public:
  DECLARE_STATES(ClassWithStatesCUAxis, State);
  DECLARE_STATE_FUN(State, charged);
  DECLARE_STATE_FUN(State, discharged);

  //event_fun = {
};
#endif

typedef ClassWithEvents
  <ClassWithStatesCUAxis, test_class_initial_state> 
    TestParent;

class ClassTest : public TestParent
{
public:
  typedef TestParent Parent;

  class TheClass 
    : public Parent::TheClass, 
      public SAutoSingleton<TheClass>
  {
    typedef ClassWithStatesCUAxis A;
  public:
    DECLARE_STATES(ClassWithStatesCUAxis, State);
    DECLARE_STATE_FUN(State, charged);
    DECLARE_STATE_FUN(State, discharged);

    event_fun<A> is_charged;
    event_fun<A> is_discharged;

    trans_event_fun<A> charging;
    trans_event_fun<A> discharging;

    CompoundEvent is_terminal_state() const override
    {
      return CompoundEvent();
    }

    MULTIPLE_INHERITANCE_DEFAULT_MEMBERS;

#if 1 
    // SSingleton version
    TheClass()
      : is_charged(this, chargedFun()),
        is_discharged(this, dischargedFun()),
        charging(this, dischargedFun(), chargedFun()),
        discharging(this, chargedFun(), dischargedFun()) 
    {
      complete_construction();
    }
  protected:
#else
  protected:
    TheClass& self() 
    {
      static std::once_flag of;
      static TheClass* slf = nullptr;
      std::call_once(of, [](){ slf = new TheClass; });
      assert(slf);
      return *slf;
    }

  private:
    TheClass()
      : is_charged(this, chargedFun()),
        is_discharged(this, dischargedFun()),
        charging(this, dischargedFun(), chargedFun()),
        discharging(this, chargedFun(), dischargedFun()) {}
    TheClass(const TheClass&) = delete;
    ~TheClass() = delete;
    TheClass& operator=(const TheClass&) = delete;
#endif
  };

  static TheClass& the_class()
  {
    return TheClass::instance();
  }

  static void charge()
  {
    RAxis<ClassWithStatesCUAxis>::move_to
      (TheClass::instance(), TheClass::chargedFun());
  }

  static void discharge()
  {
    RAxis<ClassWithStatesCUAxis>::move_to
      (TheClass::instance(), TheClass::chargedFun());
  }
};

#define TEST_OBJ_STATE(obj, axis, state)        \
  {                                             \
    RState<axis> st(ClassTest::the_class());         \
    CU_ASSERT_EQUAL_FATAL(st, state);           \
  } while(0)

static void test_arrival_event()
{
  ClassTest obj;
  bool wt;

  SharedThread([&obj] ()
  {
    USLEEP(100);
    RAxis<ClassWithStatesCUAxis>::move_to
      (obj.the_class(), ClassTest::TheClass::chargedFun());
    USLEEP(100);
    RAxis<ClassWithStatesCUAxis>::move_to
      (obj.the_class(), ClassTest::TheClass::dischargedFun());
  });

  wt = obj.the_class().is_charged().wait(1);
  CU_ASSERT_FALSE_FATAL(wt);
  TEST_OBJ_STATE(obj.the_class(), ClassWithStatesCUAxis, 
                 ClassTest::TheClass::dischargedFun());
  wt = obj.the_class().is_discharged().wait();
  CU_ASSERT_TRUE_FATAL(wt);
  TEST_OBJ_STATE(obj.the_class(), ClassWithStatesCUAxis, 
                 ClassTest::TheClass::chargedFun());

  wt = obj.the_class().is_discharged().wait(1);
  CU_ASSERT_FALSE_FATAL(wt);
  TEST_OBJ_STATE(obj.the_class(), ClassWithStatesCUAxis, 
                 ClassTest::TheClass::chargedFun());
  wt = obj.the_class().is_discharged().wait();
  CU_ASSERT_TRUE_FATAL(wt);
  TEST_OBJ_STATE(obj.the_class(), ClassWithStatesCUAxis, 
                 ClassTest::TheClass::dischargedFun());
}

static void test_transitional_event()
{
  ClassTest obj;
  bool wt;

  SharedThread([&obj] ()
  {
    USLEEP(100);
    RAxis<ClassWithStatesCUAxis>::move_to
      (obj.the_class(), ClassTest::TheClass::chargedFun());
    USLEEP(100);
    RAxis<ClassWithStatesCUAxis>::move_to
      (obj.the_class(), ClassTest::TheClass::dischargedFun());
  });

  wt = obj.the_class().charging().wait(1);
  CU_ASSERT_FALSE_FATAL(wt);
  TEST_OBJ_STATE(obj, ClassWithStatesCUAxis, 
                 ClassTest::TheClass::dischargedFun());
  wt = obj.the_class().charging().wait();
  CU_ASSERT_TRUE_FATAL(wt);
  TEST_OBJ_STATE(obj, ClassWithStatesCUAxis, 
                 ClassTest::TheClass::chargedFun());

  wt = obj.the_class().discharging().wait(1);
  CU_ASSERT_FALSE_FATAL(wt);
  TEST_OBJ_STATE(obj, ClassWithStatesCUAxis, 
                 ClassTest::TheClass::chargedFun());
  wt = obj.the_class().discharging().wait(1);
  CU_ASSERT_TRUE_FATAL(wt);
  TEST_OBJ_STATE(obj, ClassWithStatesCUAxis, 
                 ClassTest::TheClass::dischargedFun());
}
