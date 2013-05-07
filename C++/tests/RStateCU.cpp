#include "RState.hpp"
#include "REvent.hpp"
#include "RObjectWithStates.hpp"
#include "state_objects.h"
#include "CUnit.h"

void test_move_to();
void test_compare_and_move_single();
void test_compare_and_move_set();
void test_derived_axis();
void test_splitted_axis();

CU_TestInfo RStateTests[] = {
  {"RState::move_to(obj, to)", test_move_to},
  {"RState::compare_and_move(obj, from, to)", 
	test_compare_and_move_single},
  {"RState::compare_and_move(obj, from_set, to)", 
	test_compare_and_move_set},
  {"derived axis", test_derived_axis},
  {"splitted axis", test_splitted_axis},
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

DEFINE_STATES(TestAxis);

DEFINE_STATE_CONST(TestObject, State, s1);
DEFINE_STATE_CONST(TestObject, State, s2);
DEFINE_STATE_CONST(TestObject, State, s3);
DEFINE_STATE_CONST(TestObject, State, s4);
DEFINE_STATE_CONST(TestObject, State, s5);

DEFINE_STATES(DerivedAxis);

DEFINE_STATE_CONST(DerivedObject, State, q1);
DEFINE_STATE_CONST(SplittedStateObject, State, s1);
DEFINE_STATE_CONST(SplittedStateObject, State, q1);

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
  try {
	 STATE_OBJ(TestObject, move_to, orig, s2);
	 CU_FAIL_FATAL("No transition s3->s2 for orig");
  }
  catch (const InvalidStateTransition&) {}
  // now it is possible to move in TestAxis again
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

#if 1
#define TEST_SPLITTED(st1, st2) \
{ \
  CU_ASSERT_TRUE_FATAL( \
    STATE_OBJ(TestObject, state_is, original, st1)); \
  CU_ASSERT_TRUE_FATAL( \
    STATE_OBJ(TestObject, state_is, derived, st2)); \
} while(0)
#else
#define TEST_SPLITTED(st1, st2) \
{ \
  CU_ASSERT_TRUE_FATAL( \
    RMixedAxis<DerivedAxis, TestAxis>::state_is \
      (Derived
  ); \
  CU_ASSERT_TRUE_FATAL( \
    STATE_OBJ(TestObject, state_is, derived, st2)); \
} while(0)
#endif

void test_splitted_axis()
{
  TestObject original;
  SplittedStateObject derived(&original);

  STATE_OBJ(TestObject, move_to, derived, s2);
  TEST_SPLITTED(s1, s2); // a state is splitted
  STATE_OBJ(TestObject, move_to, derived, s4);
  TEST_SPLITTED(s1, s4);
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (derived, TestObject::s1State);
  TEST_SPLITTED(s1, s1); 

  // orig movement
  STATE_OBJ(TestObject, move_to, original, s2);
  STATE_OBJ(TestObject, move_to, original, s3);
  STATE_OBJ(TestObject, move_to, original, s5);
  TEST_SPLITTED(s5, s5); // due to state_changed
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (original, DerivedObject::q1State);

  RMixedAxis<DerivedAxis, TestAxis>::state_is(original, 
									 DerivedObject::q1State);
  RMixedAxis<DerivedAxis, TestAxis>::state_is(derived, 
									 DerivedObject::q1State);

  // unable to move orig in TestAxis now
  try {
	 STATE_OBJ(TestObject, move_to, original, s3);
  }
  catch (const InvalidState& ex) {
	 CU_ASSERT_EQUAL_FATAL(ex.state, 
								  DerivedObject::q1State);
  }
  RMixedAxis<DerivedAxis, TestAxis>::state_is(original, 
									 DerivedObject::q1State);
  RMixedAxis<DerivedAxis, TestAxis>::state_is(derived, 
									 DerivedObject::q1State);
  // but in DerivedAxis is possible
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (original, TestObject::s3State);
  TEST_SPLITTED(s3, s3);
  try {
	 STATE_OBJ(TestObject, move_to, original, s2);
	 CU_FAIL_FATAL("No transition s3->s2 for orig");
  }
  catch (const InvalidStateTransition&) {}
  TEST_SPLITTED(s3, s3);
  // now it is possible to move in TestAxis again
  STATE_OBJ(TestObject, move_to, original, s5);
  TEST_SPLITTED(s5, s5);
  // derived movement
  STATE_OBJ(TestObject, move_to, derived, s2);
  TEST_SPLITTED(s5, s2);
  STATE_OBJ(TestObject, move_to, derived, s3);
  TEST_SPLITTED(s5, s3);
  STATE_OBJ(TestObject, move_to, derived, s5);
  TEST_SPLITTED(s5, s5);
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (derived, DerivedObject::q1State);
  RAxis<TestAxis>::state_is(original, 
									 DerivedObject::s5State);
  RMixedAxis<DerivedAxis, TestAxis>::state_is(derived, 
									 DerivedObject::q1State);
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (derived, DerivedObject::s3State);
  TEST_SPLITTED(s5, s3);
}


