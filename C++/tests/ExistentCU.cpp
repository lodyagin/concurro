#include "Existent.hpp"
#include "tests.h"

void test_existent();

CU_TestInfo ExistentTests[] = {
  {"test Existent", test_existent},
  CU_TEST_INFO_NULL
};

// init the test suite
int ExistentCUInit() 
{
  return 0;
}

// clean the test suite
int ExistentCUClean() 
{
  return 0;
}

#define ASSERT_STATE(obj, state) \
   CU_ASSERT_TRUE_FATAL( \
     E::TheClass::State::state_is(      \
       (obj).the_class(), \
       E::TheClass::state ## Fun()));

void test_existent()
{
  struct E : public Existent<E> {};

  CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 0);
  {
    E e1;
    CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 1);
    ASSERT_STATE(e1, exist_one);
  }
  CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 0);
  
  {
    E e1;
    CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 1);
    ASSERT_STATE(e1, exist_one);

    {
      E e2;
      CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 2);
      ASSERT_STATE(e1, exist_several);
      ASSERT_STATE(e2, exist_several);
    }
    CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 1);
    ASSERT_STATE(e1, exist_one);
  }
  CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 0);

  {
    E e1;
    CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 1);
    ASSERT_STATE(e1, exist_one);

    E e2;
    CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 2);
    ASSERT_STATE(e2, exist_several);

    {
      E e3;
      CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 3);
      ASSERT_STATE(e3, exist_several);

      E e4;
      CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 4);
      ASSERT_STATE(e1, exist_several);
      ASSERT_STATE(e2, exist_several);
      ASSERT_STATE(e3, exist_several);
      ASSERT_STATE(e4, exist_several);
    }

    CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 2);
    ASSERT_STATE(e1, exist_several);
  }
  CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 0);

#if 0
  {
    E e1;
    E e2;
    CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 2);
    ASSERT_STATE(e2, exist_several);
    e2 = std::move(e1);
    CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 1);
    ASSERT_STATE(e2, exist_one);
    e2 = std::move(e2);
    CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 1);
    ASSERT_STATE(e2, exist_one);
  }

  CU_ASSERT_EQUAL_FATAL(E::get_obj_count(), 0);
#endif
}


