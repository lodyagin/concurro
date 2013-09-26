#include "Repository.hpp"
#include "tests.h"

template<template<class...> class Cont>
void test_create_object();

template<template<class...> class Cont>
void test_delete_object_by_id();

#define TEST_FUN(name, cont) { #name "<" #cont ">", name<cont>}

#define TEST_FUN_SET(name) \
  TEST_FUN(name, std::vector), \
  TEST_FUN(name, std::map), \
  TEST_FUN(name, std::unordered_map)

CU_TestInfo RepositoryTests[] = {
  TEST_FUN_SET(test_create_object),
  TEST_FUN_SET(test_delete_object_by_id),
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
  struct Par {
	 Par(const std::string& str_) : str(str_) {}
	 PAR_DEFAULT_MEMBERS(ObjT);
	 Id get_id(const ObjectCreationInfo& oi) const 
	 { 
		return ++g_obj_id; 
	 }
	 const std::string str;
  };

  ObjT(const ObjectCreationInfo& oi, const Par& par);
  ~ObjT() { destructor_called = true; }

  bool operator== (const ObjT& o) const
  { return str == o.str 
		&& repository == o.repository; }

  bool operator!= (const ObjT& o) const
  { return ! (*this == o); }

  const std::string str;
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
  
  Obj* o1 = r.create_object(typename Obj::Par("o1"));
  const typename Obj::Par par2("o2");
  CU_ASSERT_PTR_NOT_NULL_FATAL(o1);
  Obj* o2 = r.create_object(par2);
  CU_ASSERT_PTR_NOT_NULL_FATAL(o2);
  Obj* o2_2 = r.create_object(par2);
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
  Obj* o1 = r.create_object(typename Obj::Par("o1"));
  CU_ASSERT_EQUAL_FATAL(r.size(), 1);
  /*Obj* o2 =*/ r.create_object(typename Obj::Par("o2"));
  CU_ASSERT_EQUAL_FATAL(r.size(), 2);

  destructor_called = false;
  r.delete_object_by_id(1, false);
  CU_ASSERT_FALSE_FATAL(destructor_called);
  CU_ASSERT_EQUAL_FATAL(r.size(), 1);
  delete o1;

  destructor_called = false;
  r.delete_object_by_id(2, true);
  CU_ASSERT_TRUE_FATAL(destructor_called);
  CU_ASSERT_EQUAL_FATAL(r.size(), 0);

  try {
    r.delete_object_by_id(2, false);
    CU_FAIL_FATAL("Must throw NoSuchId");
  }
  catch(const typename REP(Cont,Obj)::NoSuchId&) {}

  CU_ASSERT_EQUAL_FATAL(r.size(), 0);
}




