#include "SSingleton.h"
#include "RThread.hpp"
#include "CUnit.h"
#include "tests.h"
#include <thread>
#include <functional>

void test_concurrent_creation();

CU_TestInfo SSingletonTests[] = {
  {"concurrent creation",
   test_concurrent_creation},
  CU_TEST_INFO_NULL
};

// init the test suite
int SSingletonCUInit() 
{
  return 0;
}

// clean the test suite
int SSingletonCUClean() 
{
  return 0;
}

class LightThread : public RThread<std::thread>
{
public:
  struct Par : public RThread<std::thread>::Par
  {
    std::function<void()> fun;

    Par(const std::function<void()>& funct) : fun(funct) {}

    RThreadBase* create_derivation
      (const ObjectCreationInfo& oi) const override
    {
      return new LightThread(oi, *this);
    } 
  };

  ~LightThread() { destroy(); }

  void run() override;

protected:
  std::function<void()> fun;

  LightThread(const ObjectCreationInfo&, const Par&);
};

LightThread::LightThread
  (const ObjectCreationInfo& oi, const Par& par)
    : RThread<std::thread>(oi, par), fun(par.fun)
{
  assert(fun);
}

void LightThread::run()
{
  fun();
}

void test_concurrent_creation()
{
  for (int i = 0; i < 100; i++)
  {
    RThread<std::thread>::create<LightThread>
      ([]()
       {
         struct T : public SAutoSingleton<T>
         { 
           T() { usleep(1000000); }
         } ;
         
         T::instance();
       })->start();
  }
}
