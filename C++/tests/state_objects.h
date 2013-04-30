#ifndef SOUPTCP_TESTS_STATE_OBJECTS_H_
#define SOUPTCP_TESTS_STATE_OBJECTS_H_

DECLARE_AXIS(TestAxis, StateAxis);
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

protected:
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

#endif
