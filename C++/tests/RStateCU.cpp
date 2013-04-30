#include "RState.hpp"
#include "REvent.hpp"
#include "RObjectWithStates.hpp"
#include "state_objects.h"
#include "CUnit.h"

void test_move_to();
void test_compare_and_move_single();
void test_compare_and_move_set();
void test_derived_axis();

CU_TestInfo RStateTests[] = {
  {"RState::move_to(obj, to)", test_move_to},
  {"RState::compare_and_move(obj, from, to)", 
	test_compare_and_move_single},
  {"RState::compare_and_move(obj, from_set, to)", 
	test_compare_and_move_set},
  {"derived axis", 
	test_derived_axis},
  CU_TEST_INFO_NULL
};

struct A {
  static A axis_;
};
DECLARE_AXIS( B, A);
DECLARE_AXIS(C, B);
DECLARE_AXIS(D, A);

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

DEFINE_STATES(
  DerivedAxis,
  {"q1"},
  { {"s4", "s1"}, {"s5", "q1"}, {"q1", "s3"},
										  {"s3", "s2"}});

DEFINE_STATE_CONST(DerivedObject, State, q1);

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

void test_derived_axis()
{
  TestObject orig;
  DerivedObject derived;

  STATE_OBJ(TestObject, move_to, orig, s2);
  STATE_OBJ(TestObject, move_to, derived, s2);
  STATE_OBJ(TestObject, move_to, orig, s4);
  STATE_OBJ(TestObject, move_to, derived, s4);
  try {
	 STATE_OBJ(TestObject, move_to, orig, s1);
	 CU_FAIL_FATAL("No transition s4->s1 for orig");
  }
  catch (const InvalidStateTransition&) {}
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (derived, TestObject::s1State);
  // orig movement
  STATE_OBJ(TestObject, move_to, orig, s5);
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (orig, DerivedObject::q1State);
  // unable to move orig in TestAxis now
  try {
	 STATE_OBJ(TestObject, move_to, orig, s3);
  }
  catch (const InvalidState& ex) {
	 CU_ASSERT_EQUAL_FATAL(ex.state, 
								  DerivedObject::q1State);
  }
  // but in DerivedAxis is possible
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (orig, TestObject::s3State);
  // no is possible to move in TestAxis again
  try {
	 STATE_OBJ(TestObject, move_to, orig, s2);
	 CU_FAIL_FATAL("No transition s3->s2 for orig");
  }
  catch (const InvalidStateTransition&) {}
  STATE_OBJ(TestObject, move_to, orig, s5);
  CU_ASSERT_TRUE_FATAL(
	 STATE_OBJ(TestObject, state_is, orig, s5));
  // derived movement
  STATE_OBJ(TestObject, move_to, derived, s2);
  STATE_OBJ(TestObject, move_to, derived, s3);
  STATE_OBJ(TestObject, move_to, derived, s5);
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (derived, DerivedObject::q1State);
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (derived, DerivedObject::s3State);
  CU_ASSERT_TRUE_FATAL(
	 STATE_OBJ(TestObject, state_is, derived, s3));
  CU_ASSERT_TRUE_FATAL(
	 TestObject::State::state_in
	 (derived, {TestObject::s3State}));
}


