#ifndef SOUPTCP_TESTS_STATE_OBJECTS_H_
#define SOUPTCP_TESTS_STATE_OBJECTS_H_

using namespace curr;

DECLARE_AXIS(TestAxis, StateAxis);

class TestObject 
  : public RObjectWithEvents<TestAxis, TestAxis, 1>
{
  DECLARE_EVENT(TestAxis, s3);

public:
  using Parent = RObjectWithEvents<TestAxis, TestAxis, 1>;

  DECLARE_STATES(TestAxis, State);
  DECLARE_STATE_CONST(State, s1);
  DECLARE_STATE_CONST(State, s2);
  DECLARE_STATE_CONST(State, s3);
  DECLARE_STATE_CONST(State, s4);
  DECLARE_STATE_CONST(State, s5);

  TestObject() 
  : Parent(s1State),
    CONSTRUCT_EVENT(s3)
    {}

  std::string object_name() const override
  {
    return "TestObject";
  }

  CompoundEvent is_terminal_state() const override
  {
    return CompoundEvent();
  }

  DEFAULT_LOGGER(TestObject)
};

DECLARE_AXIS(DerivedAxis, TestAxis);

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
public RStateSplitter<DerivedAxis, TestAxis, 1, 1>
{
  A_DECLARE_EVENT(DerivedAxis, TestAxis, s1);
  A_DECLARE_EVENT(DerivedAxis, TestAxis, s2);
  A_DECLARE_EVENT(DerivedAxis, TestAxis, s3);
  A_DECLARE_EVENT(DerivedAxis, TestAxis, q1);

public:
  using Parent = RStateSplitter
    <DerivedAxis, TestAxis, 1, 1>;

  DECLARE_STATES(DerivedAxis, State);
  DECLARE_STATE_CONST(State, s1);
  DECLARE_STATE_CONST(State, q1);

  SplittedStateObject(TestObject* orig)
  : Parent(orig, s1State),
    CONSTRUCT_EVENT(s1),
    CONSTRUCT_EVENT(s2),
    CONSTRUCT_EVENT(s3),
    CONSTRUCT_EVENT(q1)
    {
      Parent::init();
    }

  // sync a state delegate -> this
  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
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

  std::string object_name() const override
  {
    return "SplittedStateObject";
  }

  DEFAULT_LOGGER(SplittedStateObject);
};

#endif
