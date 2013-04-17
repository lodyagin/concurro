#include "RMutex.h"
#include "CUnit.h"
#include "RThread.h"
//#include <iostream>
#include <string>
#include <thread>
#include "REvent.h"
#include "tests.h"
//#include "concurrent/C++/REvent.h"

void test_same_thread_acquire();
void test_same_thread_overrelease();
void test_2_threads_try_acquire();

CU_TestInfo RMutexTests[] = {
  {"RMutex can be acquired by the same thread several times",
   test_same_thread_acquire },
  {"RMutex acquire in 1st thread and 2nd thread can't acquire it at same time",
   test_2_threads_try_acquire },
#if 0
  {"RMutex more releases than acquire in the same thread",
   test_same_thread_overrelease },
#endif
  CU_TEST_INFO_NULL
};

// init the test suite
int RMutexCUInit() 
{
  return 0;
}

// clean the test suite
int RMutexCUClean() 
{
  return 0;
}

void test_same_thread_acquire()
{
  RMutex mx("test_same_thread_acquire");

  MUTEX_ACQUIRE(mx);;
  MUTEX_ACQUIRE(mx);;
  MUTEX_RELEASE(mx);;
  MUTEX_RELEASE(mx);;
}

void test_same_thread_overrelease()
{
  RMutex mx("test_same_thread_overrelease");

  MUTEX_ACQUIRE(mx);;
  MUTEX_ACQUIRE(mx);;
  MUTEX_RELEASE(mx);;
  MUTEX_RELEASE(mx);;
  MUTEX_RELEASE(mx);;
}

typedef RThread<std::thread> RT;

typedef std::chrono::milliseconds MS;

struct TestT : public RT
{

public:
  int getResult(){return arg;}
  TestT(std::string id, RMutex& mutex, MS ms, std::atomic_int &t) :
  	RT(id), sleept(ms), mx(mutex), test(t) {}
  ~TestT() {destroy(); }

  void run(){
  	RT::ThreadState::move_to(*this, workingState);
    MUTEX_ACQUIRE(mx);
    std::this_thread::sleep_for(sleept);
    arg = ++test;
    MUTEX_RELEASE(mx);
  }

  MS sleept;
  int arg;
  RMutex& mx;
  std::atomic_int& test;
  typedef Logger<TestT> log;
};

void test_2_threads_try_acquire(){
  RMutex mx("mx");
  std::atomic_int test(0);
  TestT thread1("1", mx, MS(100), test);
  thread1.start();
  TestT thread2("2", mx, MS(0), test);
  thread2.start();
  sleep(1);
  if (thread1.getResult() == 1 && thread2.getResult() == 2){
    CU_PASS(1);
  } else {
    CU_FAIL(1);
  }
}




