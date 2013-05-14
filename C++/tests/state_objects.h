#ifndef SOUPTCP_TESTS_STATE_OBJECTS_H_
#define SOUPTCP_TESTS_STATE_OBJECTS_H_

#include "CUnit.h"

DECLARE_AXIS(TestAxis, StateAxis,
             {"s1", "s2", "s3", "s4", "s5"},
             { {"s1", "s2"},
               {"s2", "s3"},
               {"s2", "s4"},
               {"s3", "s5"},
               {"s4", "s5"}}
  );

class TestObject : public RObjectWithEvents<TestAxis>
{
  DECLARE_EVENT(TestAxis, s3);

public:
  DECLARE_STATES(TestAxis, State);
  DECLARE_STATE_CONST(State, s1);
  DECLARE_STATE_CONST(State, s2);
  DECLARE_STATE_CONST(State, s3);
  DECLARE_STATE_CONST(State, s4);
  DECLARE_STATE_CONST(State, s5);
TestObject() 
  : RObjectWithEvents<TestAxis>(s1State),
    CONSTRUCT_EVENT(s3)
    {}

  std::string universal_id() const
  {
    return "?";
  }

  CompoundEvent is_terminal_state() const override
  {
    return CompoundEvent();
  }

protected:
  DEFAULT_LOGGER(TestObject)
    };

DECLARE_AXIS(DerivedAxis, TestAxis,
             {"q1"},
             { {"s4", "s1"}, {"s5", "q1"}, {"q1", "s3"},
                                           {"s3", "s2"}}
  );

class DerivedObject : public TestObject
{
  A_DECLARE_EVENT(DerivedAxis, TestAxis, s1);
  A_DECLARE_EVENT(DerivedAxis, TestAxis, q1);

public:
  DECLARE_STATES(DerivedAxis, State);
  DECLARE_STATE_CONST(State, q1);
DerivedObject()
  : CONSTRUCT_EVENT(s1),
    CONSTRUCT_EVENT(q1)
    {}
};

class SplittedStateObject : 
public RStateSplitter<DerivedAxis, TestAxis>
{
  A_DECLARE_EVENT(DerivedAxis, TestAxis, s1);
  A_DECLARE_EVENT(DerivedAxis, TestAxis, s2);
  A_DECLARE_EVENT(DerivedAxis, TestAxis, s3);
  A_DECLARE_EVENT(DerivedAxis, TestAxis, q1);
public:
  DECLARE_STATES(DerivedAxis, State);
  DECLARE_STATE_CONST(State, s1);
  DECLARE_STATE_CONST(State, q1);

SplittedStateObject(TestObject* orig)
  : RStateSplitter<DerivedAxis, TestAxis>
    (orig, s1State),
    CONSTRUCT_EVENT(s1),
    CONSTRUCT_EVENT(s2),
    CONSTRUCT_EVENT(s3),
    CONSTRUCT_EVENT(q1)
    {
    }

  // sync a state delegate -> this
  void state_changed
    (StateAxis& ax, 
     AbstractObjectWithStates* object) override
  {
    if (object != this) {
      auto* obj1 = dynamic_cast<TestObject*>(object);
      auto* obj2 = dynamic_cast<TestObject*>(delegate);
      CU_ASSERT_PTR_EQUAL_FATAL(obj1, obj2);
      CU_ASSERT_PTR_NOT_NULL_FATAL(obj1);
      RAxis<DerivedAxis>::move_to(
        *this, RState<TestAxis>(*obj1));
    }
  }

  CompoundEvent is_terminal_state() const override
  {
    return CompoundEvent();
  }

  std::string universal_id() const override
  {
    return "?";
  }

  DEFAULT_LOGGER(SplittedStateObject);
};

#endif
