#include "Repository.hpp"
#include "tests.h"

template<template<class...> class Cont>
void test_create_object();

template<template<class...> class Cont>
void test_delete_object_by_id();

template<template<class...> class Cont>
void test_for_each();

#define TEST_FUN(name, cont) { #name "<" #cont ">", name<cont>}

#define TEST_FUN_SET(name) \
  TEST_FUN(name, std::vector), \
  TEST_FUN(name, std::map), \
  TEST_FUN(name, std::unordered_map)

CU_TestInfo RepositoryTests[] = {
  TEST_FUN_SET(test_create_object),
  TEST_FUN_SET(test_delete_object_by_id),
  TEST_FUN_SET(test_for_each),
  CU_TEST_INFO_NULL
};

// init the test suite
int RepositoryCUInit() 
{
  return 0;
}

// clean the test suite
int RepositoryCUClean() 
{
  return 0;
}

#define REP(C, Obj)  Repository<Obj, typename Obj::Par, C, typename Obj::Id>


static bool destructor_called(false);
static unsigned int g_obj_id;

template<template<class...> class Cont>
struct ObjT : public StdIdMember
{
public:
  typedef unsigned int Id;
  struct Par 
  {
    Par(const std::string& str_) : str(str_) {}
    Par(int i0) : i(i0) {}
    PAR_DEFAULT_MEMBERS(ObjT);
    Id get_id(const ObjectCreationInfo& oi) const 
    { 
      return ++g_obj_id; 
    }
    const std::string str;
    int i = 0;
  };

  typedef NoGuard<ObjT<Cont>,0> GuardType;

  ObjT(const ObjectCreationInfo& oi, const Par& par);
  ~ObjT() { destructor_called = true; }

  bool operator== (const ObjT& o) const
  { 
    return str == o.str && repository == o.repository; 
  }

  bool operator!= (const ObjT& o) const
  { 
    return ! (*this == o); 
  }

  const std::string str;
  int i;
  REP(Cont, ObjT<Cont>)* repository;
};

template<template<class...> class Cont>
std::ostream&
operator<< (std::ostream& out, const ObjT<Cont>& obj)
{
  out << "{str=[" << obj.str << "]}";
  return out;
}

template<template<class...> class Cont>
ObjT<Cont>::ObjT(const ObjectCreationInfo& oi,
                 const Par& par)
  : StdIdMember(oi.objectId),
    str(par.str),
    i(par.i),
    repository(dynamic_cast<REP(Cont, ObjT)*>(oi.repository))
{
  SCHECK(repository);
}

template<template<class...> class Cont>
void test_create_object()
{
  typedef ObjT<Cont> Obj;
  REP(Cont, Obj) r("test_create_object::r", 0);
  g_obj_id = 0;
  CU_ASSERT_EQUAL_FATAL(r.size(), 0);
  
  Obj* o1 = r.create_object(typename Obj::Par("o1")).get();
  const typename Obj::Par par2("o2");
  CU_ASSERT_PTR_NOT_NULL_FATAL(o1);
  Obj* o2 = r.create_object(par2).get();
  CU_ASSERT_PTR_NOT_NULL_FATAL(o2);
  Obj* o2_2 = r.create_object(par2).get();
  CU_ASSERT_PTR_NOT_NULL_FATAL(o2_2);
  CU_ASSERT_PTR_NOT_EQUAL_FATAL(o2, o2_2);
  CU_ASSERT_PTR_EQUAL_FATAL(o2->repository, &r);
  CU_ASSERT_EQUAL_FATAL(r.size(), 3);
  CU_ASSERT_EQUAL_FATAL(*o2, *o2_2);
  CU_ASSERT_NOT_EQUAL_FATAL(*o2, *o1);
}

template<template<class...> class Cont>
void test_delete_object_by_id()
{
  typedef ObjT<Cont> Obj;
  g_obj_id = 0;
  REP(Cont, Obj) r("test_delete_object_by_id::r", 1);
  
  CU_ASSERT_EQUAL_FATAL(r.size(), 0);
  Obj* o1 = r.create_object(typename Obj::Par("o1")).get();
  CU_ASSERT_EQUAL_FATAL(r.size(), 1);
  /*Obj* o2 =*/ r.create_object(typename Obj::Par("o2"));
  CU_ASSERT_EQUAL_FATAL(r.size(), 2);

  destructor_called = false;
  r.delete_object_by_id(1/*, false*/);
  CU_ASSERT_TRUE_FATAL(destructor_called);
  CU_ASSERT_EQUAL_FATAL(r.size(), 1);
  delete o1;

  destructor_called = false;
  r.delete_object_by_id(2/*, true*/);
  CU_ASSERT_TRUE_FATAL(destructor_called);
  CU_ASSERT_EQUAL_FATAL(r.size(), 0);

  try {
    r.delete_object_by_id(2/*, false*/);
    CU_FAIL_FATAL("Must throw NoSuchId");
  }
  catch(const typename REP(Cont,Obj)::NoSuchId&) {}
  catch(...) { CU_FAIL_FATAL("Improper exception"); }

  CU_ASSERT_EQUAL_FATAL(r.size(), 0);
}

template<template<class...> class Cont>
void test_for_each()
{
  typedef ObjT<Cont> Obj;
  typedef typename Obj::Par Par;
  REP(Cont, Obj) r("test_for_each::r", 0);

  int count = 0;
  int product = 1;

  r.for_each([&count](typename Obj::GuardType& o)
  {
    count++;
  });
  CU_ASSERT_EQUAL_FATAL(count, 0);

  g_obj_id = 0;
  r.create_object(Par(2));
  r.create_object(Par(3));
  r.create_object(Par(5));
  r.create_object(Par(7));

  count = 0; product = 1;
  r.for_each([&count,&product](typename Obj::GuardType& o)
  {
    count++;
    product *= o.get()->i;
  });
  CU_ASSERT_EQUAL_FATAL(count, 4);
  CU_ASSERT_EQUAL_FATAL(product, 210);

  r.delete_object_by_id(3/*, false*/);

  count = 0; product = 1;
  r.for_each([&count,&product](typename Obj::GuardType& o)
  {
    count++;
    product *= o.get()->i;
  });
  CU_ASSERT_EQUAL_FATAL(count, 3);
  CU_ASSERT_EQUAL_FATAL(product, 42);
}


