#include "SSingleton.hpp"
#include "RThread.hpp"
#include "CUnit.h"
#include "tests.h"
#include <thread>
#include <functional>
#include <atomic>

void test_sautosingleton_raxis();
void test_ssingleton();
//void test_ssingleton_move();
void test_sautosingleton_auto();
void test_sautosingleton_manual();
void test_concurrent_creation();

CU_TestInfo SSingletonTests[] = {
  {"test SAutosingleton"
   "<RMixedAxis<ExistenceAxis, ExistenceAxis>>",
   test_sautosingleton_raxis},
  {"test SSingleton", test_ssingleton},
//  {"test SSingleton move", test_ssingleton_move},
  {"test SAutoSingleton automatic construction", 
   test_sautosingleton_auto},
  {"test SAutoSingleton manual construction", 
   test_sautosingleton_manual},
  {"concurrent creation", test_concurrent_creation},
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

void test_sautosingleton_raxis()
{
  typedef ExistenceAxis T;

  T* p1, *p2;
  p1 = &T::self();
  p2 = &T::self();
  CU_ASSERT_PTR_EQUAL_FATAL(p1, p2);
}

#undef S

void test_ssingleton()
{
  struct S : public SSingleton<S, 100>
  {
    S() { complete_construction(); }
    int fun() const { return 133; }
  };

  // tests for not-existing singleton
  try {
    S::instance();
    CU_FAIL_FATAL("an exception must be raised");
  }
  catch (const NotExistingSingleton&) {}

  {
    S s1;
    CU_ASSERT_EQUAL_FATAL(S::instance().fun(), 133);
  }
  try {
    S::instance();
    CU_FAIL_FATAL("an exception must be raised");
  }
  catch (const NotExistingSingleton&) {}

  {
    S s1;
    try {
      S s2;
      CU_FAIL_FATAL("an exception must be raised");
    }
    catch (const MustBeSingleton&) {}

    CU_ASSERT_EQUAL_FATAL(S::instance().fun(), 133);
  }
  try {
    S::instance();
    CU_FAIL_FATAL("an exception must be raised");
  }
  catch (const NotExistingSingleton&) {}
}

#if 0
void test_ssingleton_move()
{
  struct S : public SSingleton<S>
  {
  };

  try {
    S s1;
    CU_ASSERT_PTR_EQUAL_FATAL(&S::instance(), &s1);
    S s2(std::move(s1));
    CU_ASSERT_PTR_EQUAL_FATAL(&S::instance(), &s2);
  }
  catch(...) 
  { 
    CU_FAIL_FATAL("The move constructor is broken");
  }
  CU_ASSERT_FALSE_FATAL(S::isConstructed());
}
#endif

void test_sautosingleton_auto()
{
  struct S : public SAutoSingleton<S>
  {
    S() { complete_construction(); }
    int fun() const { return 133; }
  };

  // tests for not-existing singleton
  CU_ASSERT_EQUAL_FATAL(S::instance().fun(), 133);
}

void test_sautosingleton_manual()
{
  struct S : public SAutoSingleton<S>
  {
    S() { complete_construction(); }
    int fun() const { return 134; }
  };

  // tests for not-existing singleton
  {
    S s1;
    CU_ASSERT_EQUAL_FATAL(S::instance().fun(), 134);
  }

  {
    CU_ASSERT_EQUAL_FATAL(S::instance().fun(), 134);
    try {
      S s2;
      CU_FAIL_FATAL("an exception must be raised");
    }
    catch (const MustBeSingleton&) {}

    // auto create here
    CU_ASSERT_EQUAL_FATAL(S::instance().fun(), 134);
  }

  try {
    S s2;
    CU_FAIL_FATAL("an exception must be raised");
  }
  catch (const MustBeSingleton&) {}
}

void test_concurrent_creation()
{
  std::atomic<bool> exception_in_thread(false);

  for (int i = 0; i < 5; i++)
  {
    RThread<std::thread>::create<LightThread>
      ([&exception_in_thread]()
       {
         struct T : public SAutoSingleton<T>
         { 
           T() { usleep(20000); complete_construction(); }
         } ;
         
         try {
           T::instance();
         }
         catch(...) {
           exception_in_thread = true;
           throw;
         }
       }, toString(i))->start();
  }
  CU_ASSERT_FALSE_FATAL(exception_in_thread);

  RThreadRepository<RT>& tr =
    RThreadRepository<RThread<std::thread>>::instance();

#if 0
  // Delete threads
  tr.for_each([](StdThread& t)
  { 
    t.remove();;
  });
  
  CU_ASSERT_EQUAL_FATAL(tr.size(), 0);
#endif
}

class P : public SAutoSingleton<P> 
{
public:
  P() { complete_construction(); }
};

P p;
