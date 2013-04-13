#include "RThreadRepository.hpp"
#include "CUnit.h"
#include <thread>

void test_local_block();
void test_local_no_start();
void test_thread_in_repository();

CU_TestInfo RThreadTests[] = {
  {"a working thread must prevent exiting "
	"from a local block",
	test_local_block},
  {"a local without start",
	test_local_no_start},
  {"thread creation in a repository",
	test_thread_in_repository},
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

typedef RThread<std::thread> RT;

static const std::chrono::milliseconds ms100(100);

struct T1 : public RT
{ 
  struct Par : public RT::Par
  {
	 RThreadBase* create_derivation
	   (const ObjectCreationInfo& oi) const
	 { 
		return new T1(oi, *this); 
	 }
  };

  typedef Logger<T1> log;

  T1() : RT("T1") {} 

  T1(const ObjectCreationInfo& oi, const Par& par) 
	 : RT(oi, par) {} 

  ~T1() { destroy(); }
  void run() { 
	 RT::ThreadState::move_to(*this, workingState);
	 std::this_thread::sleep_for(ms100);
  }
};

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
	 t1_ptr->start();
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
	 std::this_thread::sleep_for(ms100);
	 std::this_thread::sleep_for(ms100);
	 CU_ASSERT_TRUE_FATAL(
		RThreadState::state_is(t2, T2::terminatedState));
  }
}

void test_local_no_start()
{
  T2 t2(false);
  t2.start();
  std::this_thread::sleep_for(ms100);
  std::this_thread::sleep_for(ms100);
  CU_ASSERT_TRUE_FATAL(
	 RThreadState::state_is(t2, T2::workingState));
  t2.stop();
}

static RThreadRepository<
  std::thread, std::map, std::thread::native_handle_type
> thread_repository;

void test_thread_in_repository()
{
  T1* thread = dynamic_cast<T1*>
	 (thread_repository.create_thread(T1::Par()));
  thread->start();
  thread_repository.delete_object(thread, true);
}

