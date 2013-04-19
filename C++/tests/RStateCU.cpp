#include "RState.hpp"
#include "REvent.hpp"
#include "RObjectWithStates.hpp"
#include "CUnit.h"

void test_move_to();
void test_compare_and_move_single();
void test_compare_and_move_set();

CU_TestInfo RStateTests[] = {
  {"RState::move_to(obj, to)", test_move_to},
  {"RState::compare_and_move(obj, from, to)", 
	test_compare_and_move_single},
  {"RState::compare_and_move(obj, from_set, to)", 
	test_compare_and_move_set},
  CU_TEST_INFO_NULL
};

// init the test suite
int RStateCUInit() 
{
  return 0;
}

// clean the test suite
int RStateCUClean() 
{
  return 0;
}

class TestAxis : public StateAxis {};

class TestObject : public RObjectWithStates<TestAxis>
{
public:
  DECLARE_STATES(TestAxis, State);
  DECLARE_STATE_CONST(State, s1);
  DECLARE_STATE_CONST(State, s2);
  DECLARE_STATE_CONST(State, s3);
  DECLARE_STATE_CONST(State, s4);
  DECLARE_STATE_CONST(State, s5);
  TestObject() : RObjectWithStates<TestAxis>(s1State) {}

  std::string universal_id() const
  {
	 return "?";
  }

protected:
  DEFAULT_LOGGER(TestObject)
};

DEFINE_STATES(
  TestAxis,
  {"s1", "s2", "s3", "s4", "s5"},
  { {"s1", "s2"},
	 {"s2", "s3"},
	 {"s2", "s4"},
	 {"s3", "s5"},
	 {"s4", "s5"}});

DEFINE_STATE_CONST(TestObject, State, s1);
DEFINE_STATE_CONST(TestObject, State, s2);
DEFINE_STATE_CONST(TestObject, State, s3);
DEFINE_STATE_CONST(TestObject, State, s4);
DEFINE_STATE_CONST(TestObject, State, s5);

void test_move_to()
{
  TestObject obj;
  try { 
	 TestObject::State::move_to(obj, TestObject::s1State); 
	 CU_FAIL_FATAL("No transition s1->s1");
  }
  catch (const InvalidStateTransition& ex) {
	 CU_ASSERT_EQUAL_FATAL(ex.from, TestObject::s1State);
	 CU_ASSERT_EQUAL_FATAL(ex.to, TestObject::s1State);
  }

  TestObject::State::move_to(obj, TestObject::s2State); 
  
  try { 
	 TestObject::State::move_to(obj, TestObject::s1State); 
	 CU_FAIL_FATAL("No transition s2->s1");
  }
  catch (const InvalidStateTransition& ex) {
	 CU_ASSERT_EQUAL_FATAL(ex.from, TestObject::s2State);
	 CU_ASSERT_EQUAL_FATAL(ex.to, TestObject::s1State);
  }

  try { 
	 TestObject::State::move_to(obj, TestObject::s5State); 
	 CU_FAIL_FATAL("No transition s2->s5");
  }
  catch (const InvalidStateTransition& ex) {
	 CU_ASSERT_EQUAL_FATAL(ex.from, TestObject::s2State);
	 CU_ASSERT_EQUAL_FATAL(ex.to, TestObject::s5State);
  }
}

void test_compare_and_move_single()
{
  TestObject obj;

  CU_ASSERT_FALSE_FATAL(
	 TestObject::State::compare_and_move
	 (obj, TestObject::s2State, TestObject::s4State)); 
  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::state_is(obj, TestObject::s1State)); 

  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::compare_and_move
	 (obj, TestObject::s1State, TestObject::s2State)); 
  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::state_is(obj, TestObject::s2State)); 

  CU_ASSERT_FALSE_FATAL(
	 TestObject::State::compare_and_move
	 (obj, TestObject::s4State, TestObject::s3State)); 

  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::compare_and_move
	 (obj, TestObject::s2State, TestObject::s4State)); 

  try { 
	 TestObject::State::compare_and_move
		(obj, TestObject::s4State, TestObject::s3State); 
	 CU_FAIL_FATAL("No transition s4->s3");
  }
  catch (const InvalidStateTransition& ex) {
	 CU_ASSERT_EQUAL_FATAL(ex.from, TestObject::s4State);
	 CU_ASSERT_EQUAL_FATAL(ex.to, TestObject::s3State);
  }
  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::state_is(obj, TestObject::s4State)); 
}

void test_compare_and_move_set()
{
  TestObject obj;

  CU_ASSERT_FALSE_FATAL(
	 TestObject::State::compare_and_move
	 (obj, std::set<RState<TestAxis>>({TestObject::s2State}), 
	  TestObject::s4State)); 
  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::state_is(obj, TestObject::s1State)); 

  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::compare_and_move
	 (obj, std::set<RState<TestAxis>>({TestObject::s1State}),
	  TestObject::s2State)); 
  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::state_is(obj, TestObject::s2State)); 

  CU_ASSERT_FALSE_FATAL(
	 TestObject::State::compare_and_move
	 (obj, 
	  {TestObject::s1State, TestObject::s3State, 
		TestObject::s4State, TestObject::s5State },
	  TestObject::s3State)); 

  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::compare_and_move
	 (obj, {TestObject::s1State, TestObject::s2State}, 
	  TestObject::s4State)); 

  try { 
	 TestObject::State::compare_and_move
	 (obj, 
	  {TestObject::s1State, TestObject::s3State, 
		TestObject::s4State, TestObject::s5State },
	  TestObject::s3State); 
	 CU_FAIL_FATAL("No transition s4->s3");
  }
  catch (const InvalidStateTransition& ex) {
	 CU_ASSERT_EQUAL_FATAL(ex.from, TestObject::s4State);
	 CU_ASSERT_EQUAL_FATAL(ex.to, TestObject::s3State);
  }
  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::state_is(obj, TestObject::s4State)); 
}



