// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

//#include "RSignal.h"
#include "tests.h"
#include "Logging.h"
#include "RCheck.h"
#include <thread>
#include <signal.h>

void test_sigpipe();

CU_TestInfo RSignalTests[] = {
  {"test SIGPIPE", 
	test_sigpipe},
  CU_TEST_INFO_NULL
};

// init the test suite
int RSignalCUInit() 
{
  return 0;
}

// clean the test suite
int RSignalCUClean() 
{
  return 0;
}

struct ST1 : public RT 
{ 
  typedef Logger<ST1> log;

  ST1() : RT("ST1") {}
  ~ST1() { destroy(); }
  void run() { 
	 RT::ThreadState::move_to(*this, workingState);
	 rSocketCheck(::select(1, 0, 0, 0, 0) > 0);
	 LOG_INFO(log, "select exits");
  }
};

void test_sigpipe()
{
  typedef Logger<LOG::Root> log;

#if 0
  RSignalRepository::instance()
	 . tune_signal(SIGPIPE, RSignalAction::Ignore);
  RSignalRepository::instance()
	 . tune_signal(SIGPIPE, RSignalAction::Ignore);
#endif

  sigset_t ss;
  rCheck(::sigaddset(&ss, SIGPIPE) == 0);
#if 0
  rCheck(::sigwaitinfo(&ss, NULL) > 0);
  LOG_INFO(log, "We got SIGPIPE");
#else
  ST1 t1;
  t1.start();

  rCheck(::sigwaitinfo(&ss, NULL) > 0);
  LOG_INFO(log, "We got SIGPIPE");
#endif

  //USLEEP(100000000);
}


