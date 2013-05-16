#include "RThreadRepository.h"
#include "RThread.hpp"
#include "CUnit.h"
#include "tests.h"
#include <thread>

void test_local_block();
void test_local_no_start();
void test_thread_in_repository();
void test_current();
void test_destroy_without_start();
void test_cancel();

CU_TestInfo RThreadTests[] = {
  {"a working thread must prevent exiting "
   "from a local block",
   test_local_block},
  {"a local without start",
   test_local_no_start},
  {"thread creation in a repository",
   test_thread_in_repository},
  {"test RThread<std::thread>::current()",
   test_current},
  {"termination without start()",
   test_destroy_without_start},
  {"RThreadBase::cancel()", test_cancel},
  CU_TEST_INFO_NULL
};

// init the test suite
int RThreadCUInit() 
{
  return 0;
}

// clean the test suite
int RThreadCUClean() 
{
  return 0;
}

static bool check_my_name(const std::string& n);

DECLARE_AXIS(T1Axis, StateAxis,
             {
               "initial", "check_passed", "check_failed"
                 },
             {
               {"initial", "check_passed"},
               {"initial", "check_failed"}
             }
  );

class T1 : public RT, public RObjectWithEvents<T1Axis>
{ 
  DECLARE_EVENT(T1Axis, check_passed);
  DECLARE_EVENT(T1Axis, check_failed);

public:
  DECLARE_STATES(T1Axis, St);
  DECLARE_STATE_CONST(St, initial);
  DECLARE_STATE_CONST(St, check_passed);
  DECLARE_STATE_CONST(St, check_failed);

  DEFAULT_LOGGER(T1);

  /*StateAxis& get_axis() const override
  {
    return T1Axis::self();
    }*/

  CompoundEvent is_terminal_state() const override
    {
      return is_check_passed_event
        | is_check_failed_event;
    }

  struct Par : public RT::Par
  {
    Par(const std::string& name_ = std::string())
      : name(name_) {}
	 
    RThreadBase* create_derivation
    (const ObjectCreationInfo& oi) const
      { 
        return new T1(oi, *this); 
      }

    const std::string name;
  };

  T1() 
    : RT("T1"), 
      RObjectWithEvents<T1Axis>(initialState),
      CONSTRUCT_EVENT(check_passed),
      CONSTRUCT_EVENT(check_failed)
    {} 

  T1(const ObjectCreationInfo& oi, const Par& par) 
    : RT(oi, par), 
      RObjectWithEvents<T1Axis>(initialState),
      CONSTRUCT_EVENT(check_passed),
      CONSTRUCT_EVENT(check_failed),
      name(par.name) {} 

  ~T1() { destroy(); }

  void run() 
  { 
    RT::ThreadState::move_to(*this, workingState);
    if (::check_my_name(name))
      T1::St::move_to(*this, check_passedState);
    else
      T1::St::move_to(*this, check_failedState);
    USLEEP(100);
  }

  std::string universal_id() const override
    {
      return universal_object_id;
    }

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object) override
  {
    ax.state_changed(this, object, state_ax);
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

#if 0  
  Event get_event (const UniversalEvent& ue) override
    {
      return RObjectWithEvents<T1Axis>::get_event(ue);
    }

  Event get_event (const UniversalEvent& ue) const override
    {
      return RObjectWithEvents<T1Axis>::get_event(ue);
    }

  Event create_event
  (const UniversalEvent& ue) const override
    {
      return RObjectWithEvents<T1Axis>::create_event(ue);
    }
#endif

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
#if 1
    ax.update_events(this, trans_id, to);
#else
    return RObjectWithEvents<T1Axis>::update_events
      (T1Axis::self(), trans_id, to);
#endif
  }

  const std::string name;
};

static bool check_my_name(const std::string& n)
{
  T1* t1 = dynamic_cast<T1*>(RT::current());
  return (t1 && t1->name == n);
}

DEFINE_STATES(T1Axis);
		
DEFINE_STATE_CONST(T1, St, initial);
DEFINE_STATE_CONST(T1, St, check_passed);
DEFINE_STATE_CONST(T1, St, check_failed);

// It creates T1 in run()
struct T2 : public RT 
{ 
  typedef Logger<T2> log;

  const bool call_start;
  T1* t1_ptr;

  T2(bool start_) : RT("T2"), call_start(start_) {} 
  ~T2() { destroy(); }
  void run() { 
    RT::ThreadState::move_to(*this, workingState);
    T1 t1;
    t1_ptr = &t1;
    if (call_start) 
      t1.start();
  }
  void stop() {
    //t1_ptr->start();
    RT::stop();
  }
};

void test_local_block()
{
  {
    T1 t1;
    t1.start();
  }

  {
    T2 t2(true);
    t2.start();
    USLEEP(100);
    USLEEP(100);
    CU_ASSERT_TRUE_FATAL(
      RThreadState::state_is(t2, T2::terminatedState));
  }
}

void test_local_no_start()
{
  T2 t2(false);
  t2.start();
  USLEEP(100);
  USLEEP(100);
  CU_ASSERT_TRUE_FATAL(
    RThreadState::state_is(t2, T2::terminatedState));
  t2.stop();
}

void test_thread_in_repository()
{
  RThreadRepository<RT>& tr =
    RThreadRepository<RThread<std::thread>>::instance();

  RThreadFactory& tf = tr;

  // direct repository request
  {
    CU_ASSERT_EQUAL_FATAL(tr.size(), 0);
    T1* thread = dynamic_cast<T1*>
      (tf.create_thread(T1::Par()));
    CU_ASSERT_EQUAL_FATAL(tr.size(), 1);
    thread->start();
    CU_ASSERT_EQUAL_FATAL(tr.size(), 1);
    tf.delete_thread(thread); //implies stop()
    CU_ASSERT_EQUAL_FATAL(tr.size(), 0);
  }

  // by create()
  { 
    CU_ASSERT_EQUAL_FATAL(tr.size(), 0);
    T1* thread1 = RT::create<T1>();
    T1* thread2 = T1::create<T1>();
    CU_ASSERT_EQUAL_FATAL(tr.size(), 2);
    thread1->start();
    CU_ASSERT_EQUAL_FATAL(tr.size(), 2);
    thread2->start();
    thread1->remove();
    CU_ASSERT_EQUAL_FATAL(tr.size(), 1);
    thread2->remove();
    CU_ASSERT_EQUAL_FATAL(tr.size(), 0);
  }
}

void test_current()
{
  RThreadRepository<RT>& tr =
    RThreadRepository<RThread<std::thread>>::instance();

  // this thread is not registered;
  CU_ASSERT_PTR_NULL_FATAL(RT::current());

  CU_ASSERT_EQUAL_FATAL(tr.size(), 0);
  T1* thread1 = T1::create<T1>("thread1");
  T1* thread2 = T1::create<T1>("thread2");
  thread1->start();
  thread2->start();
  thread1->is_terminal_state().wait();
  thread2->is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(thread1->is_check_passed().signalled());
  CU_ASSERT_FALSE_FATAL(thread1->is_check_failed().signalled());
  CU_ASSERT_TRUE_FATAL(thread2->is_check_passed().signalled());
  CU_ASSERT_FALSE_FATAL(thread2->is_check_failed().signalled());
  thread1->remove(); // implies stop()
  thread2->remove();
}

void test_destroy_without_start()
{
  T1 t;
}

void test_cancel()
{
  T1 t1, t2;
  CU_ASSERT_TRUE_FATAL(t1.cancel());
  t2.start();
  CU_ASSERT_FALSE_FATAL(t2.cancel());
  t2.stop();
}

