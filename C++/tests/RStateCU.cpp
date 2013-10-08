#include "RState.hpp"
#include "REvent.hpp"
#include "RObjectWithStates.hpp"
#include "CUnit.h"

namespace curr {
namespace tests {
namespace RStateCU {

#include "state_objects.h"

DEFINE_AXIS_NS(
  TestAxis,
  {"s1", "s2", "s3", "s4", "s5"},
  { {"s1", "s2"},
    {"s2", "s3"},
    {"s2", "s4"},
    {"s3", "s5"},
    {"s4", "s5"}}
  );

DEFINE_AXIS_NS(
  DerivedAxis,
  {"q1"},
  { {"s4", "s1"}, {"s5", "q1"}, {"q1", "s3"},
                                {"s3", "s2"}}
  );

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

void test_move_to();
void test_compare_and_move_single();
void test_compare_and_move_set();
void test_derived_axis();
void test_splitted_axis();
void test_state_change();

}}}

using namespace curr::tests::RStateCU;

CU_TestInfo RStateTests[] = {
  {"RState::move_to(obj, to)", test_move_to},
  {"RState::compare_and_move(obj, from, to)", 
	test_compare_and_move_single},
  {"RState::compare_and_move(obj, from_set, to)", 
	test_compare_and_move_set},
  {"derived axis", test_derived_axis},
  {"splitted axis", test_splitted_axis},
  {"state_change", test_state_change},
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

namespace curr {
namespace tests {
namespace RStateCU {

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

#define TEST_SPLITTED(st1, st2) \
{ \
  CU_ASSERT_TRUE_FATAL( \
    STATE_OBJ(TestObject, state_is, original, st1)); \
  CU_ASSERT_TRUE_FATAL( \
    STATE_OBJ(TestObject, state_is, derived, st2)); \
} while(0)

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
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (derived, DerivedObject::q1State);
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (derived, TestObject::s3State);
  RMixedAxis<DerivedAxis, TestAxis>::move_to
	 (derived, TestObject::s2State);
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

DECLARE_AXIS(A1, StateAxis);
DECLARE_AXIS(A2, StateAxis);
DECLARE_AXIS(A3, A1);
DECLARE_AXIS(AD1, A1);
DECLARE_AXIS(AD2, A1);

DEFINE_AXIS_NS(A1, {"a", "b"}, {{"a", "b"}});
DEFINE_AXIS_NS(A2, {"a", "b"}, {{"a", "b"}});
DEFINE_AXIS_NS(A3, {}, {{"b", "a"}, {"a", "b"}});
DEFINE_AXIS_NS(AD1, {"c"}, {{"b", "c"}, {"c", "b"}});
DEFINE_AXIS_NS(AD2, {"d"}, {{"b", "d"}});

static int c1_cnt = 0;
static int c2_cnt = 0;
static int c2_a1_cnt = 0;
static int c2_a2_cnt = 0;
static int c3_cnt = 0;
static bool c3_update_once;
static int d1_cnt = 0;
static int d1h_cnt = 0;
static int d1hs_cnt = 0;
static int d2_cnt = 0;
static int d2h_cnt = 0;
static int d2hs_cnt = 0;
static int dd_cnt = 0;

// TODO also should work with RObjectWithStates
class C1 : public RObjectWithEvents<A1> 
{
public:
  DECLARE_STATES(A1, S1);
  DECLARE_STATE_CONST(S1, a);
  DECLARE_STATE_CONST(S1, b);

  C1() : RObjectWithEvents<A1>(aState) {}

  void state_changed_impl
  (StateAxis& ax, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object,
   const UniversalState& new_state) override
    {
      RObjectWithEvents<A1>::state_changed_impl
        (ax, state_ax, object, new_state);
      c1_cnt++;
    }

  CompoundEvent is_terminal_state() const override
  {
    return CompoundEvent();
  }
};

DEFINE_STATE_CONST(C1, S1, a);
DEFINE_STATE_CONST(C1, S1, b);

class C2 
  : public RObjectWithEvents<A1>,
    public RObjectWithEvents<A2>
{
public:
  DECLARE_STATES(A1, S1);
  DECLARE_STATES(A2, S2);

  static const RState<A1> a1;
  static const RState<A1> b1;
  static const RState<A2> a2;
  static const RState<A2> b2;

  C2() 
    : RObjectWithEvents<A1>(a1),
      RObjectWithEvents<A2>(a2) 
  {}

  void state_changed_impl
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
    RObjectWithEvents<A1>::state_changed_impl
      (ax, state_ax, object, new_state);
    RObjectWithEvents<A2>::state_changed_impl
      (ax, state_ax, object, new_state);
    c2_cnt++;
    if (is_same_axis<A1>(ax))
      c2_a1_cnt++;
    if (is_same_axis<A2>(ax))
      c2_a2_cnt++;
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

  CompoundEvent create_event 
    (const curr::UniversalEvent& ue) const override 
  { 
    return create_event(ue); 
  } 
  
  void update_events 
    (curr::StateAxis& ax, 
     curr::TransitionId trans_id, 
     uint32_t to) override 
  { 
    ax.update_events(this, trans_id, to); 
  }

  CompoundEvent is_terminal_state() const override
  {
    return CompoundEvent();
  }
};

const RState<A1> C2::a1("a");
const RState<A1> C2::b1("b");
const RState<A2> C2::a2("a");
const RState<A2> C2::b2("b");

class C3 : public RStateSplitter<A3, A1>
{
public:
  DECLARE_STATES(A3, S3);
  DECLARE_STATE_CONST(S3, a);
  DECLARE_STATE_CONST(S3, b);

  C3(RObjectWithEvents<A1>* orig) 
    : RStateSplitter(orig, aState) 
  {
    // TODO remove the direct call
    RStateSplitter::init();
  }

  void state_changed_impl
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
#if 0
      // it is pure virtual
      RStateSplitter<A3, A1>::state_changed_impl
        (ax, state_ax, object, new_state);
#endif
      if (is_same_axis<A3>(ax))
      {
        // FIXME unable to distinguish call as a result of
        // move_to which follows
        if (!c3_update_once) {
          c3_cnt++;
          c3_update_once = true;
        }
        if(!S3::state_is(*this, new_state)) {
          RMixedAxis<A3, A1>::move_to(*this, new_state);
        }
      }
  }

  CompoundEvent is_terminal_state() const override
  {
    return CompoundEvent();
  }
};

#undef S

class D1 
  : public virtual C1,
    public RStateSplitter<AD1, A1>
{
public:
  DECLARE_STATES(AD1, S);
  DECLARE_STATE_CONST(S, c);

  D1() 
    : RStateSplitter
      (this, C1::aState,
       RStateSplitter::state_hook(&D1::state_hook))
  {
    RStateSplitter::init();
  }

  void state_changed_impl
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
    d1_cnt++;
  }

  void state_hook
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state)
  {
    // FIXME? the second call as a result of move_to which
    // follows
    if (!S::state_is(*this, new_state)) { 
      d1hs_cnt++;
    }
    if (!AD1::is_same(ax)) { // FIXME the same
      d1h_cnt++;
      S::move_to(*this, new_state);
    }
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

  CompoundEvent create_event 
    (const curr::UniversalEvent& ue) const override 
  { 
    return create_event(ue); 
  } 
  
  void update_events 
    (curr::StateAxis& ax, 
     curr::TransitionId trans_id, 
     uint32_t to) override 
  { 
    ax.update_events(this, trans_id, to); 
  }

  CompoundEvent is_terminal_state() const override
  {
    return CompoundEvent();
  }
};

DEFINE_STATE_CONST(D1, S, c);

class D2 
  : public virtual C1,
    public RStateSplitter<AD2, A1>
{
public:
  DECLARE_STATES(AD2, S);
  DECLARE_STATE_CONST(S, d);

  D2() 
    : RStateSplitter
      (this, C1::aState,
       RStateSplitter::state_hook(&D2::state_hook))
  {
    RStateSplitter::init();
  }

  void state_changed_impl
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
    d2_cnt++;
  }

  void state_hook
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state)
  {
    if (!S::state_is(*this, new_state)) { 
      d2hs_cnt++;
    }
    if (!AD2::is_same(ax)) {
      d2h_cnt++;
      S::move_to(*this, new_state);
    }
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

  CompoundEvent create_event 
    (const curr::UniversalEvent& ue) const override 
  { 
    return create_event(ue); 
  } 
  
  void update_events 
    (curr::StateAxis& ax, 
     curr::TransitionId trans_id, 
     uint32_t to) override 
  { 
    ax.update_events(this, trans_id, to); 
  }

  CompoundEvent is_terminal_state() const override
  {
    return CompoundEvent();
  }
};

DEFINE_STATE_CONST(D2, S, d);

class DD : public D1, public D2
{
public:
  void state_changed_impl
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
    dd_cnt++;
    ax.state_changed
      (this, object, state_ax, new_state);
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

  CompoundEvent create_event 
    (const curr::UniversalEvent& ue) const override 
  { 
    return create_event(ue); 
  } 
  
  void update_events 
    (curr::StateAxis& ax, 
     curr::TransitionId trans_id, 
     uint32_t to) override 
  { 
    ax.update_events(this, trans_id, to); 
  }

  CompoundEvent is_terminal_state() const override
  {
    return CompoundEvent();
  }
};

DEFINE_STATE_CONST(C3, S3, a);
DEFINE_STATE_CONST(C3, S3, b);

void test_state_change()
{
  //1. Single-axis case
  {
    C1 o1;
    CU_ASSERT_EQUAL_FATAL(c1_cnt, 0);
    move_to(o1, C1::bState);
    CU_ASSERT_EQUAL_FATAL(c1_cnt, 1);
  }

  //2. 2 axis case
  {
    C2 o2;
    CU_ASSERT_EQUAL_FATAL(c2_cnt, 0);
    RAxis<A1>::move_to(o2, C2::b1);
    CU_ASSERT_EQUAL_FATAL(c2_cnt, 1);
    RAxis<A2>::move_to(o2, C2::b2);
    CU_ASSERT_EQUAL_FATAL(c2_cnt, 2);
    CU_ASSERT_EQUAL_FATAL(c2_a1_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(c2_a2_cnt, 1);
  }

  //3.1. RStateSplitter, simple
  {
    C1 o1;
    C3 o3(&o1); // a derived, should be notified by o1
                // changes
    c1_cnt = 0;
    CU_ASSERT_EQUAL_FATAL(c3_cnt, 0);
    c3_update_once = false; 
    move_to(o1, C1::bState);
    CU_ASSERT_EQUAL_FATAL(c1_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(c3_cnt, 1);
    c3_update_once = false;
    RAxis<A3>::move_to(o3, C3::aState);
    CU_ASSERT_EQUAL_FATAL(c1_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(c3_cnt, 2);
  }
  
  //3.2. RStateSplitter, basic has 2 axes
  {
    C2 o2;
    C3 o3(&o2); // a derived, should be notified by o2
                // changes
    c2_cnt = 0;
    c3_cnt = 0;
    c3_update_once = false; 
    RAxis<A1>::move_to(o2, C1::bState);
    CU_ASSERT_EQUAL_FATAL(c2_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(c3_cnt, 1);
    c3_update_once = false; 
    RAxis<A3>::move_to(o3, C3::aState);
    CU_ASSERT_EQUAL_FATAL(c2_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(c3_cnt, 2);
  }
  
  //4. The diamond with A1 as a base axis
  {
    DD dia;
    c1_cnt = 0;
    d1_cnt = 0;
    d1h_cnt = 0;
    d2_cnt = 0;
    d2h_cnt = 0;
    dd_cnt = 0;

    RAxis<A1>::move_to(dia, C1::bState);
    CU_ASSERT_EQUAL_FATAL(c1_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d1_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d1h_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d1hs_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d2_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d2h_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d2hs_cnt, 1);
    //CU_ASSERT_EQUAL_FATAL(dd_cnt, 5);
    RAxis<AD1>::move_to(dia, D1::cState);
    CU_ASSERT_EQUAL_FATAL(c1_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d1_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d1h_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d1hs_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d2_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d2h_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d2hs_cnt, 1);
    //CU_ASSERT_EQUAL_FATAL(dd_cnt, );
    RAxis<AD1>::move_to(dia, D1::bState);
    CU_ASSERT_EQUAL_FATAL(c1_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d1_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d1h_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d1hs_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d2_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d2h_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d2hs_cnt, 1);
    //CU_ASSERT_EQUAL_FATAL(dd_cnt, );
    RAxis<AD2>::move_to(dia, D2::dState);
    CU_ASSERT_EQUAL_FATAL(c1_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d1_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d1h_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d1hs_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d2_cnt, 0);
    CU_ASSERT_EQUAL_FATAL(d2h_cnt, 1);
    CU_ASSERT_EQUAL_FATAL(d2hs_cnt, 1);
    //CU_ASSERT_EQUAL_FATAL(dd_cnt, );
  }
  
}

}
}
}




