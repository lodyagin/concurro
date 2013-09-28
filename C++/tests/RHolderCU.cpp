// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "RHolder.hpp"
#include "RThread.hpp"
#include "AutoRepository.h"
#include "tests.h"

void test_nreaders1writer();
void test_rholder();

CU_TestInfo RHolderTests[] = {
  {"test NReaders1WriterGuard", 
   test_nreaders1writer },
  {"test RHolder",
   test_rholder},
  CU_TEST_INFO_NULL
};

// init the test suite
int RHolderCUInit() 
{
  return 0;
}

// clean the test suite
int RHolderCUClean() 
{
  return 0;
}

void test_nreaders1writer()
{
  struct T
  {
    int incw(int) { return ++w; }
    int incr(int) const { return ++r; }
    mutable int r = 3;
    int w = 3;
  };

  NReaders1WriterGuard<T,1000> g(new T);
  const auto& gr = g;

  gr->incr(0);
  gr->incr(gr->incr(0));
  g->incw(0);

  SharedThread([&g]()
               {
                 g->incw(0);
               })
    -> is_terminated().wait();

  const bool locked1 = !SharedThread([&g]()
               {
                 g->incw(g->incw(0));
               })
    -> is_terminated().wait(800);

  CU_ASSERT_TRUE_FATAL(locked1);

  const bool locked2 = !SharedThread([&g,&gr]()
               {
                 gr->incr(g->incw(0));
               })
    -> is_terminated().wait(800);

  CU_ASSERT_TRUE_FATAL(locked2);

  const bool locked3 = !SharedThread([&g, &gr]()
               {
                 g->incw(gr->incr(0));
               })
    -> is_terminated().wait(800);

  CU_ASSERT_TRUE_FATAL(locked3);
}

class Obj : public StdIdMember
{
public:
  struct Par 
  {
    PAR_DEFAULT_MEMBERS(Obj);

    unsigned get_id(const ObjectCreationInfo& oi) const 
      { THROW_PROGRAM_ERROR; }
  };

protected:
  Obj(const ObjectCreationInfo& oi, const Par& p)
    : StdIdMember(oi.objectId)
    {}
};

void test_rholder()
{
  AutoRepository<Obj,unsigned>::init<std::vector>();

  //RHolder<Obj> h(1);
}

